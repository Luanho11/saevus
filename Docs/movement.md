# SAEVUS movement

Status: Phase 3 (advanced movement) in progress. Walk, run, sprint, jump, air control, coyote time, jump buffer, dash. Built on `UCharacterMovementComponent` (CMC).

## Why CMC and not Mover

CMC is mature: it has client prediction, server correction, root motion and custom movement modes, and it's what most shipped UE action games use. The Mover plugin may become the better choice later. Switching needs a concrete reason, for example a CMC limitation hit by a planned ability.

## Gait

`ESAEVUSGait` = Walk / Run / Sprint. The player *requests* sprint and walk (held keys). `GetCurrentGait()` resolves the requests:

- sprint requested → **Sprint** (sprint wins over walk);
- walk requested → **Walk**;
- neither → **Run** (the default).

`GetMaxSpeed()` returns the gait speed while walking on the ground **and while falling**, so a sprint jump keeps its momentum instead of being clamped to run speed. In the air the cap uses the **take-off gait** (`GetAirGait()`) while `bLockGaitInAir` is on (default): pressing or releasing sprint mid-air changes nothing until landing. `MaxWalkSpeed` mirrors the run speed for systems that don't know about gait (AI, navigation).

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
| bLockGaitInAir | true | air speed cap = take-off gait |
| CoyoteTime | 0.12 s | ground jump still allowed after walking off a ledge |
| JumpBufferTime | 0.12 s | a jump pressed this long before landing fires on touchdown |

These are first-pass values, not tuned by feel yet. Tuning happens in the editor on the Data Asset, not in code.

## Jump assists

Generic platformer techniques (not taken from GunZ), all in `USAEVUSMovementComponent` and tunable per Data Asset:

- **Coyote time.** Walking off a ledge (leaving `Walking` for `Falling` without a jump press) opens a `CoyoteTime` window. `ASAEVUSCharacter::CanJumpWhileFalling()` (the engine hook for "first jump while falling") returns true inside it, so the normal jump path runs. A jump take-off never opens the window.
- **Jump buffer.** `ASAEVUSCharacter::RequestJump()` is the single jump entry point (input binds to it). Pressed while airborne and outside the coyote window, it stores a `JumpBufferTime` timer. When the movement mode becomes `Walking` with the timer still running, the component calls `Jump()` after the engine has reset the jump state, and the jump fires on the next update.
- Timers count down in `OnMovementUpdated`, so they follow simulation time (also during replays), not wall-clock time.

## Dash

Phase 3, first ability (design goals in `research/gunz.md`). Input `IA_Dash` (Left Ctrl, mouse thumb button, gamepad B), handled in `USAEVUSMovementComponent`:

- **Start.** `RequestDash()` sets a request consumed at the next movement update (`UpdateCharacterStateBeforeMovement`). If the dash isn't allowed then (already dashing, cooldown, crouching, no air dashes left), the press is dropped, not queued.
- **Direction.** The movement input direction at that update (`Acceleration`, which is replicated with the move), or the facing with no input. Locked for the whole dash. With `bFaceDashDirection` the body snaps to it.
- **During.** `CalcVelocity` holds horizontal velocity at `Speed`: no steering, friction or braking. `GetMaxSpeed()` returns `Speed`. In the air, `GetGravityZ()` returns 0 while `bSuspendGravityDuringDash` is on, so the vertical speed set at the start (`AirDashVerticalSpeed`, 0 = flat) is held. Walking off a ledge mid-dash keeps dashing across the gap.
- **End.** After `Duration`, or when a jump cancels a ground dash: horizontal speed is capped to `ExitSpeed`, then `Cooldown` starts.
- **Air budget.** `MaxAirDashes` per airborne period, refilled on landing. Whether wall contact also refills it is decided with wall movement.
- **Designed cancel.** Jump during a ground dash: the dash ends and the jump leaves at `ExitSpeed`. This is the first entry of the cancel rules from the research doc; more come with combat.

| Value (`FSAEVUSDashTuning`) | Default | Notes |
|---|---|---|
| Speed | 1600 cm/s | |
| Duration | 0.18 s | about 2.9 m per dash |
| ExitSpeed | 700 cm/s | between run and sprint |
| Cooldown | 0.3 s | from the end of the dash |
| bFaceDashDirection | true | |
| MaxAirDashes | 1 | 0 disables air dash |
| AirDashVerticalSpeed | 0 cm/s | |
| bSuspendGravityDuringDash | true | |

