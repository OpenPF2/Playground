# Unreal Engine compatibility

OpenPF2, Playground, and EnhancedUnrealSpecs have passed the Win64 matrix below. The project association is 5.3, which is also the asset-authoring baseline. Invoke another installed engine explicitly when building or testing that version.

## Validated patch versions

Validation completed on 2026-10-19 using the same source and asset fingerprint, `A71692A74F28EC1181E32CCCB1660D31061295EA682C48971591FEE45FEE1895`.

| Engine patch | MSVC selected | Windows SDK | Offscreen RHI | Blueprint warnings | All six stages |
| --- | --- | --- | --- | --- | --- |
| 5.3.2 | 14.36.32548 | 10.0.26100.0 | D3D11, SM5 | 0 | Passed |
| 5.4.4 | 14.38.33145 | 10.0.26100.0 | D3D12 | 0 | Passed |
| 5.5.4 | 14.38.33145 | 10.0.22621.0 | D3D12 | 0 | Passed |
| 5.6.1 | 14.38.33145 | 10.0.22621.0 | D3D12 | 0 | Passed |
| 5.7.4 | 14.44.35228 | 10.0.22621.0 | D3D12 | 2 | Passed |
| 5.8.3 | 14.44.35228 | 10.0.22621.0 | D3D12 | 2 | Passed |

Every row passed Development Editor and Game builds, all 1,886 OpenPF2 and 71 EnhancedUnrealSpecs cases, compilation of 257 Blueprints, Soul Cave cooking and packaging, delayed offscreen rendering with normal shutdown, and a loopback server/client connection. There were zero C++, C#, or linker warnings, zero failed or skipped tests, and zero Blueprint errors or load failures. All six screenshots were visually inspected.

The installed MSVC directories are `14.36.32532`, `14.38.33130`, and `14.44.35207`; the table records the actual toolchain versions reported by UBT. The [local result table](../Saved/Compatibility/results.md) links each engine's logs, test reports, screenshot, and package. Full warnings and toolchain paths are retained in each `result.json`.

### Rendering scope

UE 5.3.2's D3D12 `-RenderOffscreen` path emits a handled `SwapChain4.GetReference()` ensure during viewport initialization in this installed engine. Its validated offscreen run uses the project's supported D3D11 SM5 backend through `-d3d11`. UE 5.4.4 through 5.8.3 use D3D12. The runner keeps runtime error and ensure checks enabled; it does not count the failing D3D12 offscreen probe as a pass.

This matrix validates the listed backends and patch releases on Win64. It does not establish support for UE 5.3.2 D3D12 offscreen rendering or other untested graphics backends.

## Validated source revisions

The validated OpenPF2 code is at `c63f159e56f81140f23bd2c27369b8a76d305541`; EnhancedUnrealSpecs is at `a5a267751eb06f28be32802aa884559387903d26`. The Playground commit containing this guide pins both submodules, including subsequent contributor documentation and copyright notice updates.

The recorded fingerprint identifies the tested snapshot before the copyright end-year updates. Those later source edits change copyright comments only; exact byte comparisons verified that all other source content and third-party notices were preserved. The full engine matrix was not rerun for this comment and documentation follow-up.

## Target rules

Both the Editor and Game targets select named settings through UnrealBuildTool's engine-version preprocessor symbols.

| Engine | Build settings | Include order |
| --- | --- | --- |
| 5.3 | `BuildSettingsVersion.V4` | `EngineIncludeOrderVersion.Unreal5_3` |
| 5.4 | `BuildSettingsVersion.V5` | `EngineIncludeOrderVersion.Unreal5_4` |
| 5.5 | `BuildSettingsVersion.V5` | `EngineIncludeOrderVersion.Unreal5_5` |
| 5.6 | `BuildSettingsVersion.V5` | `EngineIncludeOrderVersion.Unreal5_6` |
| 5.7 | `BuildSettingsVersion.V6` | `EngineIncludeOrderVersion.Unreal5_7` |
| 5.8 | `BuildSettingsVersion.V7` | `EngineIncludeOrderVersion.Unreal5_8` |

