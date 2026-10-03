# SAEVUS combat

Status: Phase 4 (melee foundation) done; feel tuning and animations pending. Firearms and weapon switching are Phase 5.

Design goals come from `research/gunz.md`: cancels are a **designed, data-driven system**, every base action is one input, and movement and melee interlock.

## Pieces

| Piece | Role |
|---|---|
| `USAEVUSCombatComponent` (`Combat/`) | Runs attack timelines, buffers attack presses, chains combos, detects hits, answers "may this movement action cancel the attack now?" |
| `FSAEVUSAttackDefinition` (`Combat/SAEVUSCombatTypes.h`) | One attack: phases, damage, hit sphere, lunge, ground speed scale, chain point, cancel windows |
| `FSAEVUSMeleeMoveset` / `USAEVUSMeleeMovesetAsset` | Ground chain, air chain, input buffer, dash interaction. The struct constructor holds the default moveset; `DA_MeleeMoveset_Default` overrides it |
| `ASAEVUSTrainingDummy` (`World/`) | Pawn-sized target that counts hits and damage and shows them above itself. Not an enemy |
| `USAEVUSMovementComponent` hooks | `StartLunge`, `SetActionSpeedScale`, `SnapFacing`, `CancelDash` |
| `ASAEVUSCharacter` | Routes `IA_Attack`; asks combat before a dash or jump goes through |

Input: `IA_Attack` = left mouse button / gamepad X (face button left).

## Attack timeline

No attack animations exist yet (engine template content has none), so the timeline itself is the attack. Times are seconds from the attack's start:

```
0 ──── Startup ────┬── Active ──┬──────── Recovery ────────┐ total
                   │ hit sphere │                          │
                   │    live    │  ChainFrom ──► next attack may start
 lunge ─────────►  (LungeDuration)
 DashCancelFrom / JumpCancelFrom ──► that action may cut the attack short
```

- **Start.** An attack press is buffered for `InputBufferTime` and starts as soon as allowed: on the ground or in the air (not on walls). A press during a dash ends the dash when `bAttackEndsDash` is on. The first attack comes from the air chain if the character is falling, otherwise from the ground chain.
- **Facing and lunge.** The attack faces the movement input direction (or straight ahead without input), snaps the body to it, and starts the lunge: horizontal speed held at `LungeSpeed` for `LungeDuration`. This is the "sword-driven movement" hook.
- **Committed movement.** While the attack runs, the ground speed cap is multiplied by `GroundMoveSpeedScale` (the lunge is not limited by it). Air momentum is untouched.
- **Hits.** During the active phase a sphere (`HitRadius`, at `HitForwardOffset` ahead and `HitHeightOffset` up) overlaps pawns, physics bodies and dynamic objects. Each actor is hit once per attack, through the engine damage path (`UGameplayStatics::ApplyDamage` → `TakeDamage`), and `OnAttackHit` fires for future feedback (hit-stop, VFX, sound).
- **Chain.** A buffered press starts the next attack of the same chain once the running one reaches `ChainFrom`. After the last attack, the chain ends; the next press starts again from the first.
- **End.** At the end of recovery, or when cancelled, or when wall movement takes over.

## Cancel rules

Each attack declares, per movement action, from when it may be cut short (negative = never). The character asks `TryCancelForAction` before passing a dash or jump to movement; outside the window the press is dropped.

| Attack | Startup / Active / Recovery (s) | Damage | Lunge | Chain from | Dash cancel from | Jump cancel from |
|---|---|---|---|---|---|---|
| Slash1 | 0.10 / 0.08 / 0.30 | 10 | 450 cm/s × 0.12 s | 0.18 | 0.18 | 0.18 |
| Slash2 | 0.10 / 0.08 / 0.30 | 12 | 450 cm/s × 0.12 s | 0.18 | 0.18 | 0.18 |
| Finisher | 0.18 / 0.12 / 0.45 | 25 | 650 cm/s × 0.18 s | (last) | 0.40 | never |
| AirSlash | 0.08 / 0.10 / 0.25 | 12 | none | 0.18 | 0.18 | 0.18 |

Ground speed scale: 0.25 for the slashes, 0 for the finisher. Hit sphere: 60 cm radius 90 cm ahead (finisher 75 cm, 100 cm ahead). Input buffer 0.2 s. First-pass values, not tuned by feel.

Reading: the quick slashes can be left as soon as they've hit (end of active), so they slot into movement. The finisher commits: a jump can't get out of it, and a dash only late in its recovery.

Designed cancel pairs so far: attack → dash/jump (table above), dash → attack (`bAttackEndsDash`), jump during a ground dash (movement), wall jump during a dash (movement). Weapon switching joins the table in Phase 5.

## Networking

Not yet. Combat state is local; the lunge and speed scale are not part of the saved move. Phase 9 moves the attack request and timeline into the predicted move data, the same way the dash already works.

## Debug

- F6 HUD: `Attack: <name> #<index> <phase> <time>` and `(buffered)`.
- `SAEVUS.Debug.Combat 1`: draws the hit sphere while active (yellow: no hit, red: hit).
- Sandbox: three dummies near the start (`Dummy_A` alone at X 6 m, `Dummy_B1` / `Dummy_B2` side by side).
- The rendered smoke test runs one attack and checks its duration against the data.

## Tests (`SAEVUS.Combat.*`)

| Test | Checks |
|---|---|
| `Timeline` | starts on the press; startup, active and total durations match the data (±1 step) |
| `HitsDummy` | a dummy in range is hit exactly once for the attack's damage; out of range not at all |
| `Combo` | buffered presses chain 1 → 2 → 3; the chain ends; an expired press doesn't chain |
| `Cancels` | dash refused during startup, accepted in its window (attack ends, dash starts); finisher refuses jump |
| `Movement` | attack faces the input; lunge distance ≈ speed × duration; slowed ground speed during the attack, full cap after |
| `AirAttack` | an attack in the air uses the air chain and doesn't stop the jump |
| `AttackEndsDash` | an attack pressed during a dash ends the dash |

## Not in this phase

Attack animations and montages (asset pipeline), hit reactions, hit-stop, health and death (Phase 7 shares a health model between player and enemies), blocking, weapon switching and firearms (Phase 5).
