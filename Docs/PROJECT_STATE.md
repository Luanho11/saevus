# SAEVUS PROJECT STATE

_Last updated: 2026-10-03 (Session 3)_

## Current Objective

Phase 1 — Locomotion feel (see `Docs/development-roadmap.md`). The user authorized unattended work through the roadmap phases.

## Current Phase

**Bootstrap: DONE (2026-10-03).** All acceptance criteria met: compiles clean, editor/game opens, character spawns, camera works, walk/run/sprint/jump/air control verified (automation tests + rendered smoke test), Sandbox map and Debug HUD exist, git has commits.

**Phase 1 — Locomotion feel: code work DONE, waiting on the owner's playtest.** Done: coyote time, jump buffer, take-off gait locked in the air, open movement questions decided or deferred (`movement.md`), performance baseline (`performance.md`). Left: the feel-tuning pass on `DA_MovementSettings_Default` / `DA_CameraSettings_Default`, which needs a human playing it.

**Phase 2 — GunZ research: DRAFT DONE (2026-10-03).** `Docs/research/gunz.md`: public-source summary, SAEVUS design goals, Phase 3 proposal and six questions for the owner.

**Phase 3 — Advanced movement: IN PROGRESS.** Order: dash (**done**) → wall jump (**done**) → wall run → wall climb.

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

- Phase 1: coyote time, jump buffer and air-gait lock (`FSAEVUSAirMovementTuning`, character `RequestJump` / `CanJumpWhileFalling`), shared test fixture `Tests/SAEVUSMovementTestWorld.h`, 8/8 tests pass.
- Phase 1: smoke test records game / render / GPU time; `Tools/Run-SmokeTest.ps1` (scalability / extra cvars / `-ProfileGPU`); scalability sweep recorded in `Docs/performance.md`.
- Phase 3 dash: `FSAEVUSDashTuning`, `RequestDash` / `CanDash` / `IsDashing` in the movement component, `IA_Dash` (Left Ctrl, mouse thumb, gamepad B), air-dash budget, jump-cancel, saved-move state (`FLAG_Custom_2`), HUD line, smoke-test stage, 5 new tests (13/13 pass).
- Phase 3 wall jump: `FSAEVUSWallJumpTuning`, `TryWallJump` / `FindWallJumpNormal` (8 capsule sweeps, ignores foot-level ledges), budget + air-dash refill, saved-move state, HUD line, Sandbox chimney + 4.5 m ledge, 6 new tests (19/19 pass).
- `.gitattributes`: removed `lockable` from `.uasset`/`.umap` (no LFS lock server; it left assets read-only and the asset script could not save).

## In Progress

- Phase 3: wall run (next).

## Pending

- Owner answers the questions at the end of `Docs/research/gunz.md` (Phase 3 uses toggleable defaults until then).
- Owner playtests `L_Sandbox` and tunes feel on the Data Assets (or reports what feels off).
- Landing recovery and gait-correct animation (sprint foot sliding with the template blend space) are deferred until animation work; see `movement.md`.
- Owner reviews `Docs/development-roadmap.md` (order of later phases).
- `combat.md` and `asset-pipeline.md` belong to their own phases.

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
| `Tools/Run-Tests.ps1`, `Tools/Run-SmokeTest.ps1` | Headless automation tests; rendered smoke test with frame timing |
| `Docs/performance.md` | Performance baseline and how to measure |

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
- **Dash is a timed state inside Walking/Falling**, not a custom movement mode: it only overrides velocity and gravity. Wall run / wall climb will be `MOVE_Custom` sub-modes.
- **Git LFS without `lockable`** until there is an LFS remote with locking.

## Known Problems

- Dev hardware is below Epic's recommendation: 7 GB RAM vs 32 GB recommended, integrated GPU (Vega 8, 1 GB dedicated + shared, driver Adrenalin 21.20.02 from 2021, DX12 SM6.6 OK). UBT limits itself to 1 parallel compile, so a full module build takes about 3–4 min. First rendered launch compiles shaders for several minutes.
- Smart App Control was turned off by the user (required for locally built DLLs).
- The editor appended `[/Script/AndroidFileServerEditor...]` defaults to `DefaultEngine.ini` (engine plugin defaults, harmless).
- Git `user.name` isn't configured globally; the repo uses a local `Hostos`.

## Testing

- `powershell -ExecutionPolicy Bypass -File Tools\Verify-Bootstrap.ps1` → build, `SAEVUS.*` automation tests, asset script, asset verify. **All passed** (2026-10-03).
- `Tools\Run-Tests.ps1` → 19/19 `SAEVUS.Movement.*` pass (2026-10-03, Session 3): Gait.Priority, Gait.Speeds, Settings.Apply, Air.JumpApex (142.8 vs 142.9 cm), Air.AirControl (434 vs ~420 cm/s), JumpAssist.CoyoteTime / JumpBuffer / AirGaitLock, Dash.Ground (293 vs 288 cm) / Direction / Cooldown / AirBudget / JumpCancel, WallJump.Basic / NoWall / NotOffLedge / Tangential / Budget / RefillsAirDash.
- `Tools\Verify-Bootstrap.ps1` (build, tests, asset script, asset verify) → all passed after adding `IA_Dash` and the wall-jump chimney.
- Rendered smoke test: `Tools\Run-SmokeTest.ps1 [-Scalability N]` → **PASSED**: run 550.0, sprint 850.0, jump apex 142.9 cm, dash 1600.0; screenshot `Saved/Screenshots/WindowsEditor/SAEVUS_SmokeTest.png`.
- Performance baseline (720p, warm): Low 118 fps, Medium 55–68, High 45–49, Epic 37. GPU-bound; game thread 7–9 ms. Medium is the 60 fps tier on the Vega 8, mostly thanks to its sub-native resolution. Details in `Docs/performance.md`.
- Simulation tests tick the movement component directly (a game-mode-less test world does not run its tick groups).
- Engine noise to ignore: 14 × `LogAutomationTest: Error: Condition failed` during editor startup.

## Git

Branch `main`, Git LFS for `.uasset`/`.umap` and source art. `user.name` is set only in this repo (`Hostos`). Last commit: see `git log -1`.

## Next Step

Owner: play `L_Sandbox` (F6 for the debug HUD) and give feel feedback, and approve or reorder `Docs/development-roadmap.md`. After that: Phase 2 (GunZ research, documents only) per the roadmap.

## Notes

- Machine: Windows 11 Home 10.0.26200 x64, AMD Ryzen 5 2500U (4C/8T), 7 GB RAM, AMD Radeon Vega 8 (driver 27.20.22002.57), ~733 GB free on C:.
- Already installed before Session 1: Git 2.54.0 + Git LFS 3.7.1, Python 3.14.5, winget 1.29.
- Paths: VS Build Tools `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`; UE `C:\Program Files\Epic Games\UE_5.8`; Blender `C:\Program Files\Blender Foundation\Blender 4.2`.
- UE 5.8 released 2026-06-17; latest hotfix 5.8.3.
- Engine debug keys in `BaseInput.ini`: F1–F5 view modes, F9 screenshot, F11 fullscreen/immersive.
