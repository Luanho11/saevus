# SAEVUS — Claude Code rules

SAEVUS: 3D third-person action RPG, Unreal Engine 5.8 C++. Movement is the core (acrobatic movement, melee, firearms, PvE, bosses, later PvP). Stylized painted art, dark post-apocalyptic tone. Not photorealistic.

## Source of truth
- Code and repo docs are the source of truth; conversation is extra context. If an old conversation contradicts the repo, trust the repo and report the contradiction.
- Operational state: `Docs/PROJECT_STATE.md`. Session history: `Docs/SESSION_LOG.md`.

## Session protocol
- **Start:** read this file → `Docs/PROJECT_STATE.md` → last entry of `Docs/SESSION_LOG.md` only if needed → `git status` / uncommitted changes. Before any major task, summarize: CURRENT PHASE, CURRENT OBJECTIVE, COMPLETED, IN PROGRESS, NEXT STEP. Never redo completed work just because the chat is new.
- **End of a task/phase (not after every command):** update `PROJECT_STATE.md`, add a concise `SESSION_LOG.md` entry, check `git status`, state the last commit and the single next step.
- The user works phase by phase: do only the current phase, then stop and report.

## Hard constraints
- Target hardware is modest (dev machine: 7 GB RAM, integrated Vega 8). Do not require Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights or heavy shaders.
- No external assets (Fab, characters, weapons, maps, animations, GunZ resources). Placeholders and engine content only until the asset pipeline is designed.
- GunZ: do not invent its mechanics, copy its code or formulas. GunZ research is a later phase; architecture only leaves room for wall run, wall jump, dash, sword movement, weapon switching, aerial combat, animation canceling.
- Design gameplay so it can be replicated later (no full networking or dedicated server yet).
- Don't add features outside the current phase (login, backend, inventory, abilities, enemies, PvP, monetization, etc.).

## Code
- Prefix `SAEVUS` (`ASAEVUSCharacter`, `USAEVUSMovementComponent`). Feature folders under `Source/SAEVUS/` (Core, Character, Movement, Camera, ...).
- Small classes with one responsibility, composition over inheritance, no magic numbers: gameplay values live in tuning structs / Data Assets.
- Validate after each major block: build, fix, open editor, run, check log. Small commits.
- Toolchain: VS 2022 Build Tools 17.14 (MSVC 14.44), Windows SDK 10.0.26100. Do not switch to VS 2026 without discussion.
