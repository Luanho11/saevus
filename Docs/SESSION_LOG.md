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

### Night work (user asleep, Smart App Control still on)
- Static review against the 5.8 sources. Fixed: test characters stuck in `MOVE_None`; mannequin mesh set via its setter (transient property); key names now fail loudly.
- Validated the engine-only half of the asset script in a Blueprint-only scratch project: the input assets, the full `L_Sandbox` build and the saves all work.
- Added `saevus_verify_assets.py` and `Tools/Verify-Bootstrap.ps1` (one-command build/test/assets/verify).
- Git initialized with LFS; initial commit plus fixes.
- Docs: architecture, movement, environment, development roadmap (proposal).

### Next
User turns Smart App Control off → `Tools\Verify-Bootstrap.ps1` → play `L_Sandbox` → commit the generated assets → owner reviews the roadmap.

## 2026-10-03 — Session 3

### Goal
Finish Phase 1 (locomotion feel) code work.

### Completed
- Coyote time, jump buffer, take-off gait locked in the air, with tests (committed `d09ac17`; 8/8 tests pass after a fresh build).
- Smoke test now records game / render / GPU time over the movement stages only; optional `-SAEVUSSmokeProfileGPU`. `Tools/Run-SmokeTest.ps1` wraps it.
- Scalability sweep at 720p on the Vega 8, written up in `Docs/performance.md`.

### Findings
- GPU-bound; game thread flat at 7–9 ms. Medium ≈ 55–68 fps, High ≈ 45–49, Epic ≈ 37. Medium's gain is its sub-native resolution; no single scalability group dominates.
- The first run after a rebuild is cold (400 ms hitches, game time 28 ms); only warm runs count.

### Decisions
- Landing recovery and gait-correct animation deferred until animation work (they need landing / sprint animations to read well).
- Shipped default scalability is a settings-menu decision, not Phase 1.

### Next
Owner playtests `L_Sandbox` and gives feel feedback; owner approves the roadmap; then Phase 2 (GunZ research, documents only).

### Phase 2 (same session)
- `Docs/research/gunz.md` from public sources (Wikipedia, StrategyWiki/Fandom excerpts, critpoints.net analysis, MMOBomb on GunZ 2). Key finding: GunZ's depth came from unintended animation cancels; SAEVUS goal is a designed, data-driven cancel system with a low input floor.
- Phase 3 order proposed: dash → wall jump → wall run → wall climb. Owner questions listed in the doc.

### Phase 3 — dash (same session)
- Dash in `USAEVUSMovementComponent` with `FSAEVUSDashTuning` (speed, duration, exit speed, cooldown, air budget, gravity suspension), `IA_Dash` input, HUD line, smoke-test stage. Saved move carries the request (`FLAG_Custom_2`) and the dash state.
- Tests: 5 new `SAEVUS.Movement.Dash.*`, 13/13 pass. Verify-Bootstrap passed; rendered smoke test PASSED (dash 1600 cm/s).
- Problem: the asset script could not save; Git LFS `lockable` had left every `.uasset`/`.umap` read-only. Removed `lockable` (no lock server) and cleared the flag.
- Next: wall jump.
