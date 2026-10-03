# GunZ research → SAEVUS design goals

Status: Phase 2 draft (2026-10-03). Built from public community documentation only. **Needs the owner's own GunZ experience** (see "Questions for the owner") before Phase 3 locks anything in.

## Rules for this document

- Describes what GunZ players *do* and why it feels good, in plain words. No code, formulas, frame data, timings, assets or animation data from GunZ, and none are needed.
- SAEVUS values are designed and tuned from scratch in SAEVUS's own tuning structs.
- Community technique names (K-style, butterfly…) are used here only to identify what is being discussed. SAEVUS features get their own names.

## Sources

- Wikipedia, *GunZ: The Duel* — gameplay overview (wall running, flips, mid-air dodges; K-style as animation/weapon-switch cancels; Quest mode with bosses).
- StrategyWiki, *GunZ The Duel / Controls* and */Korean-Style and Advanced Techniques* (read through search excerpts; the pages refuse direct fetches).
- GunZ Fandom wiki, *K-Style* (search excerpts).
- critpoints.net, *MTM: Gunz – Legendkiller 2* (2016) — design analysis of K-style.
- MMOBomb, *GunZ 2: Some Answers and Trailer Revealed* — MAIET's statement that K-style would not return in GunZ 2.

## What GunZ players do

### Base actions (as designed)

| Action | What the player does | What it gives |
|---|---|---|
| Tumble / dash | Double-tap a direction | A burst of speed in that direction; a "locked" state during which the camera, weapon switching and blocking are restricted |
| Wall run | Approach a wall diagonally (running or dashing) and jump/hold jump | Run along the wall for a limited time, then leap off |
| Wall climb | Run straight at a wall and jump | Run up the wall, ending in a flip away from it |
| Wall jump | Jump off a wall | Redirects the character in the air |
| Guard / block (sword) | Hold block | Deflects bullets and melee; does not cover the whole body |
| Slash | Attack with a melee weapon | Damage; also the key to cancels (below) |
| Sword dash / heavy attack | Melee special input | A lunging attack that moves the character |
| Weapon switch | Number keys | Swap between guns and melee instantly |

### Emergent techniques (not designed — found by players)

The game's real identity came from cancels. Per the sources, the slash animation interrupted many other animations and reset their counters:

- **Slash cancels locked states.** Slashing during a dash or a wall leap ends the lock, giving back camera, weapon switch and block.
- **Block or weapon switch cancels the slash.** So slash → block becomes a near-free action.
- **Jump cancels dash recovery**, letting dashes chain faster.
- **Chains.** Jump → dash → slash → block ("butterfly") repeated keeps the player moving, evasive and threatening; skilled players stayed airborne across whole maps and fired guns faster than intended.
- **Wall cancel.** Slash out of the wall-climb backflip to bounce up walls faster.

### Why it feels good

- **Always moving.** The core principle cited everywhere: never stand still. Movement *is* defense.
- **Three-dimensional duels.** Fights happen on walls and in the air, not just on the floor.
- **Expression through sequencing.** Mastery is mostly input sequencing and rhythm, a "dance".
- **Guns and swords interlock.** Switching weapons is part of movement, not a menu action.

### What went wrong

- **The depth was an exploit.** Without cancels the game was described as slow and tedious; with them, it was the main attraction. The design depended on a bug.
- **Brutal learning curve.** Double-taps, number keys, mouse buttons and jump at once. New players met veterans doing things the game never taught.
- **Fragile.** Any animation or balance change could break the whole playstyle.
- **Hard to read.** Observers and newcomers couldn't tell what was happening or why.
- **GunZ 2 removed K-style** citing its "derivative problems"; veterans pushed back because it was the skill ceiling.

## SAEVUS design goals

These are SAEVUS's own targets, derived from the above.

1. **Movement first, stillness is risk.** Ground, wall and air movement must feel good on their own before combat exists, and standing still should rarely be the best option.
2. **Cancels are a designed system, not a bug.** Every action declares *when* it can be cancelled and *into what*, as data (cancel windows on simulation time). Cancels that make play richer are kept on purpose. Cancels that break things are simply not in the table.
3. **Low floor, high ceiling.** Every base action is one input (a dedicated dash key; double-tap only as an option). Depth comes from timing and chaining, not from finger gymnastics.
4. **Walls are part of the arena.** Wall run, wall climb and wall jump are first-class movement modes with their own tuning, and the Sandbox tests them.
5. **Air is a play space, with limits we choose.** Air actions draw from explicit budgets (e.g. air dashes that refill on ground or wall contact). Whether skilled players can stay airborne indefinitely is a deliberate design value, not an accident.
6. **Melee and guns interlock.** Weapon switching is fast and can cancel some actions, under the same cancel table as everything else (Phases 4–5).
7. **Readable.** Each action has a distinct silhouette and timing a spectator can follow. Matters for PvE telegraphs, bosses and PvP.
8. **Network-ready.** Every ability is driven by inputs + saved-move state, server-authoritative and client-predicted, so Phase 9 doesn't rewrite it.
9. **Teach it.** The Sandbox (and later tutorial spaces) shows techniques on purpose, like skill maps, instead of leaving them to be discovered.

## Proposal for Phase 3 (advanced movement)

Order, each with its own tuning struct, tests, Sandbox geometry and saved-move data:

1. **Dash** — directional burst on a dedicated input, ground and air; short action lock; air dashes from a budget refilled on landing / wall contact. Establishes the action-lock + cancel-window plumbing.
2. **Wall jump** — jump input near a wall while falling pushes off along the wall normal; refills or consumes the air budget (to decide).
3. **Wall run** — entered from a diagonal approach while airborne with jump held; runs along the wall for a limited time with reduced gravity; exits by jump (leap off), timeout or losing the wall.
4. **Wall climb** — entered running straight at a wall; runs upward for a limited height, then flips away (or wall-jumps if pressed).

Architecture: `MOVE_Custom` sub-modes (`ESAEVUSCustomMovementMode::WallRun`, `WallClimb`) inside `USAEVUSMovementComponent`; dash as a timed state inside Walking/Falling. A small action-lock / cancel-window model is shared with the Phase 4 combat component. Details go in `movement.md` as each ability lands.

## Questions for the owner

1. Which techniques did you actually use and love? Which felt like noise?
2. Should a skilled player be able to stay airborne indefinitely, or should the air budget always run out?
3. Dash on a dedicated key, double-tap, or both?
4. During a dash, should the camera lock (as in GunZ) or stay free?
5. Overall speed: closer to GunZ's pace, or slower and heavier to fit the dark tone?
6. Block: should it cover only part of the body (directional risk) or be simple?
