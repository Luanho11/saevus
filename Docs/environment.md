# SAEVUS development environment

## Toolchain

| Tool | Version | Path |
|---|---|---|
| Unreal Engine (Epic Launcher binary) | 5.8.3 (CL 58210709) | `C:\Program Files\Epic Games\UE_5.8` |
| VS 2022 Build Tools | 17.14, MSVC 14.44.35207 | `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools` |
| Windows SDK | 10.0.26100.0 | |
| .NET Framework SDK | 4.8 (+ 4.6.2 targeting pack) | `C:\Program Files (x86)\Windows Kits\NETFXSDK\4.8` |
| Git / Git LFS | 2.54.0 / 3.7.1 | |
| Python (system) | 3.14.5 | the editor uses its own embedded Python |
| CMake | 4.4.3 | |
| Blender | 4.2.16 LTS | `C:\Program Files\Blender Foundation\Blender 4.2` |

Stay on VS 2022 / MSVC 14.44. UE 5.8 was validated against MSVC 14.38–14.50, and VS 2026 ships a newer compiler. UnrealBuildTool needs the .NET Framework SDK (SwarmInterface); without it the build fails with *"Could not find NetFxSDK install dir"*.

## Windows requirement: Smart App Control off

Smart App Control blocks every unsigned DLL without cloud reputation, and that includes every DLL UnrealBuildTool produces. With it on, the editor refuses the game module (`GetLastError=4551`, CodeIntegrity event 3077). Check the state:

```powershell
(Get-ItemProperty "HKLM:\SYSTEM\CurrentControlSet\Control\CI\Policy").VerifiedAndReputablePolicyState
# 0 = off, 1 = enforce, 2 = evaluation
```

Turn it off in Windows Security → App & browser control → Smart App Control settings.

## Build, test, verify

From the project root:

```powershell
# Everything: build, SAEVUS.* tests, create assets, verify assets. Logs in Saved\Logs\Verify\
powershell -ExecutionPolicy Bypass -File Tools\Verify-Bootstrap.ps1
# Skip re-creating assets
powershell -ExecutionPolicy Bypass -File Tools\Verify-Bootstrap.ps1 -SkipAssets
```

Individual steps:

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" SAEVUSEditor Win64 Development -Project="$PWD\SAEVUS.uproject" -WaitMutex -NoHotReload
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "$PWD\SAEVUS.uproject" -ExecCmds="Automation RunTests SAEVUS;Quit" -TestExit="Automation Test Queue Empty" -unattended -nopause -nullrhi -nosplash -NoSound
```

Open the editor: double-click `SAEVUS.uproject` (build first, so it doesn't ask to rebuild).

## Low-memory machine (7 GB RAM, Vega 8)

- Close browsers and other apps before building or opening the editor. UBT runs one compile at a time when free RAM is under 1.5 GB, so a full module build takes about 3–4 minutes.
- The first editor launch compiles shaders and fills the DDC; it's slow once, then faster.
- Rendering features that need a stronger GPU are off project-wide (see `architecture.md`). Don't turn them on in maps or post-process volumes.
- To work on code only, use `-nullrhi` runs (tests, scripts), which skip the GPU.

## Repository

- Git LFS tracks `*.uasset`, `*.umap` (lockable) and source art (`*.blend`, `*.fbx`, `*.png`, `*.tga`, `*.psd`, `*.wav`).
- Ignored: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, IDE files.
- `user.name` is set only in this repo (`Hostos`). Set a global one with `git config --global user.name "<name>"`.
