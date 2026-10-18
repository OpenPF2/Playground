#requires -Version 7.2
<#
.SYNOPSIS
Build and exercise identical project inputs in isolated copies for every supported Unreal engine.
.DESCRIPTION
Run engines sequentially. Results are tied to actual source and asset fingerprints.
Use -Phases Build to find compiler failures before cooking the complete matrix.
Generated data stays separate for each engine. Shared source assets are never saved.
#>
[CmdletBinding()]
param(
    [ValidateSet('5.3', '5.4', '5.5', '5.6', '5.7', '5.8')]
    [string[]] $EngineVersions = @('5.3', '5.4', '5.5', '5.6', '5.7', '5.8'),
    [string] $EngineRoot = 'C:\Program Files\Epic Games',
    [string] $OutputRoot = '',
    [ValidateSet('Build', 'Tests', 'Blueprints', 'Package', 'Smoke', 'Network')]
    [string[]] $Phases = @('Build', 'Tests', 'Blueprints', 'Package', 'Smoke', 'Network')
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$sourceRoot = Split-Path -Parent $PSScriptRoot
$allowedOutputRoot = [IO.Path]::GetFullPath((Join-Path $sourceRoot 'Saved\Compatibility'))
if (!$OutputRoot) { $OutputRoot = $allowedOutputRoot }
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)
if ($OutputRoot -ne $allowedOutputRoot -and !$OutputRoot.StartsWith($allowedOutputRoot + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Compatibility outputs must stay under this project''s Saved\Compatibility directory.'
}
[IO.Directory]::CreateDirectory($OutputRoot) | Out-Null
$requiredPhases = @('Build', 'Tests', 'Blueprints', 'Package', 'Smoke', 'Network')
$map = '/Game/OpenPF2Playground/Maps/Lvl_SoulCave_PF2'
$worldStartedPattern = 'Bringing World ' + [regex]::Escape($map) + '\..+ up for play'

function Resolve-ChildPath([string] $Root, [string] $RelativePath) {
    $childPath = [IO.Path]::GetFullPath((Join-Path $Root $RelativePath))
    if (!$childPath.StartsWith([IO.Path]::GetFullPath($Root) + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Path escapes its expected directory: $RelativePath"
    }
    return $childPath
}

function Get-FileDigest([string] $Path) {
    $stream = [IO.FileStream]::new($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read, 1MB, [IO.FileOptions]::SequentialScan)
    $algorithm = [Security.Cryptography.SHA256]::Create()
    try { return [Convert]::ToHexString($algorithm.ComputeHash($stream)) }
    finally { $algorithm.Dispose(); $stream.Dispose() }
}

function Assert-SnapshotInputs([string] $ProjectRoot, [Collections.Generic.HashSet[string]] $ExpectedPaths) {
    # Refuse source, assets, or default configuration left behind by manual work in a reused copy.
    $repositoryRoots = @($ProjectRoot)
    $pluginRoot = Join-Path $ProjectRoot 'Plugins'
    if (Test-Path -LiteralPath $pluginRoot) {
        $repositoryRoots += @(Get-ChildItem -LiteralPath $pluginRoot -Directory | ForEach-Object FullName)
    }
    foreach ($repositoryRoot in $repositoryRoots) {
        foreach ($directoryName in @('Source', 'Content', 'Config')) {
            $directoryPath = Join-Path $repositoryRoot $directoryName
            if (!(Test-Path -LiteralPath $directoryPath)) { continue }
            foreach ($file in Get-ChildItem -LiteralPath $directoryPath -Recurse -File) {
                $relativePath = [IO.Path]::GetRelativePath($ProjectRoot, $file.FullName).Replace('\', '/')
                if (!$ExpectedPaths.Contains($relativePath)) { throw "Unexpected input in validation copy: $($file.FullName). Use a fresh OutputRoot or remove the extra file." }
            }
        }
    }
}

function Invoke-LoggedCommand([string] $Executable, [string[]] $Arguments, [string] $LogPath) {
    & $Executable @Arguments *> $LogPath
    if ($LASTEXITCODE -ne 0) { throw "$Executable exited with $LASTEXITCODE. See $LogPath" }
}

function Read-RuntimeLog([string] $LogPath) {
    $stream = [IO.File]::Open($LogPath, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite)
    $reader = [IO.StreamReader]::new($stream)
    try { return $reader.ReadToEnd() }
    finally { $reader.Dispose() }
}

function Assert-RuntimeLog([string] $LogPath) {
    # A failed deferred party spawn can return an actor already marked for destruction.
    $errorMatch = [regex]::Match((Read-RuntimeLog $LogPath), '(?im)^.*(?:Error:|Fatal error|Assertion failed|Ensure condition failed|Accessed None|NetworkFailure|SpawnActor failed .*for \[BP_(?:Greystone|TwinBlast)PF2Character_C\]).*$')
    if ($errorMatch.Success) { throw "Runtime errors in $LogPath. First error: $($errorMatch.Value)" }
}

function Wait-LogMatch([Diagnostics.Process] $Process, [string] $LogPath, [string] $Pattern, [int] $TimeoutSeconds = 300) {
    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    do {
        if ((Test-Path -LiteralPath $LogPath) -and ((Read-RuntimeLog $LogPath) -match $Pattern)) { return }
        if ($Process.HasExited) { throw "Process exited with $($Process.ExitCode) before '$Pattern'. See $LogPath" }
        Start-Sleep -Milliseconds 500
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Timed out waiting for '$Pattern'. See $LogPath"
}

function Start-OwnedProcess([string] $Executable, [string[]] $Arguments) {
    # Start-Process joins its array without escaping spaces. Quote values explicitly for Unreal.
    $quotedArguments = @($Arguments | ForEach-Object {
        if ($_ -match '^(-[^=]+=)(.*\s.*)$') { $Matches[1] + '"' + $Matches[2] + '"' }
        elseif ($_ -match '\s') { '"' + $_ + '"' }
        else { $_ }
    })
    return Start-Process -FilePath $Executable -ArgumentList $quotedArguments -WorkingDirectory (Split-Path -Parent $Executable) -WindowStyle Hidden -PassThru
}

function Stop-OwnedProcess([Diagnostics.Process] $Process) {
    # The object retains the handle of the process created by this run.
    if ($null -ne $Process) {
        if (!$Process.HasExited) { $Process.Kill($true); $Process.WaitForExit() }
        $Process.Dispose()
    }
}

function Write-MatrixSummary {
    $rows = @('# Unreal Engine compatibility results', '', '| Engine | Status | Build | Tests | Blueprints | Package | Smoke | Network |', '|---|---|---|---|---|---|---|---|')
    $evidenceRows = @('', '| Engine | Offscreen RHI | Logs | Test reports | Screenshot | Packages | Recorded warnings |', '|---|---|---|---|---|---|---|')
    foreach ($minor in @('5.3', '5.4', '5.5', '5.6', '5.7', '5.8')) {
        $resultPath = Join-Path $OutputRoot "$minor\result.json"
        if (!(Test-Path -LiteralPath $resultPath)) { continue }
        $entry = Get-Content -LiteralPath $resultPath -Raw | ConvertFrom-Json -AsHashtable
        $states = @($requiredPhases | ForEach-Object { if ($entry.Steps.Contains($_)) { $entry.Steps[$_].Status } else { 'Not run' } })
        $status = if ($entry.Fingerprint -ne $script:fingerprint) { 'Outdated inputs' }
            elseif (@($states | Where-Object { $_ -ne 'Passed' }).Count -eq 0) { 'Passed' }
            elseif ($states -contains 'Failed') { 'Failed' }
            elseif ($states -contains 'Running') { 'Running' }
            else { 'Incomplete' }
        $rows += '| ' + $entry.EngineVersion + ' | ' + $status + ' | ' + ($states -join ' | ') + ' |'
        $screenshotLink = 'Not run'
        if ($entry.Steps.Contains('Smoke') -and $entry.Steps.Smoke.Contains('Screenshot')) {
            $relativeScreenshot = [IO.Path]::GetRelativePath($OutputRoot, $entry.Steps.Smoke.Screenshot).Replace('\', '/')
            $screenshotLink = "[Frame](<$relativeScreenshot>)"
        }
        $graphicsApi = 'Not run'
        if ($entry.Steps.Contains('Smoke') -and $entry.Steps.Smoke.Contains('Runtime')) {
            $renderLog = Join-Path $entry.Steps.Smoke.Runtime 'Rendered.log'
            if (Test-Path -LiteralPath $renderLog) {
                $rhiMatch = [regex]::Match((Read-RuntimeLog $renderLog), 'LogRHI: Using (?:Default|Forced) RHI: ([A-Za-z0-9]+)')
                if ($rhiMatch.Success) { $graphicsApi = $rhiMatch.Groups[1].Value }
            }
        }
        $warningCount = if ($entry.Contains('Warnings')) { @($entry.Warnings).Count } else { 0 }
        $evidenceRows += "| $($entry.EngineVersion) | $graphicsApi | [Logs]($minor/Logs) | [OpenPF2]($minor/Reports/OpenPF2/index.json), [Specs]($minor/Reports/EnhancedUnrealSpecs/index.json) | $screenshotLink | [Packages]($minor/Packages) | $warningCount |"
    }
    $rows += $evidenceRows
    $rows += @('', 'Each engine directory contains result.json, Logs, Reports, Packages, and Runtime evidence.', 'A pass requires all six stages for the same source fingerprint. Test counts, warnings, compiler, and SDK are recorded in result.json.')
    [IO.File]::WriteAllLines((Join-Path $OutputRoot 'results.md'), $rows)
}

# Copy materialized tracked worktree files, including submodules, excluding Git metadata and generated data.
$inputFiles = [Collections.Generic.List[object]]::new()
Write-Host 'Fingerprinting tracked source and assets...'
foreach ($repository in @('', 'Plugins/OpenPF2', 'Plugins/EnhancedAutomationSpecs')) {
    $repositoryPath = if ($repository) { Join-Path $sourceRoot $repository } else { $sourceRoot }
    $trackedFiles = & git -C $repositoryPath -c core.quotepath=false ls-files
    if ($LASTEXITCODE -ne 0) { throw "Cannot enumerate $repositoryPath" }
    foreach ($trackedFile in $trackedFiles) {
        $sourceFile = Resolve-ChildPath $repositoryPath $trackedFile
        if (!(Test-Path -LiteralPath $sourceFile -PathType Leaf)) { continue }
        $relativePath = if ($repository) { "$repository/$trackedFile" } else { $trackedFile }
        $inputFiles.Add([PSCustomObject]@{
            RelativePath = $relativePath
            Source = $sourceFile
            Length = (Get-Item -LiteralPath $sourceFile).Length
            Hash = Get-FileDigest $sourceFile
        })
        if ($inputFiles.Count % 1000 -eq 0) { Write-Host "Fingerprinted $($inputFiles.Count) input files..." }
    }
}
$buildInputs = $inputFiles | Where-Object { $_.RelativePath -match '(^|/)(Source|Content|Config|Platforms|Shaders|Resources|Build)/|\.(uproject|uplugin)$' }
$expectedPaths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($inputFile in $inputFiles) { [void]$expectedPaths.Add($inputFile.RelativePath) }
$fingerprintText = ($buildInputs | Sort-Object RelativePath | ForEach-Object { $_.RelativePath + ':' + $_.Hash }) -join [Environment]::NewLine
$script:fingerprint = [Convert]::ToHexString([Security.Cryptography.SHA256]::HashData([Text.Encoding]::UTF8.GetBytes($fingerprintText)))
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
$vswhere = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
if (!(Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio discovery tool vswhere.exe is unavailable.' }
$compilerPaths = @(& $vswhere -products '*' -find 'VC\Tools\MSVC\*\bin\Hostx64\x64\cl.exe')
if (!$compilerPaths.Count) { throw 'No installed x64 MSVC compiler was found.' }
$sdkIncludeRoot = Join-Path $programFilesX86 'Windows Kits\10\Include'
$sdkVersions = @(Get-ChildItem -LiteralPath $sdkIncludeRoot -Directory | Where-Object { Test-Path -LiteralPath (Join-Path $_.FullName 'um\Windows.h') } | ForEach-Object Name)
if (!$sdkVersions.Count) { throw 'No Windows SDK headers were found.' }
$sourceBytes = ($inputFiles | Measure-Object Length -Sum).Sum
$drive = Get-PSDrive -Name ([IO.Path]::GetPathRoot($OutputRoot).Substring(0, 1))
$missingCopies = @($EngineVersions | Where-Object { !(Test-Path -LiteralPath (Join-Path $OutputRoot "$_\Project")) }).Count
if ($drive.Free -lt ($sourceBytes * $missingCopies + 30GB)) { throw 'Insufficient free space for the selected project copies plus 30 GiB of build/cook headroom.' }
$preflight = @{
    Fingerprint = $fingerprint
    SourceBytes = $sourceBytes
    AvailableCompilers = @($compilerPaths | ForEach-Object { @{ Path = $_; Version = (Get-Item -LiteralPath $_).VersionInfo.FileVersion } })
    AvailableWindowsSDKs = $sdkVersions
    FreeBytes = $drive.Free
}
$preflight | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $OutputRoot 'preflight.json')

$hasFailures = $false
# UE 5.3 enforces a 260-character build-output limit. A temporary alias keeps the physical copies under Saved.
$driveAlias = $null
$occupiedDrives = @([IO.Directory]::GetLogicalDrives() | ForEach-Object { $_.Substring(0, 2).ToUpperInvariant() })
foreach ($letter in 90..68) {
    $candidate = ([char]$letter).ToString() + ':'
    if ($candidate -notin $occupiedDrives) { $driveAlias = $candidate; break }
}
if (!$driveAlias) { throw 'No unused drive letter is available for the isolated build paths.' }
& "$env:SystemRoot\System32\subst.exe" $driveAlias $OutputRoot
if ($LASTEXITCODE -ne 0) { throw "Cannot create the temporary build alias $driveAlias." }
$shortOutputRoot = $driveAlias + '\'
Write-Host "Using temporary build alias $driveAlias for $OutputRoot"
try {
foreach ($minor in $EngineVersions) {
    $versionRoot = Join-Path $OutputRoot $minor
    $projectRoot = Join-Path $versionRoot 'Project'
    $logs = Join-Path $versionRoot 'Logs'
    foreach ($directory in @($versionRoot, $projectRoot, $logs)) { [IO.Directory]::CreateDirectory($directory) | Out-Null }
    $engine = Join-Path $EngineRoot "UE_$minor\Engine"
    $version = Get-Content -LiteralPath (Join-Path $engine 'Build\Build.version') -Raw | ConvertFrom-Json
    if ("$($version.MajorVersion).$($version.MinorVersion)" -ne $minor) { throw "Installed engine does not match UE $minor." }
    $engineVersion = "$minor.$($version.PatchVersion)"
    $build = Join-Path $engine 'Build\BatchFiles\Build.bat'
    $uat = Join-Path $engine 'Build\BatchFiles\RunUAT.bat'
    $editor = Join-Path $engine 'Binaries\Win64\UnrealEditor-Cmd.exe'
    foreach ($executable in @($build, $uat, $editor)) { if (!(Test-Path -LiteralPath $executable)) { throw "Missing engine executable: $executable" } }
    $sdkPolicy = Join-Path $engine 'Config\Windows\Windows_SDK.json'
    if (Test-Path -LiteralPath $sdkPolicy) {
        $sdkRequirements = Get-Content -LiteralPath $sdkPolicy -Raw | ConvertFrom-Json -AsHashtable
        $sdkRequirements.Source = $sdkPolicy
    } elseif ($minor -eq '5.3') {
        # UE 5.3 predates the JSON policy. These bounds come from its MicrosoftPlatformSDK.Versions.cs.
        $sdkRequirements = @{
            MainVersion = '10.0.18362.0'; MinVersion = '10.0.00000.0'; MaxVersion = '10.9.99999.0'
            MinSoftwareVersion = '10.0.18362.0'; MaxSoftwareVersion = $null
            Source = Join-Path $engine 'Source\Programs\UnrealBuildTool\Platform\Windows\MicrosoftPlatformSDK.Versions.cs'
        }
    } else { throw "Missing Windows SDK policy for UE $engineVersion." }
    $minimumSdkVersion = [version]$sdkRequirements.MinVersion
    $maximumSdkVersion = [version]$sdkRequirements.MaxVersion
    if ($sdkRequirements.Contains('MinSoftwareVersion') -and $sdkRequirements.MinSoftwareVersion) {
        if ([version]$sdkRequirements.MinSoftwareVersion -gt $minimumSdkVersion) { $minimumSdkVersion = [version]$sdkRequirements.MinSoftwareVersion }
    }
    if ($sdkRequirements.Contains('MaxSoftwareVersion') -and $sdkRequirements.MaxSoftwareVersion) {
        if ([version]$sdkRequirements.MaxSoftwareVersion -lt $maximumSdkVersion) { $maximumSdkVersion = [version]$sdkRequirements.MaxSoftwareVersion }
    }
    if (@($sdkVersions | Where-Object { [version]$_ -ge $minimumSdkVersion -and [version]$_ -le $maximumSdkVersion }).Count -eq 0) {
        throw "No installed Windows SDK satisfies UE $engineVersion ($minimumSdkVersion through $maximumSdkVersion)."
    }
    Write-Host "UE $engineVersion Preparing isolated project..."
    $manifestPath = Join-Path $versionRoot 'inputs.json'
    if (Test-Path -LiteralPath $manifestPath) {
        $currentPaths = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
        foreach ($inputFile in $inputFiles) { [void]$currentPaths.Add($inputFile.RelativePath) }
        foreach ($previousPath in (Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json)) {
            if (!$currentPaths.Contains($previousPath)) {
                $obsoleteFile = Resolve-ChildPath $projectRoot $previousPath
                if (Test-Path -LiteralPath $obsoleteFile -PathType Leaf) {
                    (Get-Item -LiteralPath $obsoleteFile).IsReadOnly = $false
                    Remove-Item -LiteralPath $obsoleteFile
                }
            }
        }
    }
    $verifiedInputsPath = Join-Path $versionRoot 'verified-inputs.json'
    $verifiedInputs = @{}
    if (Test-Path -LiteralPath $verifiedInputsPath) { $verifiedInputs = Get-Content -LiteralPath $verifiedInputsPath -Raw | ConvertFrom-Json -AsHashtable }
    $nextVerifiedInputs = @{}
    $preparedCount = 0
    foreach ($inputFile in $inputFiles) {
        $destination = Resolve-ChildPath $projectRoot $inputFile.RelativePath
        $isAsset = $destination -match '\.(uasset|umap)$'
        $destinationInfo = if (Test-Path -LiteralPath $destination) { Get-Item -LiteralPath $destination } else { $null }
        $cachedInput = $verifiedInputs[$inputFile.RelativePath]
        # Reuse only immutable assets whose content was verified previously and whose metadata is unchanged.
        $canReuseAsset = $isAsset -and $null -ne $destinationInfo -and $null -ne $cachedInput -and
            $destinationInfo.IsReadOnly -and $destinationInfo.Length -eq $inputFile.Length -and
            $destinationInfo.LastWriteTimeUtc.Ticks -eq $cachedInput.LastWriteTicks -and $cachedInput.Hash -eq $inputFile.Hash
        if ($canReuseAsset) {
            $nextVerifiedInputs[$inputFile.RelativePath] = $cachedInput
            $preparedCount++
            continue
        }
        [IO.Directory]::CreateDirectory((Split-Path -Parent $destination)) | Out-Null
        if (Test-Path -LiteralPath $destination) { (Get-Item -LiteralPath $destination).IsReadOnly = $false }
        [IO.File]::Copy($inputFile.Source, $destination, $true)
        if ((Get-FileDigest $destination) -ne $inputFile.Hash) { throw "Input changed during snapshot preparation: $($inputFile.RelativePath)" }
        if ($isAsset) {
            $assetInfo = Get-Item -LiteralPath $destination
            $assetInfo.IsReadOnly = $true
            $nextVerifiedInputs[$inputFile.RelativePath] = @{ Hash=$inputFile.Hash; LastWriteTicks=$assetInfo.LastWriteTimeUtc.Ticks }
        }
        $preparedCount++
        if ($preparedCount % 1000 -eq 0) { Write-Host "UE $engineVersion Prepared $preparedCount input files..." }
    }
    $nextVerifiedInputs | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $verifiedInputsPath
    ConvertTo-Json -InputObject @($inputFiles.RelativePath) | Set-Content -LiteralPath $manifestPath
    Assert-SnapshotInputs $projectRoot $expectedPaths
    $project = Join-Path $shortOutputRoot "$minor\Project\OpenPF2Playground.uproject"
    $resultPath = Join-Path $versionRoot 'result.json'
    $result = @{ EngineVersion = $engineVersion; Changelist = $version.Changelist; Fingerprint = $fingerprint; Steps = [ordered]@{} }
    if (Test-Path -LiteralPath $resultPath) {
        $previousResult = Get-Content -LiteralPath $resultPath -Raw | ConvertFrom-Json -AsHashtable
        if ($previousResult.Fingerprint -eq $fingerprint -and $previousResult.Changelist -eq $version.Changelist) { $result = $previousResult }
    }
    $result.SDKRequirements = $sdkRequirements
    $result | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $resultPath
    $ddcArguments = @()
    if ([IO.File]::ReadAllText((Join-Path $engine 'Config\BaseEngine.ini')).Contains('[InstalledNoZenLocalFallback]')) {
        $ddcArguments = @('-ddc=InstalledNoZenLocalFallback')
    }
    $editorFlags = @('-unattended', '-nop4', '-nosplash', '-nosound') + $ddcArguments
    $commonArguments = @($project) + $editorFlags
    foreach ($phase in $requiredPhases | Where-Object { $_ -in $Phases }) {
        Write-Host "UE $engineVersion $phase starting at $([DateTimeOffset]::Now.ToString('o'))"
        if ($phase -eq 'Build') {
            foreach ($dependentPhase in $requiredPhases | Where-Object { $_ -ne 'Build' }) { [void]$result.Steps.Remove($dependentPhase) }
        } elseif ($phase -eq 'Package') { [void]$result.Steps.Remove('Smoke') }
        $result.Steps[$phase] = @{ Status = 'Running'; StartedUtc = [DateTime]::UtcNow.ToString('o') }
        $result | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $resultPath
        Write-MatrixSummary
        $phaseResult = $result.Steps[$phase]
        try {
            if ($phase -ne 'Build' -and (!$result.Steps.Contains('Build') -or $result.Steps.Build.Status -ne 'Passed')) { throw 'A successful Build stage for these inputs is required first.' }
            switch ($phase) {
                'Build' {
                    # Recompile one small source so even warm builds report the compiler and SDK actually used.
                    (Get-Item -LiteralPath (Join-Path $projectRoot 'Source\OpenPF2Playground\OpenPF2Playground.cpp')).LastWriteTimeUtc = [DateTime]::UtcNow
                    foreach ($target in @('OpenPF2PlaygroundEditor', 'OpenPF2Playground')) {
                        Invoke-LoggedCommand $build @($target, 'Win64', 'Development', "-Project=$project", '-WaitMutex', '-NoHotReloadFromIDE', '-NoUBTMakefiles', "-Log=$logs\UBT-$target.log") (Join-Path $logs "Build-$target.log")
                    }
                    $phaseResult.Toolchain = @(Get-ChildItem -LiteralPath $logs -Filter 'Build-*.log' | Select-String -Pattern 'Using Visual Studio.*toolchain.*SDK' | ForEach-Object Line | Sort-Object -Unique)
                    if (!$phaseResult.Toolchain.Count) { throw 'Build succeeded, but its compiler and SDK were not recorded. Inspect the build logs.' }
                }
                'Tests' {
                    foreach ($suite in @(@{Name='OpenPF2'; Minimum=1886}, @{Name='EnhancedUnrealSpecs'; Minimum=71})) {
                        $report = Join-Path $versionRoot "Reports\$($suite.Name)"
                        $testLog = Join-Path $logs "Tests-$($suite.Name).log"
                        $reportIndex = Join-Path $report 'index.json'
                        if (Test-Path -LiteralPath $reportIndex) { Remove-Item -LiteralPath $reportIndex }
                        Invoke-LoggedCommand $editor ($commonArguments + @('-NullRHI', "-ExecCmds=Automation RunTests $($suite.Name)", '-TestExit=Automation Test Queue Empty', "-ReportExportPath=$report", "-abslog=$testLog", '-stdout', '-FullStdOutLogOutput')) (Join-Path $logs "Tests-$($suite.Name)-Console.log")
                        $testReport = Get-Content -LiteralPath $reportIndex -Raw | ConvertFrom-Json
                        if ($testReport.succeeded -lt $suite.Minimum -or $testReport.failed -ne 0 -or $testReport.notRun -ne 0 -or $testReport.succeededWithWarnings -ne 0) { throw "Incomplete or failing $($suite.Name) suite. See $report" }
                        $phaseResult[$suite.Name] = @{ Succeeded=$testReport.succeeded; Failed=$testReport.failed; NotRun=$testReport.notRun; Warnings=$testReport.succeededWithWarnings; Report=$report }
                    }
                }
                'Blueprints' {
                    $blueprintLog = Join-Path $logs 'Blueprints.log'
                    $allowListPath = Join-Path $projectRoot 'Saved\Compatibility-Blueprint-AllowList.txt'
                    [IO.Directory]::CreateDirectory((Split-Path -Parent $allowListPath)) | Out-Null
                    $allowedAssets = @($inputFiles | ForEach-Object {
                        if ($_.RelativePath -match '^Content/(.+)\.uasset$') {
                            '/Game/' + $Matches[1] + '.' + [IO.Path]::GetFileName($Matches[1])
                        } elseif ($_.RelativePath -match '^Plugins/([^/]+)/Content/(.+)\.uasset$') {
                            '/' + $Matches[1] + '/' + $Matches[2] + '.' + [IO.Path]::GetFileName($Matches[2])
                        }
                    })
                    [IO.File]::WriteAllLines($allowListPath, $allowedAssets)
                    Invoke-LoggedCommand $editor ($commonArguments + @('-NullRHI', '-run=CompileAllBlueprints', '-AllowListFile=Saved/Compatibility-Blueprint-AllowList.txt', "-abslog=$blueprintLog", '-stdout', '-FullStdOutLogOutput')) (Join-Path $logs 'Blueprints-Console.log')
                    $blueprintText = [IO.File]::ReadAllText($blueprintLog)
                    if ($blueprintText -notmatch 'Compiling Completed with (\d+) errors and (\d+) warnings and (\d+) blueprints that failed to load') { throw 'Blueprint commandlet did not report its compile totals.' }
                    $phaseResult.Errors = [int]$Matches[1]; $phaseResult.Warnings = [int]$Matches[2]; $phaseResult.FailedToLoad = [int]$Matches[3]
                    if ($phaseResult.Errors -ne 0 -or $phaseResult.FailedToLoad -ne 0) { throw "Blueprint failures. See $blueprintLog" }
                    $phaseResult.Compiled = [regex]::Matches($blueprintText, 'Loading and Compiling:').Count
                    if ($phaseResult.Compiled -lt 257) { throw "Only $($phaseResult.Compiled) Blueprints compiled; at least 257 existing project/plugin Blueprints are required." }
                }
                'Package' {
                    $archive = Join-Path $versionRoot 'Packages'
                    Invoke-LoggedCommand $uat (@('BuildCookRun', "-project=$project", '-nop4', '-platform=Win64', '-clientconfig=Development', "-map=$map", '-build', '-cook', '-stage', '-pak', '-iostore', '-package', '-archive', "-archivedirectory=$archive", '-unattended', '-utf8output', '-nocompileeditor') + $ddcArguments) (Join-Path $logs 'Package.log')
                    $phaseResult.Archive = $archive
                }
                'Smoke' {
                    # The cooked map must reach play, render a settled frame, and shut down normally.
                    if (!$result.Steps.Contains('Package') -or $result.Steps.Package.Status -ne 'Passed') { throw 'Package must pass for these inputs before Smoke.' }
                    $game = Join-Path $versionRoot 'Packages\Windows\OpenPF2Playground\Binaries\Win64\OpenPF2Playground.exe'
                    $runtimeRoot = Join-Path $versionRoot ('Runtime\' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss'))
                    [IO.Directory]::CreateDirectory($runtimeRoot) | Out-Null
                    $phaseResult.Runtime = $runtimeRoot
                    $smokeLog = Join-Path $runtimeRoot 'Rendered.log'
                    $smokeProcess = $null
                    try {
                        # DEFER queues one command per game tick; screenshot-delay frames alone do not tick gameplay.
                        $screenshotCommand = ('defer ' * 300) + 'HighResShot 1'
                        $offscreenArguments = @()
                        # UE 5.3's D3D12 offscreen viewport assumes a swap chain. This project also cooks D3D11 SM5.
                        if ($minor -eq '5.3') { $offscreenArguments = @('-d3d11') }
                        $smokeProcess = Start-OwnedProcess $game (@($map, '-RenderOffscreen', '-windowed', '-ResX=1280', '-ResY=720', '-unattended', '-nosound', '-nosplash', '-benchmark', '-FPS=30', '-seconds=30', "-UserDir=$runtimeRoot", "-abslog=$smokeLog", "-ExecCmds=t.MaxFPS 30,r.HighResScreenshotDelay 3,$screenshotCommand") + $offscreenArguments)
                        Wait-LogMatch $smokeProcess $smokeLog $worldStartedPattern
                        Wait-LogMatch $smokeProcess $smokeLog 'High resolution screenshot saved as'
                        $screenshots = @(Get-ChildItem -LiteralPath $runtimeRoot -Recurse -Filter 'HighresScreenshot*.png' -File)
                        if (!$screenshots.Count) { throw 'No rendered screenshot was produced.' }
                        $phaseResult.Screenshot = $screenshots[-1].FullName
                        if (!$smokeProcess.WaitForExit(300000)) { throw 'Packaged clean shutdown timed out.' }
                        if ($smokeProcess.ExitCode -ne 0) { throw "Packaged clean exit failed with $($smokeProcess.ExitCode)." }
                        Assert-RuntimeLog $smokeLog
                        if ([IO.File]::ReadAllText($smokeLog) -notmatch 'LogExit: Exiting') { throw 'Normal engine shutdown was not logged.' }
                        $phaseResult.CleanExitCode = $smokeProcess.ExitCode
                    } finally { Stop-OwnedProcess $smokeProcess }
                }
                'Network' {
                    # A client must join this engine's loopback server and load the replicated map.
                    $portProbe = [Net.Sockets.UdpClient]::new([Net.IPEndPoint]::new([Net.IPAddress]::Loopback, 0))
                    $port = $portProbe.Client.LocalEndPoint.Port
                    $portProbe.Dispose()
                    $serverLog = Join-Path $logs 'Network-Server.log'
                    $clientLog = Join-Path $logs 'Network-Client.log'
                    foreach ($oldLog in @($serverLog, $clientLog)) { if (Test-Path -LiteralPath $oldLog) { Remove-Item -LiteralPath $oldLog } }
                    $serverProcess = $null; $clientProcess = $null
                    try {
                        $serverProcess = Start-OwnedProcess $editor (@($project, $map) + $editorFlags + @('-server', '-NullRHI', '-MULTIHOME=127.0.0.1', "-port=$port", "-abslog=$serverLog", '-ExecCmds=t.MaxFPS 30'))
                        Wait-LogMatch $serverProcess $serverLog "IpNetDriver listening on port $port"
                        Wait-LogMatch $serverProcess $serverLog $worldStartedPattern
                        $clientProcess = Start-OwnedProcess $editor (@($project, "127.0.0.1:$port") + $editorFlags + @('-game', '-NullRHI', '-MULTIHOME=127.0.0.1', "-abslog=$clientLog", '-ExecCmds=t.MaxFPS 30'))
                        Wait-LogMatch $clientProcess $clientLog 'Welcomed by server'
                        Wait-LogMatch $clientProcess $clientLog $worldStartedPattern
                        Wait-LogMatch $serverProcess $serverLog 'Join succeeded'
                        Start-Sleep -Seconds 5
                        Assert-RuntimeLog $serverLog
                        Assert-RuntimeLog $clientLog
                        $phaseResult.Port = $port
                    } finally { Stop-OwnedProcess $clientProcess; Stop-OwnedProcess $serverProcess }
                }
            }
            $phaseResult.Status = 'Passed'
            Write-Host "UE $engineVersion $phase PASSED"
        } catch {
            $phaseResult.Status = 'Failed'; $phaseResult.Error = $_.Exception.Message
            $hasFailures = $true
            Write-Warning "UE $engineVersion $phase FAILED: $($_.Exception.Message)"
        } finally {
            $phaseResult.CompletedUtc = [DateTime]::UtcNow.ToString('o')
            $currentLogs = @(Get-ChildItem -LiteralPath $logs -Filter '*.log' | Where-Object { $_.Name -match '^(Build-|Tests-|Blueprints\.log$|Package\.log$|Network-)' -and $_.Name -notmatch '-Console\.log$' })
            if ($result.Steps.Contains('Smoke') -and $result.Steps.Smoke.Contains('Runtime')) {
                $currentLogs += @(Get-ChildItem -LiteralPath $result.Steps.Smoke.Runtime -Filter '*.log' -File)
            }
            $result.Warnings = @($currentLogs | Select-String -Pattern 'Log\w+: Warning:.*|^WARNING:.*$|^.*warning (?:C|CS|LNK)\d+.*$|^The config file .*will be staged, but is not explicitly allowed or denied\..*$' -AllMatches | ForEach-Object { $_.Matches.Value } | Sort-Object -Unique)
            $result | ConvertTo-Json -Depth 12 | Set-Content -LiteralPath $resultPath
            Write-MatrixSummary
        }
        if ($phaseResult.Status -eq 'Failed') { break }
    }
}
Write-MatrixSummary
if ($hasFailures) { exit 1 }
} finally {
    & "$env:SystemRoot\System32\subst.exe" $driveAlias /d
}
