# SAEVUS SESSION LOG

Short history of the development sessions. The current state is in `Docs/PROJECT_STATE.md`.

## 2026-10-02 — Session 1

### Goal
Bootstrap the environment and the UE 5.8 C++ base, then set up the persistent project context.

### Completed
- Environment inspection (OS, x64, Git, CMake, Python, VS, MSVC, SDK, UE, Blender).
- Installed: CMake 4.4.3, VS 2022 Build Tools 17.14 (MSVC 14.44.35207 + SDK 10.0.26100.0), Blender 4.2.16 LTS, Epic Games Launcher.
- Context system: `CLAUDE.md`, `Docs/PROJECT_STATE.md`, `Docs/SESSION_LOG.md`.

### Changes
- `Source/SAEVUS/` feature-folder structure.
- First C++ (not compiled yet): module files, `USAEVUSMovementComponent`, movement tuning structs + `USAEVUSMovementSettings`, `ESAEVUSGait`.

### Problems
- UE 5.8 needs a login to the Epic Launcher; stopped for the user to install it. Download in progress at the end of the session.
- Hardware well below Epic's recommendation (7 GB RAM, integrated GPU).

### Decisions
- VS 2022 Build Tools / MSVC 14.44 instead of VS 2026 (compiler newer than the one Epic validated for UE 5.8).
- CharacterMovementComponent instead of Mover; gait requests through saved moves.
- No Lumen / Nanite / RT / VSM / MegaLights.

### Next
Verify the UE 5.8 install, then continue Bootstrap from FASE 4 (create the project).

## 2026-10-03 — Session 2

### Goal
Continue Bootstrap from FASE 3 (verify UE 5.8).

### Completed
- UE 5.8.3 verified. Project created: `.uproject`, targets, config (no Lumen/Nanite/RT/VSM/MegaLights), engine template content packs copied.
- Added the .NET Framework 4.8 SDK to VS Build Tools (UBT required it).
- Wrote the GameMode, PlayerController, Character, camera component + settings, input config, Debug HUD + CVar and movement automation tests. The full module builds clean (0 warnings).
- Wrote the editor Python script for the bootstrap assets and `L_Sandbox`.

### Problems
- Smart App Control (Enforce) blocks loading `UnrealEditor-SAEVUS.dll` (error 4551). The editor, tests and asset script can't run until it's off.

### Decisions
- F6 toggles the Debug HUD (F1–F5, F9, F11 are engine bindings).
- Sprint FOV follows actual speed, not the sprint request.
- Combat and weapon components stay out of Bootstrap (not in its scope).

### Next
User turns Smart App Control off → run the tests, run the asset script, play `L_Sandbox`, then git init + commit.
