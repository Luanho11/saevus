# SAEVUS combat

Status: Phases 4 (melee foundation) and 5 (firearms, weapon switching) done; feel tuning, animations and weapon models pending.

Design goals come from `research/gunz.md`: cancels are a **designed, data-driven system**, every base action is one input, and movement and melee interlock.

## Pieces

| Piece | Role |
|---|---|
| `USAEVUSCombatComponent` (`Combat/`) | Runs attack timelines, buffers attack presses, chains combos, detects hits, answers "may this movement action cancel the attack now?" |
| `FSAEVUSAttackDefinition` (`Combat/SAEVUSCombatTypes.h`) | One attack: phases, damage, hit sphere, lunge, ground speed scale, chain point, cancel windows |
| `FSAEVUSMeleeMoveset` / `USAEVUSMeleeMovesetAsset` | Ground chain, air chain, input buffer, dash interaction. The struct constructor holds the default moveset; `DA_MeleeMoveset_Default` overrides it |
| `ASAEVUSTrainingDummy` (`World/`) | Pawn-sized target that counts hits and damage and shows them above itself. Not an enemy |
| `USAEVUSMovementComponent` hooks | `StartLunge`, `SetActionSpeedScale`, `SnapFacing`, `CancelDash` |
| `USAEVUSWeaponComponent` (`Weapons/`) | Loadout by slot, switching with equip time, per-slot magazines, trigger, reload, hitscan shots |
| `USAEVUSWeaponDefinition` (`Weapons/SAEVUSWeaponTypes.h`) | One weapon: melee (with its moveset) or firearm (`FSAEVUSFirearmTuning`), plus equip time |
| `ASAEVUSCharacter` | Routes input; asks combat before a dash, jump or weapon switch goes through |

Input: `IA_Attack` (left mouse / gamepad X) is the primary action: melee attack, or the trigger of an equipped firearm (held for automatic fire). `IA_Reload` (R / gamepad Y), `IA_NextWeapon` (mouse wheel down / right shoulder), `IA_WeaponSlot1..4` (keys 1–4).

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

## Weapons and switching

The loadout is a list of `USAEVUSWeaponDefinition` assets on the weapon component. Default (`BP_SAEVUSCharacter`): 1 Sword, 2 Pistol, 3 Rifle, 4 Shotgun.

- **Switching.** Selecting a slot starts its `EquipTime`; nothing can be fired or swung until it ends. A switch drops a reload in progress and any queued shot. Mid-attack, a switch obeys the attack's `SwitchCancelFrom` window, like dash and jump. Equipping a melee weapon hands its moveset to the combat component (null = code default).
- **Firing.** A trigger press asks for one shot (a press during the cooldown fires when it ends); an automatic weapon keeps firing while held, at `FireInterval` (the cooldown carries at most one frame of overshoot, so the rate holds at any frame rate). Each slot keeps its own magazine; a dry trigger reloads. Shooting never locks movement: dash, jump and wall moves stay free.
- **Hitscan, two traces per pellet.** (1) From the view (camera for a player, eyes otherwise) along the aim, to find the aim point. (2) From the muzzle (`MuzzleOffset` on the owner) toward that point, up to `Range`. Cover in front of the character therefore blocks shots the camera could see past. Damage goes through `ApplyPointDamage`. The body snaps to face the aim when firing.
- **Collision.** Shots use the project's **Weapon** trace channel (`ECC_GameTraceChannel1`, `Config/DefaultEngine.ini`, `Core/SAEVUSCollisionChannels.h`). It blocks by default, including pawns, which ignore the Visibility channel.
- **Spread.** Each pellet is placed randomly in a cone of `SpreadDegrees` (from a component-owned random stream, ready to be seeded per shot for networking).

| Weapon (`DA_Weapon_*`) | Kind | Damage | Pellets | Interval | Mode | Magazine | Reload | Range | Spread | Equip |
|---|---|---|---|---|---|---|---|---|---|---|
| Sword | melee (default moveset) | — | — | — | — | — | — | — | — | 0.2 s |
| Pistol | firearm | 20 | 1 | 0.25 s | semi | 12 | 1.2 s | 50 m | 0.5° | 0.25 s |
| Rifle | firearm | 9 | 1 | 0.1 s | auto | 30 | 1.8 s | 60 m | 1.5° | 0.4 s |
| Shotgun | firearm | 7 | 8 | 0.8 s | semi | 6 | 2.0 s | 20 m | 6° | 0.45 s |

Values live in the asset script's `WEAPONS` table and on the Data Assets; first-pass, generic archetypes, not tuned by feel. Reserve ammo, weapon pickups and inventory are out of scope (see `CLAUDE.md`).

No weapon models: external assets are not allowed yet, so firearms are invisible; `SAEVUS.Debug.Combat 1` draws each shot (red: hit). Weapon meshes come with the asset pipeline.

## Networking

Not yet. Combat and weapon state are local; the lunge and speed scale are not part of the saved move. Phase 9 moves attack, switch and fire requests into the predicted move data (the dash already works this way) and makes the server confirm hits; the spread stream will need a per-shot seed shared by client and server.

## Debug

- F6 HUD: `Attack: <name> #<index> <phase> <time>` and `(buffered)`.
- F6 HUD also shows `Weapon: <name> [slot/count] ammo n/m`, `(switching to n)`, `(reloading)`.
- `SAEVUS.Debug.Combat 1`: draws the hit sphere while active and each shot's trace (yellow: no hit, red: hit).
- Sandbox: three dummies near the start (`Dummy_A` alone at X 6 m, `Dummy_B1` / `Dummy_B2` side by side).
- Sandbox shooting range: dummies at 10, 20 and 30 m (X negative, Y = 10 m).
- The rendered smoke test runs one attack and checks its duration against the data, then switches to slot 2 and fires once.

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

`SAEVUS.Weapons.*`:

| Test | Checks |
|---|---|
| `Hitscan` | one shot hits the dummy for the data's damage and uses a round; out of range no hit; cover in front of the muzzle stops it |
| `FireModes` | semi-auto fires once per press however long held; automatic fires at its rate (10 shots in 1 s); every pellet is a hit |
| `Reload` | dry trigger reloads; no shots while reloading; magazine full after; no reload with a full magazine |
| `Switch` | no attack or shot while switching; firearm equipped after its equip time; primary action fires; reselecting does nothing; next weapon wraps to the sword, which attacks |
| `SwitchCancels` | switch refused before the attack's window, accepted in it (attack ends); switching drops a reload |
| `MeleeMoveset` | equipping a melee weapon gives combat that weapon's moveset |

## Not in this phase

Attack and firing animations and weapon models (asset pipeline), hit reactions, hit-stop, health and death (Phase 7 shares a health model between player and enemies), blocking, aiming mode / strafing, reserve ammo and pickups.