First-pass values, not tuned by feel. Open owner questions that may change this: dedicated key vs double-tap, camera lock during a dash, whether air time should be unlimited for skilled play.

## Networking readiness

`FSAEVUSSavedMove` stores the sprint and walk requests. They travel as `FLAG_Custom_0` (sprint) and `FLAG_Custom_1` (walk) in the compressed flags and are restored in `UpdateFromCompressedFlags`. Moves with different requests are never combined. When replication is turned on, gait stays client-predicted with no extra work. The dash request travels as `FLAG_Custom_2`; the saved move also stores the dash state at the start of the move (direction, time left, cooldown, air dashes used) and restores it in `PrepMoveFor`, so a replay after a correction re-simulates the dash exactly. Moves that press dash or start/end a dash are never combined. `FLAG_Custom_3` is still free. Abilities that need more state should use the CMC move-container / network data API instead of flags.

Known limitation: the air gait and the coyote/buffer timers are local simulation state derived from mode transitions, not part of the saved move. After a server correction they can be off for one transition. The jump itself still travels through the normal `bPressedJump` path, so the server stays authoritative.

## Future abilities (not implemented)

Wall run, wall jump, dash, sword-driven movement and aerial combat will be **custom movement modes** (`MOVE_Custom` + a SAEVUS sub-mode enum) inside `USAEVUSMovementComponent`, or small helper objects it owns, each with its own tuning struct. Their exact behaviour depends on the GunZ research phase. Nothing is designed from GunZ mechanics yet.

## Open design questions

- ~~Sprint in the air~~ — decided (Phase 1): the take-off gait is locked in the air (`bLockGaitInAir`, can be turned off).
- **Sprint direction:** with orient-to-movement the body always faces where it runs, so there is no "backwards sprint" to restrict. Revisit if a strafing / aim mode is added (firearms phase).
- **Landing recovery:** no landing slowdown yet. It needs landing animations to read well; left for the animation work.
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
| `JumpAssist.CoyoteTime` | jump right after walking off a real ledge works; refused with CoyoteTime 0 |
| `JumpAssist.JumpBuffer` | jump pressed 15 cm above the floor fires on landing; dropped with JumpBufferTime 0 |
| `Dash.Ground` | dash speed from tuning; facing direction with no input; distance about Speed x Duration (10 % tolerance); exit speed cap |
| `Dash.Direction` | follows input, not facing; body snaps to it; steering mid-dash is ignored |
| `Dash.Cooldown` | no dash while dashing or during cooldown; allowed after it |
| `Dash.AirBudget` | one air dash; height held (gravity off); second refused; landing refills |
| `Dash.JumpCancel` | jump ends a ground dash and leaves at exit speed |
| `JumpAssist.AirGaitLock` | sprint mid-air keeps the run cap when locked, raises it when unlocked; a jump take-off opens no coyote window |

The simulation tests tick a real `ASAEVUSCharacter` in a throwaway world (`Tests/SAEVUSMovementTestWorld.h`: no map, no controller, `bRunPhysicsWithNoController`, engine-cube platforms). The fixture ticks the movement component directly and advances world time itself, starting at 10 s: in editor builds CMC refuses to start falling during the first second of world time.

In-game check: `UnrealEditor.exe SAEVUS.uproject -game -windowed -ResX=1280 -ResY=720 -SAEVUSSmokeTest` runs, sprints, jumps and dashes on its own, then logs `[SAEVUS smoke] PASSED/FAILED`, a frame-time summary and a screenshot. `Tools\Run-SmokeTest.ps1` wraps it; frame-time numbers are in `performance.md`.

## Sandbox (`/Game/SAEVUS/Maps/L_Sandbox`)

100 × 100 m grid floor, plus:

- steps 50–250 cm for jump height;
- slopes at 20°, 35° and 50° (the default walkable limit is about 44.8°);
- 200 cm decks with gaps of 2–6 m for jump distance and air control;
- a 30 m × 6 m wall reserved for later wall movement.

Press F6 to show gait, speed, vertical speed, movement mode and location.
