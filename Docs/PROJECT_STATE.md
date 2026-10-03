# SAEVUS PROJECT STATE

_Last updated: 2026-10-03 (Session 2)_

## Current Objective

Bootstrap: prepare the development environment and a compilable UE 5.8 C++ base. The phase is done when: SAEVUS compiles, the editor opens, the character spawns, the camera works, walk/run/sprint/jump/air control work, the Sandbox map and Debug HUD exist, and Git has an initial commit.

## Current Phase

Bootstrap, **FASE 13 build ✅ / FASE 14 run ⛔**. Blocked: Windows **Smart App Control** (Enforce mode) won't load the locally built, unsigned `UnrealEditor-SAEVUS.dll` (`GetLastError=4551`, CodeIntegrity event 3077). The editor can't load the game module, so the tests, the asset script and the editor itself can't run. Waiting for the user to turn Smart App Control off.

Bootstrap phase order: 1 inspect environment ✅ · 2 install missing dependencies ✅ · 3 verify UE 5.8 ✅ · 4 create project ✅ · 5 folders ✅ · 6 base classes ✅ (code) · 7 character ✅ (code) · 8 camera ✅ (code) · 9 walk/run/sprint ✅ (code) · 10 jump/air control ✅ (code) · 11 Sandbox ⏳ (script validated in a scratch project, not run in SAEVUS) · 12 Debug HUD ✅ (code) · 13 build ✅ · 14 run ⛔ · 15 commit ✅.

## Completed

- Environment inspected (see Notes).
- Installed through winget, official sources: CMake 4.4.3, VS 2022 Build Tools 17.14.37710 (MSVC 14.44.35207, Windows SDK 10.0.26100.0), Blender 4.2.16 LTS, Epic Games Launcher.
- UE 5.8.3 (CL 58210709) installed by the user. The launcher manifest shows a complete install; `UnrealEditor.exe` and the `TP_ThirdPerson` template are present.
- Added the .NET Framework 4.8 SDK + 4.6.2 targeting pack to VS Build Tools (UBT needs NetFxSDK for SwarmInterface).
- Project files: `SAEVUS.uproject` (EngineAssociation 5.8; PythonScriptPlugin + EditorScriptingUtilities, editor only), `Source/SAEVUS.Target.cs`, `Source/SAEVUSEditor.Target.cs` (BuildSettingsVersion.V7, IncludeOrder Unreal5_8), `Config/Default{Engine,Game,Editor,Input}.ini`.
- Rendering config: Lumen, Nanite, ray tracing, VSM, MegaLights, distance fields and static lighting are off; SSR reflections; TAA.
- Engine template content copied into `Content/Characters` (mannequin + ABP_Unarmed), `Content/Input` (IA_Move/Look/MouseLook/Jump, IMC_Default/MouseLook) and `Content/LevelPrototyping` (grid materials, meshes).
- C++ written and **compiled clean (0 warnings)** with `SAEVUSEditor Win64 Development`:
  - `ASAEVUSGameMode`, `ASAEVUSPlayerController` (adds IMCs, owns the F6 debug toggle action);
  - `ASAEVUSCharacter` (uses `USAEVUSMovementComponent`, spring arm + `USAEVUSCameraComponent`, binds actions from `USAEVUSInputConfig`);
  - `USAEVUSCameraComponent` + `FSAEVUSCameraTuning` / `USAEVUSCameraSettings` (arm framing, lag, sprint FOV blended by real speed);
  - `ASAEVUSDebugHUD` + the `SAEVUS.Debug.HUD` CVar (`Debug/SAEVUSDebugCVars`);
  - the Session 1 movement code compiled with no changes;
  - automation tests `SAEVUS.Movement.*` (gait priority, gait speeds, settings apply, jump apex vs v²/2g, air control).
- `Tools/EditorScripts/saevus_bootstrap_assets.py`: idempotent editor script that creates IA_Sprint/IA_Walk/IA_ToggleDebugHUD + IMC_SAEVUS, DA_InputConfig / DA_MovementSettings_Default / DA_CameraSettings_Default, BP_SAEVUSCharacter / BP_SAEVUSPlayerController / BP_SAEVUSGameMode and `L_Sandbox`.
  - Engine-only parts (input assets, key mappings, BP creation + compile, the whole `L_Sandbox` build, game-mode override, save) **validated in UE 5.8** with a Blueprint-only scratch project, which Smart App Control doesn't block. The parts that use SAEVUS classes (Data Assets, BP CDO edits) still need the real run.
