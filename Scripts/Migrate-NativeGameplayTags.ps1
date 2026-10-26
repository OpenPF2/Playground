#requires -Version 7.2
# OpenPF2 Playground, Copyright 2026, Guy Elsmore-Paddock. All Rights Reserved.
<#
.SYNOPSIS
Resave tracked gameplay-tag content in UE 5.3, or verify the saved content without redirects.
.DESCRIPTION
Resave validates the legacy migration map, compiles and saves every Blueprint, and records canonical values.
Audit loads the same manifest in a fresh process, requires no active redirects, and compares the saved values.
Remove project redirects only after Resave succeeds. The commandlet never deletes invalid tag values.
#>
[CmdletBinding()]
param(
    [ValidateSet('Resave', 'Audit')]
    [string] $Mode = 'Resave',
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.3',
    [string] $OutputRoot = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$projectRoot = Split-Path -Parent $PSScriptRoot
$allowedOutputRoot = [IO.Path]::GetFullPath((Join-Path $projectRoot 'Saved\TagMigration-20261025'))
if (!$OutputRoot) { $OutputRoot = $allowedOutputRoot }
$OutputRoot = [IO.Path]::GetFullPath($OutputRoot)
if ($OutputRoot -ne $allowedOutputRoot -and !$OutputRoot.StartsWith($allowedOutputRoot + '\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Migration output must stay under Saved\TagMigration-20261025.'
}
[IO.Directory]::CreateDirectory($OutputRoot) | Out-Null
$engineVersion = Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine\Build\Build.version') -Raw | ConvertFrom-Json
if ($engineVersion.MajorVersion -ne 5 -or $engineVersion.MinorVersion -ne 3) {
    throw 'Shared assets must be saved and audited with Unreal Engine 5.3.'
}
$manifestPath = Join-Path $OutputRoot 'packages.txt'
$baselinePath = Join-Path $OutputRoot 'baseline.json'
$reportPath = if ($Mode -eq 'Resave') { $baselinePath } else { Join-Path $OutputRoot 'audit.json' }
if ($Mode -eq 'Resave') {
    $packages = [Collections.Generic.List[string]]::new()
    foreach ($repository in @('', 'Plugins/OpenPF2')) {
        $repositoryRoot = if ($repository) { Join-Path $projectRoot $repository } else { $projectRoot }
        $tracked = @(& git -C $repositoryRoot -c core.quotepath=false ls-files '*.uasset' '*.umap')
        if ($LASTEXITCODE -ne 0) { throw "Cannot enumerate tracked packages in $repositoryRoot" }
        foreach ($relativePath in $tracked) {
            $packagePath = [IO.Path]::GetFullPath((Join-Path $repositoryRoot $relativePath))
            if (!$packagePath.StartsWith($repositoryRoot + '\', [StringComparison]::OrdinalIgnoreCase)) {
                throw "Package escapes repository: $relativePath"
            }
            if (!(Test-Path -LiteralPath $packagePath -PathType Leaf)) { throw "Missing package: $packagePath" }
            $stream = [IO.File]::OpenRead($packagePath)
            try {
                $header = [byte[]]::new(64)
                $count = $stream.Read($header, 0, $header.Length)
                if ([Text.Encoding]::ASCII.GetString($header, 0, $count).StartsWith('version https://git-lfs.github.com/spec/')) {
                    throw "Unmaterialized Git LFS package: $packagePath"
                }
            } finally { $stream.Dispose() }
            $packages.Add($packagePath)
        }
    }
    $packages.Sort([StringComparer]::Ordinal)
    [IO.File]::WriteAllLines($manifestPath, $packages)
} elseif (!(Test-Path -LiteralPath $baselinePath) -or !(Test-Path -LiteralPath $manifestPath)) {
    throw 'Run Resave successfully before Audit.'
}
if ($Mode -eq 'Audit') {
    $baseline = Get-Content -LiteralPath $baselinePath -Raw | ConvertFrom-Json
    if ($baseline.Failures -ne 0 -or $baseline.InvalidTags -ne 0) { throw 'Cannot audit against a failed migration baseline.' }
    $currentPackages = foreach ($repository in @('', 'Plugins/OpenPF2')) {
        $repositoryRoot = if ($repository) { Join-Path $projectRoot $repository } else { $projectRoot }
        $tracked = @(& git -C $repositoryRoot -c core.quotepath=false ls-files '*.uasset' '*.umap')
        if ($LASTEXITCODE -ne 0) { throw "Cannot enumerate tracked packages in $repositoryRoot" }
        foreach ($relativePath in $tracked) { [IO.Path]::GetFullPath((Join-Path $repositoryRoot $relativePath)) }
    }
    if (Compare-Object @(Get-Content -LiteralPath $manifestPath) @($currentPackages)) {
        throw 'Tracked package inventory changed after Resave. Create a new migration baseline.'
    }
}
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$project = Join-Path $projectRoot 'OpenPF2Playground.uproject'
$redirectConfig = Join-Path $projectRoot 'Plugins\OpenPF2\Config\DefaultGameplayTags.ini'
$arguments = @($project, '-run=PF2MigrateGameplayTags', '-unattended', '-nop4', '-nosplash', '-nosound',
    '-NullRHI', '-stdout', '-FullStdOutLogOutput', "-Manifest=$manifestPath", "-Report=$reportPath",
    "-RedirectConfig=$redirectConfig", "-abslog=$OutputRoot\$Mode.log")
if ($Mode -eq 'Resave') { $arguments += '-Resave' }
else { $arguments += @('-NoRedirects', "-Baseline=$baselinePath") }
& $editor @arguments *> (Join-Path $OutputRoot "$Mode-console.log")
if ($LASTEXITCODE -ne 0) { throw "$Mode failed with exit code $LASTEXITCODE. See $OutputRoot\$Mode.log" }
$report = Get-Content -LiteralPath $reportPath -Raw | ConvertFrom-Json
if ($report.Failures -ne 0 -or $report.InvalidTags -ne 0) { throw 'Migration report contains failures or invalid tags.' }
if ($report.Packages.Count -ne @(Get-Content -LiteralPath $manifestPath).Count -or $report.Blueprints -lt 257) {
    throw 'Migration did not cover the complete package manifest and existing Blueprint inventory.'
}
$logText = [IO.File]::ReadAllText((Join-Path $OutputRoot "$Mode.log"))
if ($logText -match '(?im)^.*(?:Error:|Fatal error|Assertion failed|Ensure condition failed|LogGameplayTags:\s+Warning:).*$') {
    throw "Errors were logged during $Mode. Inspect $OutputRoot\$Mode.log"
}
Write-Host "$Mode passed: $($report.Packages.Count) packages, $($report.Blueprints) Blueprints, $($report.Saved) saved."
if ($report.LegacyReferences.Count -ne 0) {
    Write-Warning "$($report.LegacyReferences.Count) untyped legacy-name references require inspection in $reportPath."
}