The targets reject versions outside this range. Modules inherit the selected target include order. Changes to the supported range require an explicit rules update and validation.

## Source and asset contracts

- Use `Misc/EngineVersionComparison.h` and `UE_VERSION_OLDER_THAN` for C++ API boundaries. Keep reflected layouts and Blueprint interfaces consistent across engines.
- UE 5.4 introduced the array shrinking enum and reflected-map iterator accessor used by OpenPF2. UE 5.3 uses their older equivalents.
- UE 5.5 introduced the ability asset-tag accessors, dynamic ability-spec tag accessor, replication/PIE accessors, and scoped automation flags used here.
- On UE 5.8, the ability system component destructor detaches its active-effect container owner before base destruction. The installed 5.8.3 engine logs that owner's full path while deleting pending effects, after its outer actor may already have been purged. Detaching the dying owner prevents a shutdown access violation and leaves pending-effect cleanup enabled.
- `AddDefaultAbilityTag` composes a new tag container without modifying its input. Ability constructors assign that result through the API available in their engine.
- The spec declaration macros retain their invocation syntax. Use `EnhancedAutomationApplicationContextMask` for a context mask that compiles on every supported engine.
- Author and save shared assets in UE 5.3. The validation runner marks its copied assets read-only and runs Blueprint compilation without resaving source packages.
- The mapped-abilities widget handles an empty selected character during startup or possession changes. It unbinds the previous component, subscribes only to a valid new component, and retains empty-slot placeholders. This correction was saved with UE 5.3.2.
- The additional-party-member spawn tries to adjust its location and always retains required party members. The previous inherited collision policy rejected Twin Blast on the UE 5.5 server and left a dying actor to receive ownership and ability initialization. This correction was also saved with UE 5.3.2. Failed required-party spawns fail the runtime checks.

## Run the matrix

### Native gameplay tag migration

Use `Scripts/Migrate-NativeGameplayTags.ps1 -Mode Resave` with the UE 5.3 editor built from the current source. It loads every tracked Playground and OpenPF2 asset, validates the plugin's legacy-name redirects, refreshes and compiles every Blueprint, and saves Blueprints and other packages containing tags. The report compares canonical tag values before and after Blueprint reconstruction and refuses to save a package that loses values or retains invalid tags.

After a successful resave, remove the copied redirects from Playground's `Config/DefaultGameplayTags.ini` and run the script with `-Mode Audit`. This starts a fresh editor process, requires zero active redirects, loads the same complete manifest, and compares the serialized values with the resave baseline. Keep the plugin's redirect map for consuming projects that still have older assets. Reports and logs are under `Saved/TagMigration-20261025`.

The manifest includes all tracked `.uasset` and `.umap` files in both repositories, including third-party content and maps with embedded Blueprints. Missing packages, Git LFS pointers, legacy references, invalid tags, compilation errors, and changed baseline values fail the migration. Add new assets before creating the baseline so they enter the tracked manifest.

### Compatibility matrix

Install PowerShell 7.2 or later, Git with Git LFS, the selected Unreal engines, Visual Studio C++ tools, and Windows SDKs accepted by those engines. Initialize both submodules and materialize their Git LFS assets before running. The runner discovers installed tools and records the compiler and SDK selected by UnrealBuildTool. It regenerates each target's build graph and recompiles the small Playground entry source so reused copies do not retain stale source lists and otherwise up-to-date builds still report their toolchains. It requires room for the selected materialized project copies plus at least 30 GiB of build/cook headroom.

SDK preflight checks the engine's allowed and software-version bounds before preparing its copy. UE 5.3 predates `Windows_SDK.json`; its explicit bounds match `MicrosoftPlatformSDK.Versions.cs`, including the 10.0.18362.0 software minimum. Newer engines use their installed JSON policy.

Avoid concurrent Unreal builds outside the runner. Installed engines share the user's marketplace build-rules cache, and an unrelated build can hold that file open while another engine tries to regenerate it.

