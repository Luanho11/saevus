# SAEVUS movement

Status: Phase 3 (advanced movement) done; feel tuning pending. Walk, run, sprint, jump, air control, coyote time, jump buffer, dash, wall jump, wall run, wall climb. Built on `UCharacterMovementComponent` (CMC).

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
- **Air budget.** `MaxAirDashes` per airborne period, refilled on landing and (by default) by a wall jump.
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

## Wall jump

Phase 3, second ability. No new input: it is the jump pressed while airborne next to a wall.

- **Trigger.** `UpdateCharacterStateBeforeMovement` → `TryWallJump()`: the jump was pressed on this update (`bPressedJump` with `JumpKeyHoldTime` still 0, and the press didn't already do a normal or coyote jump), the character is falling, and wall jumps are left. Coyote time therefore wins over a wall jump, and a held jump key never repeats it.
- **Wall detection.** `FindWallJumpNormal()`: eight horizontal capsule sweeps of `DetectionDistance`. A hit counts if its normal is near horizontal (`|Z| <= MaxWallNormalZ`) and it touches the capsule's straight side, not its lower hemisphere. That last rule keeps the edge you just walked off from counting as a wall (found by the coyote-time test). The nearest wall wins.
- **Launch.** Velocity = wall normal × `AwaySpeed` + the speed along the wall × `TangentialSpeedKept`, vertical = `UpSpeed`. Ends a dash in progress, clears the jump buffer, snaps the body to the new direction (`bFaceJumpDirection`).
- **Budget.** `MaxWallJumps` per airborne period, refilled on landing. With `bRefillsAirDashes` a wall jump gives the air dashes back, so jump → dash → wall jump → dash chains.

| Value (`FSAEVUSWallJumpTuning`) | Default | Notes |
|---|---|---|
| AwaySpeed | 650 cm/s | |
| UpSpeed | 750 cm/s | a bit more than the ground jump (700) |
| TangentialSpeedKept | 0.5 | |
| MaxWallJumps | 3 | 0 disables wall jumps |
| DetectionDistance | 30 cm | gap between capsule and wall |
| MaxWallNormalZ | 0.3 | about 72 deg; steeper surfaces are walls |
| bRefillsAirDashes | true | |
| bFaceJumpDirection | true | |

First-pass values. In the Sandbox chimney (3 m gap) each hop gains about 1.5 m, so three wall jumps plus the ground jump reach the 4.5 m ledge.

## Wall run

Phase 3, third ability, and the first custom movement mode: `MOVE_Custom` + `ESAEVUSCustomMovementMode::WallRun`. Code in `SAEVUSMovementComponentWalls.cpp` (with wall detection and the wall jump). No new input.

- **Entry** (`TryStartWallRun`, each update while falling, skipped while dashing or on the update of a wall jump): movement input present and not pointing away from the wall (`DetachInputDot`); a wall within `DetectionDistance` beside the motion or diagonally ahead (4 capsule sweeps, same wall rules as the wall jump); speed along it at least `MinEntrySpeed`; not falling faster than `MaxEntryFallSpeed`; not the wall last run on (see below).
- **On entry.** Direction = the velocity along the wall. Falling speed is cancelled and upward speed capped to `MaxEntryUpSpeed`, so a jump into a wall gives a short rising arc. The body faces along the wall. Air dashes refill (`bRefillsAirDashes`).
- **While running** (`PhysWallRun`): speed along the wall is held at `Speed`; gravity is the character's gravity × `GravityScale`; the wall is re-probed each update and its normal followed (curved walls), and the capsule is pulled back to a 1 cm gap from it.
- **Exits.** Time (`MaxDuration`) → fall, keeping the run's speed. Wall ends or is lost → fall. Input away from the wall → fall. Coming down onto a floor → landing. Something blocking the way → fall. Jump → wall jump off the wall being run on (wall-jump tuning and budget; the run's speed along the wall counts as the tangential part).
- **Same wall.** The wall last run on can't be run again until landing or running a different wall. Alternating between two facing walls works; climbing one wall forever doesn't.
- No dash during a wall run (the dash needs walking or falling).
- Animation: the template AnimBP doesn't know the custom mode, so it plays ground locomotion at the run speed. A proper wall-run pose belongs to the animation work.

