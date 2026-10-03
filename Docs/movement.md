# SAEVUS movement

Status: Bootstrap. Walk, run, sprint, jump and air control. Built on `UCharacterMovementComponent` (CMC).

## Why CMC and not Mover

CMC is mature: it has client prediction, server correction, root motion and custom movement modes, and it's what most shipped UE action games use. The Mover plugin may become the better choice later. Switching needs a concrete reason, for example a CMC limitation hit by a planned ability.

## Gait

`ESAEVUSGait` = Walk / Run / Sprint. The player *requests* sprint and walk (held keys). `GetCurrentGait()` resolves the requests:

- sprint requested → **Sprint** (sprint wins over walk);
- walk requested → **Walk**;
- neither → **Run** (the default).

`GetMaxSpeed()` returns the gait speed while walking on the ground **and while falling**. The air speed cap matches the ground gait, so a sprint jump keeps its momentum instead of being clamped to run speed. `MaxWalkSpeed` mirrors the run speed for systems that don't know about gait (AI, navigation).

## Tuning

All values live in `Source/SAEVUS/Movement/SAEVUSMovementSettings.h`. The struct initializers are the defaults; `DA_MovementSettings_Default` (a `USAEVUSMovementSettings`) overrides them per character. `ApplyMovementSettings()` pushes them into the CMC on `InitializeComponent` and on `SetMovementSettings()`.

| Value | Default | Notes |
|---|---|---|
| WalkSpeed / RunSpeed / SprintSpeed | 250 / 550 / 850 cm/s | |
| MaxAcceleration | 2400 cm/s² | |
| BrakingDeceleration | 2000 cm/s² | no input on the ground |
| GroundFriction | 8 | direction-change snappiness |
| TurnRate | 720 deg/s | orient-to-movement |
| JumpZVelocity | 700 cm/s | apex = v² / 2g = 700² / (2 × 980 × 1.75) ≈ 143 cm |
| JumpMaxHoldTime | 0 s | fixed-height jump |
| GravityScale | 1.75 | heavier, snappier arc than the engine's 1.0 |
| AirControl | 0.35 | fraction of ground acceleration in the air |
| AirControlBoostMultiplier / Threshold | 2 / 25 cm/s | easier direction change from a near-vertical jump |
| FallingLateralFriction / BrakingDecelerationFalling | 0 / 0 | the take-off momentum is kept |

These are first-pass values, not tuned by feel yet. Tuning happens in the editor on the Data Asset, not in code.

## Networking readiness

`FSAEVUSSavedMove` stores the sprint and walk requests. They travel as `FLAG_Custom_0` (sprint) and `FLAG_Custom_1` (walk) in the compressed flags and are restored in `UpdateFromCompressedFlags`. Moves with different requests are never combined. When replication is turned on, gait stays client-predicted with no extra work. `FLAG_Custom_2` and `FLAG_Custom_3` are still free. Abilities that need more state should use the CMC move-container / network data API instead of flags.

## Future abilities (not implemented)

Wall run, wall jump, dash, sword-driven movement and aerial combat will be **custom movement modes** (`MOVE_Custom` + a SAEVUS sub-mode enum) inside `USAEVUSMovementComponent`, or small helper objects it owns, each with its own tuning struct. Their exact behaviour depends on the GunZ research phase. Nothing is designed from GunZ mechanics yet.

## Open design questions

- **Sprint in the air:** the air speed cap follows the *current* gait, so pressing sprint mid-air raises the cap to sprint speed. Alternatives: latch the gait at take-off, or cap air speed at the take-off speed. Decide during the movement-feel pass.
- **Sprint direction:** sprint currently works in any direction, including backwards. Restricting it to a forward cone is a feel decision for the same pass.
- **Animation:** the template `ABP_Unarmed` blend space tops out near jog speed, so sprint plays the run cycle faster and the feet slide. Proper gait animation belongs to the animation phase.

## Tests

`Source/SAEVUS/Tests/SAEVUSMovementTests.cpp` (filter `SAEVUS.Movement`):

| Test | Checks |
|---|---|
| `Gait.Priority` | run by default; walk; sprint beats walk; release returns to run |
| `Gait.Speeds` | gait speeds come from tuning; walk < run < sprint |
| `Settings.Apply` | a Data Asset overrides CMC values; null restores defaults |
| `Air.JumpApex` | simulated apex height within 2 % of v² / 2g |
| `Air.AirControl` | air acceleration ≈ MaxAcceleration × AirControl (±10 %); AirControl 0 keeps zero lateral speed |

The two `Air.*` tests tick a real `ASAEVUSCharacter` in a throwaway world (no map, no controller, `bRunPhysicsWithNoController`).

## Sandbox (`/Game/SAEVUS/Maps/L_Sandbox`)

100 × 100 m grid floor, plus:

- steps 50–250 cm for jump height;
- slopes at 20°, 35° and 50° (the default walkable limit is about 44.8°);
- 200 cm decks with gaps of 2–6 m for jump distance and air control;
- a 30 m × 6 m wall reserved for later wall movement.

Press F6 to show gait, speed, vertical speed, movement mode and location.