- `Tools/EditorScripts/saevus_verify_assets.py`: fresh-process check that the saved BP defaults (mesh, anim class, settings, mapping contexts, pawn class) persisted.
- `Tools/Verify-Bootstrap.ps1`: one command for build → tests → assets → verify. Fails fast with a clear message while Smart App Control blocks the DLL (checked live).
- Static review against the 5.8 sources found and fixed: test characters stayed in `MOVE_None` (the engine skips the default mode when `bRunPhysicsWithNoController` is set after spawn), and the mannequin mesh must be set through `SetSkeletalMeshAsset` (the property is transient).
- `DefaultEngine.ini`: `L_Sandbox` is the editor and game default map; `BP_SAEVUSGameMode` is the global game mode.
- Git: repo with LFS; commits `4752e9f`, `b32600a`, plus the docs commit.
- Docs: `architecture.md`, `movement.md`, `environment.md`, `development-roadmap.md` (**proposal**, needs the owner's approval).

## In Progress

- The run is blocked by Smart App Control (see Known Problems). The `SAEVUS.*` automation tests and the SAEVUS-class parts of the asset script have not run yet.

## Pending

- User turns off Smart App Control. Then run `Tools\Verify-Bootstrap.ps1` and fix whatever fails.
- Open the editor and play `L_Sandbox`: walk (LeftAlt) / run / sprint (LeftShift, L3), jump, air control, camera, F6 Debug HUD. Take a `-game` screenshot and check the log. Commit the generated assets.
- Owner reviews `Docs/development-roadmap.md` and picks the phase after Bootstrap.
- `combat.md` and `asset-pipeline.md` are deliberately not written: they belong to their own phases (see the roadmap).
- Outside Bootstrap scope, so deliberately not written yet: `USAEVUSCombatComponent`, `USAEVUSWeaponComponent`.

## Architecture

- `ASAEVUSCharacter` (ACharacter) composes `USAEVUSMovementComponent` (CMC subclass, set via `SetDefaultSubobjectClass`), a spring arm and `USAEVUSCameraComponent`. Combat and weapon components come in their own phases.
- Movement: built on CharacterMovementComponent, with future abilities as custom movement modes. Gait = Walk/Run/Sprint; sprint wins over walk. `GetMaxSpeed()` uses the gait speed both on the ground and in the air, so jump momentum is preserved. Network-ready through `FSAEVUSSavedMove` (FLAG_Custom_0 = sprint, FLAG_Custom_1 = walk).
- Camera: on register, `USAEVUSCameraComponent` pushes `FSAEVUSCameraTuning` into its parent spring arm. It blends the FOV from base to sprint by actual horizontal speed between run and sprint speed.
- Data-driven: tuning structs hold the defaults in one place; Data Assets (`USAEVUSMovementSettings`, `USAEVUSCameraSettings`, `USAEVUSInputConfig`) override them.
- Input: Enhanced Input. `ASAEVUSPlayerController` adds the mapping contexts (template IMC_Default + IMC_MouseLook + IMC_SAEVUS) and owns controller-level actions (debug toggle). The Character binds gameplay actions from `USAEVUSInputConfig`.
- The native GameMode uses native classes as fallbacks; `BP_SAEVUSGameMode` points to the BP pawn and controller. BPs carry asset references; C++ doesn't.
- Debug HUD: `ASAEVUSDebugHUD` canvas, drawn while `SAEVUS.Debug.HUD` is on. F6 toggles it (F1–F5, F9 and F11 are engine debug bindings).

## Important Files

| File | Purpose |
|---|---|
| `CLAUDE.md` | Permanent project rules for Claude Code |
| `Docs/PROJECT_STATE.md` | Operational state (this file) |
| `Docs/SESSION_LOG.md` | Short history of each session |
| `SAEVUS.uproject`, `Source/SAEVUS*.Target.cs` | Project and build targets |
| `Config/DefaultEngine.ini` | Modest-hardware rendering profile |
| `Source/SAEVUS/SAEVUS.Build.cs` | Module dependencies (Core, CoreUObject, Engine, InputCore, EnhancedInput) |
| `Source/SAEVUS/Movement/` | Movement component, tuning structs + Data Asset, gait enum |
| `Source/SAEVUS/Character/` | Character, input config Data Asset |
| `Source/SAEVUS/Camera/` | Camera component, camera tuning + Data Asset |
| `Source/SAEVUS/Core/` | GameMode, PlayerController |
| `Source/SAEVUS/Debug/` | Debug HUD, debug CVars |
| `Source/SAEVUS/Tests/SAEVUSMovementTests.cpp` | `SAEVUS.Movement.*` automation tests |
| `Tools/EditorScripts/saevus_bootstrap_assets.py` | Creates or updates the bootstrap assets and `L_Sandbox` |

## Technical Decisions

- **UE 5.8.x binary from the Epic Launcher** (no source build).
- **VS 2022 Build Tools 17.14 / MSVC 14.44 / SDK 10.0.26100**, not VS 2026 18.10. UE 5.8 supports MSVC 14.38–14.50, and VS 2026 18.10 ships a compiler newer than the 14.50 Epic validated. Build Tools instead of the full IDE to save RAM.
- **CharacterMovementComponent, not the Mover plugin**, for now. CMC is mature for predicted networked movement and custom modes. Revisit only with a concrete reason.
- **Gait requests through compressed flags / saved moves**, so sprint stays client-predicted later.
- **Rendering for modest hardware:** no Lumen / Nanite / RT / VSM / MegaLights / distance fields; conventional dynamic lighting; TAA instead of TSR.
- **Engine template content** (mannequin, template input assets, prototype materials) is allowed; external assets are not.
- **Sprint FOV follows actual speed**, not the sprint request, so it doesn't widen while blocked by a wall.
- **Debug HUD on F6.**
- **Bootstrap assets come from a script** (`Tools/EditorScripts`), so they can be rebuilt instead of hand-edited.

## Known Problems

- **Smart App Control (Enforce) blocks the game DLL.** `HKLM\SYSTEM\CurrentControlSet\Control\CI\Policy\VerifiedAndReputablePolicyState = 1`. Windows refuses any unsigned, locally compiled DLL (error 4551, CodeIntegrity events 3077/3033), and self-signing doesn't help. The fix is turning Smart App Control off (Windows Security → App & browser control → Smart App Control settings → Off). That's the user's decision.
- Dev hardware is below Epic's recommendation: 7 GB RAM vs 32 GB recommended, integrated GPU. UBT limits itself to 1 parallel compile, so a full module build takes about 3–4 min. Close other apps before building or opening the editor.
- Git `user.name` isn't configured globally (only `user.email`); the repo uses a local `Hostos`.

## Testing

- Build: `Engine/Build/BatchFiles/Build.bat SAEVUSEditor Win64 Development -Project=<path>\SAEVUS.uproject -WaitMutex -NoHotReload` → **Succeeded, 0 warnings**.
- Everything else: `powershell -ExecutionPolicy Bypass -File Tools\Verify-Bootstrap.ps1` (build → `SAEVUS.*` tests → asset script → asset verify; logs in `Saved\Logs\Verify\`). Currently stops at step 2 with the Smart App Control message.
- Engine noise to ignore: 14 × `LogAutomationTest: Error: Condition failed` during editor startup, before any project code runs (seen in an empty Blueprint-only project too).

## Git

Branch `main`, Git LFS for `.uasset`/`.umap` and source art. `user.name` is set only in this repo (`Hostos`). Last commit: see `git log -1`.

## Next Step

Once the user confirms Smart App Control is off: run `Tools\Verify-Bootstrap.ps1`, fix failures, play `L_Sandbox` in the editor, commit the generated assets, and close Bootstrap.

## Notes

- Machine: Windows 11 Home 10.0.26200 x64, AMD Ryzen 5 2500U (4C/8T), 7 GB RAM, AMD Radeon Vega 8 (driver 27.20.22002.57), ~733 GB free on C:.
- Already installed before Session 1: Git 2.54.0 + Git LFS 3.7.1, Python 3.14.5, winget 1.29.
- Paths: VS Build Tools `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`; UE `C:\Program Files\Epic Games\UE_5.8`; Blender `C:\Program Files\Blender Foundation\Blender 4.2`.
- UE 5.8 released 2026-06-17; latest hotfix 5.8.3.
- Engine debug keys in `BaseInput.ini`: F1–F5 view modes, F9 screenshot, F11 fullscreen/immersive.