| Value (`FSAEVUSWallRunTuning`) | Default | Notes |
|---|---|---|
| Speed | 750 cm/s | |
| MaxDuration | 1.2 s | 0 disables wall running |
| GravityScale | 0.3 | of the character's gravity: falls about 3.7 m over a full run from rest |
| MinEntrySpeed | 300 cm/s | along the wall |
| MaxEntryUpSpeed | 300 cm/s | |
| MaxEntryFallSpeed | 600 cm/s | |
| DetectionDistance | 40 cm | |
| MaxWallNormalZ | 0.3 | |
| DetachInputDot | 0.7 | |
| bRefillsAirDashes | true | |

First-pass values, not tuned by feel.

## Wall climb

Phase 3, last ability: `MOVE_Custom` + `ESAEVUSCustomMovementMode::WallClimb`, in `SAEVUSMovementComponentWalls.cpp`. No new input.

- **Entry** (`TryStartWallClimb`, checked before the wall run each update, so a head-on approach climbs and a glancing one runs): falling, not dashing, climbs left; a wall straight ahead of the motion within `DetectionDistance`; both motion and input head-on (`MinHeadOnDot`); speed toward the wall at least `MinEntrySpeed`; not falling faster than `MaxEntryFallSpeed`. In practice: run at a wall, jump, keep pushing into it.
- **While climbing** (`PhysWallClimb`): constant upward `Speed`, no gravity, body faces the wall, capsule kept at a 1 cm gap (same helpers as the wall run: `ProbeCurrentWall`, `MoveAlongWall`).
- **Exits.** Time (`MaxDuration`) → flip away (`FlipAwaySpeed` out, `FlipUpSpeed` up, body turned away). Top of the wall reached (the wall no longer touches the capsule's straight side) → forward onto it at `LedgeForwardSpeed` while still rising, so the character lands on top. Input away → let go and drop. Ceiling → drop. Jump → wall jump off it.
- **Budget.** `MaxClimbs` per airborne period (default 1), refilled on landing. Starting a climb refills the air dashes.
- Animation: the template AnimBP sees a custom mode with no horizontal speed and plays idle. A climb pose belongs to the animation work.

| Value (`FSAEVUSWallClimbTuning`) | Default | Notes |
|---|---|---|
| Speed | 650 cm/s | |
| MaxDuration | 0.55 s | about 3.4 m measured (3.6 m nominal; the last step is the flip) |
| MaxClimbs | 1 | |
| MinEntrySpeed | 200 cm/s | toward the wall |
| MinHeadOnDot | 0.85 | about 32 deg |
| MaxEntryFallSpeed | 600 cm/s | |
| DetectionDistance | 40 cm | |
| MaxWallNormalZ | 0.3 | |
| DetachInputDot | 0.7 | |
| FlipAwaySpeed / FlipUpSpeed | 350 / 250 cm/s | |
| LedgeForwardSpeed | 300 cm/s | |
| bRefillsAirDashes | true | |

First-pass values. With a ground jump first, a 3.5 m wall is topped (the test character ends standing on it).

## Networking readiness

`FSAEVUSSavedMove` stores the sprint and walk requests. They travel as `FLAG_Custom_0` (sprint) and `FLAG_Custom_1` (walk) in the compressed flags and are restored in `UpdateFromCompressedFlags`. Moves with different requests are never combined. When replication is turned on, gait stays client-predicted with no extra work. The dash request travels as `FLAG_Custom_2`; the saved move also stores the dash state at the start of the move (direction, time left, cooldown, air dashes used) and restores it in `PrepMoveFor`, so a replay after a correction re-simulates the dash exactly. Moves that press dash or start/end a dash are never combined. The wall jump needs no flag: it is driven by the replicated jump press (`FLAG_JumpPressed`; the server resets `JumpKeyHoldTime` on each new press), and the wall-jump count is saved and restored like the dash state. The wall-run / wall-climb state (wall normal, run direction, time left, last run wall, climbs used) is saved and restored the same way; the movement mode itself is engine-replicated. `FLAG_Custom_3` is still free. Abilities that need more state should use the CMC move-container / network data API instead of flags.

Known limitation: the air gait and the coyote/buffer timers are local simulation state derived from mode transitions, not part of the saved move. After a server correction they can be off for one transition. The jump itself still travels through the normal `bPressedJump` path, so the server stays authoritative.

## Future abilities (not implemented)

Sword-driven movement and aerial combat come with the combat phases and will plug into the same component through the cancel rules described in `research/gunz.md`. Each gets its own tuning struct and tests.

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
| `WallJump.Basic` | wall found; pushed away at AwaySpeed; UpSpeed; faces away; one jump spent |
| `WallJump.NoWall` | jump in open air does nothing |
| `WallJump.NotOffLedge` | the ledge just walked off is not a wall (coyote time off) |
| `WallJump.Tangential` | keeps TangentialSpeedKept of the speed along the wall |
| `WallJump.Budget` | second jump refused with budget 1; landing refills |
| `WallJump.RefillsAirDash` | ends the air dash; refills air dashes only when enabled |
| `WallRun.Enter` | falling along a wall with input starts it; speed; facing; stays at the wall |
| `WallRun.EntryConditions` | refused with no input, too slow, input away, falling too fast |
| `WallRun.Duration` | lasts MaxDuration (72 steps); then falls; same wall not re-run |
| `WallRun.Landing` | sliding down onto the floor lands |
| `WallRun.WallEnds` | running off the end of the wall falls, keeping speed |
| `WallRun.JumpOff` | jump = wall jump off the run wall, keeps half the run speed |
| `WallRun.Detach` | input away from the wall drops off |
| `WallClimb.Enter` | head-on jump at a wall with input climbs; speed; faces wall; one climb spent |
| `WallClimb.EntryConditions` | refused with no input, input along the wall, glancing approach, too slow; accepted head-on |
| `WallClimb.Flip` | climbs about Speed x MaxDuration (340 vs 358 cm), then flips away facing out |
| `WallClimb.Ledge` | a 3.5 m wall is topped and the character lands on it |
| `WallClimb.OncePerAir` | no second climb before landing; landing refills |
| `WallClimb.JumpOff` | jump = wall jump off the climbed wall |
| `WallClimb.LetGo` | input away drops off |
| `JumpAssist.AirGaitLock` | sprint mid-air keeps the run cap when locked, raises it when unlocked; a jump take-off opens no coyote window |

The simulation tests tick a real `ASAEVUSCharacter` in a throwaway world (`Tests/SAEVUSMovementTestWorld.h`: no map, no controller, `bRunPhysicsWithNoController`, engine-cube platforms). The fixture ticks the movement component directly and advances world time itself, starting at 10 s: in editor builds CMC refuses to start falling during the first second of world time.

In-game check: `UnrealEditor.exe SAEVUS.uproject -game -windowed -ResX=1280 -ResY=720 -SAEVUSSmokeTest` runs, sprints, jumps and dashes on its own, then logs `[SAEVUS smoke] PASSED/FAILED`, a frame-time summary and a screenshot. `Tools\Run-SmokeTest.ps1` wraps it; frame-time numbers are in `performance.md`.

## Sandbox (`/Game/SAEVUS/Maps/L_Sandbox`)

100 × 100 m grid floor, plus:

- steps 50–250 cm for jump height;
- slopes at 20°, 35° and 50° (the default walkable limit is about 44.8°);
- 200 cm decks with gaps of 2–6 m for jump distance and air control;
- a 30 m × 6 m wall reserved for wall running;
- a wall-jump chimney at X = -30 m: two 7 m walls with a 3 m gap, and a 4.5 m ledge at its far end;
- a wall-run corridor at Y = -30 m: two 20 m × 6 m walls, 5 m apart;
- wall-climb blocks at X = 30 m: a 3.5 m block to climb onto, and a 10 m wall that ends every climb in a flip.

Press F6 to show gait, speed, vertical speed, movement mode and location.