From the project directory in PowerShell:

```powershell
# Full validation for all six engine versions.
& .\Scripts\Test-UnrealCompatibility.ps1

# Find compiler failures before cooking.
& .\Scripts\Test-UnrealCompatibility.ps1 -Phases Build

# Verify prerequisites and prepare copies without repeating acceptance stages.
& .\Scripts\Test-UnrealCompatibility.ps1 -Phases @()

# Run a subset, or resume selected checks after fixing a failure.
& .\Scripts\Test-UnrealCompatibility.ps1 -EngineVersions '5.3','5.8' -Phases Tests,Blueprints,Package,Smoke,Network
```

The default engine root is `C:\Program Files\Epic Games`; `-EngineRoot` overrides it. `-OutputRoot` may select a child of this project's `Saved/Compatibility` directory. A temporary unused drive alias keeps generated paths short enough for UE 5.3 and is removed on normal completion or an exception.

The runner copies materialized tracked files from Playground, OpenPF2, and EnhancedUnrealSpecs. It hashes build inputs, verifies new copies, and reuses previously verified read-only assets only while their hashes and metadata still match. It rejects unexpected source, content, or default configuration files in reused copies. Each engine has separate project, intermediate, binary, configuration, cook, package, and report directories. Commit new source files before running so they are included in the snapshot.

## Acceptance and evidence

An engine passes only when all stages pass for the same source fingerprint and engine changelist:

1. Development Editor and Game builds, including the applicable OpenPF2 modules.
2. At least 1,886 passing OpenPF2 tests and 71 passing EnhancedUnrealSpecs tests, with zero failed, unrun, or warning results.
3. Blueprint compilation with zero compilation errors or packages that fail to load.
4. A complete Soul Cave cook, stage, package, and archive using that engine.
5. Packaged map startup, an offscreen screenshot after 300 gameplay ticks, and a normal zero-code shutdown of that same rendered process. Benchmark mode advances at 30 simulated frames per second and exits after 30 simulated seconds. The screenshot's render-delay setting alone does not advance gameplay.
6. A client joining a loopback editor server and loading the replicated Soul Cave map.

`Saved/Compatibility/results.md` summarizes the matrix. Each engine's `result.json`, `Logs`, `Reports`, `Packages`, and `Runtime` directories contain its evidence. A repeated Build stage invalidates dependent stage results; a repeated Package stage invalidates its Smoke result. Compiler and engine warnings remain visible in the result file.

Rendered screenshots need visual inspection, and these smoke checks do not replace an interactive gameplay test. Other platforms and untested patch releases require their own validation.

## Warning categories

The runner preserves warning text in each engine's `result.json`. Warnings observed during validation include:

- Legacy Paragon assets reference missing retarget rigs and invalid animation-test-map build data. The developer test AI controller references the removed `PawnActionsComponent` class.
- The developer-only `GE_BonusToPerceptionRolls` effect references the stale `PerceptionModifier` attribute.
- `GetIdForLogs` in two interfaces produces a Python exposure-name collision.
- Newer Blueprint compilers warn that the instant `GE_InitializeEncounterPoints` and `GE_ResetEncounterPoints_Instant` effects cannot stack.
- Older gameplay-tag configuration files stage without explicit allow/deny entries. Newer engines deprecate the existing `MatchSubstring` core redirects.
- Editor networking can report unsupported camera visualization components, a null particle notify, and navigation-data regeneration. These warnings do not prevent the loopback connection.
- Installed-engine/editor warnings include missing AudioWidgets resources, editor data-storage widget and menu registration, trace-server availability, a timed-out HTTP connectivity probe, and a shader-worker job-object completion-port warning. UE 5.5 also reports capitalization differences when resolving the temporary drive alias to its physical path.

These remaining warnings did not fail the documented acceptance stages. No warning has been hidden through compiler switches or test exclusions. Consult the per-engine logs for exact messages and affected packages.
