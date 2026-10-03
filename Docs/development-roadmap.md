# SAEVUS development roadmap — PROPOSAL

> **Status: draft for the owner to approve, reorder or cut.** Progress (2026-10-03): Phases 0–5 done (Phases 1, 3, 4 and 5 still need the owner's feel tuning; Phase 2 is a draft with open questions). Per `CLAUDE.md`, work is done phase by phase. Each phase ends with build + tests + editor run + `PROJECT_STATE.md` update + commit, then stops for review.

Ordering principle: movement is the core of the game, so it gets solid and feels right before anything is layered on it. Every system is built so it can be replicated later, but real networking comes late, once the single-player rules are stable.

## Phase 0 — Bootstrap (in progress)
Compilable UE 5.8 C++ base: character, camera, walk/run/sprint, jump, air control, Sandbox map, Debug HUD, tests, git.
**Blocked on:** Smart App Control (see `environment.md`).
**Done when:** `Tools/Verify-Bootstrap.ps1` passes and the Sandbox plays correctly in the editor.

## Phase 1 — Locomotion feel
- Tuning pass on the movement and camera Data Assets, played on the target hardware.
- Decide the open questions in `movement.md` (sprint in the air, sprint direction).
- Landing (landing slowdown or recovery window), coyote time and jump buffering as tuning values. Generic platformer techniques only, nothing copied from GunZ.
- Animation hookup with engine template animations only: gait-correct speeds, jump/fall/land states.
- Performance baseline on the Vega 8 (frame time in the Sandbox, `stat unit`), recorded in `PROJECT_STATE.md`.

## Phase 2 — GunZ research (documents only)
- Study how GunZ movement *feels* and which player expressions it allows (wall run, wall jump, dash, sword movement, weapon switching, aerial combat, animation canceling), from public footage and the owner's experience.
- Output: `Docs/research/gunz.md` with **SAEVUS's own** design goals derived from it. No code, formulas or resources copied.

## Phase 3 — Advanced movement
- Custom movement modes in `USAEVUSMovementComponent`, each with its own tuning struct and tests, designed from the Phase 2 document: dash, wall jump, wall run.
- Sandbox additions to test each one.
- Saved-move / network-data support for each ability, so they stay predictable later.

## Phase 4 — Melee foundation
- `USAEVUSCombatComponent`: attack input buffer, combo windows, animation canceling windows (data-driven), hit detection.
- Sword-driven movement hooks into the movement component.
- Training dummy (not an enemy AI).

## Phase 5 — Firearms and weapon switching
- `USAEVUSWeaponComponent`: weapon definitions as Data Assets, hitscan first, switching with cancel rules shared with melee.

## Phase 6 — Asset pipeline design
- Stylized painted look on modest hardware: shading approach, texture budgets, Blender → UE export conventions, naming, LOD rules.
- Placeholder art can be replaced only after this phase. It may need to move earlier if art becomes the bottleneck.

## Phase 7 — PvE enemies
- Basic enemy AI (StateTree), health/damage model shared with the player.

## Phase 8 — Bosses
- Boss framework: phases, telegraphed attacks, arena scripting.

## Phase 9 — Networking
- Replicate movement abilities, combat and weapons; listen server first, then a dedicated server target.

## Phase 10 — PvP
- Modes, matchmaking assumptions, anti-cheat considerations.

## Explicitly out of scope until scheduled
Login, backend, inventory, progression, monetization.
