# SAEVUS architecture

Status: Bootstrap. This file describes what exists in code. Planned systems are marked as such.

## Module and folders

One runtime module, `SAEVUS` (`Source/SAEVUS/`), split into feature folders. `SAEVUS.Build.cs` adds the module root to the include paths, so includes are written by folder: `#include "Movement/SAEVUSMovementComponent.h"`.

| Folder | Contents now | Reserved for |
|---|---|---|
| `Core/` | `ASAEVUSGameMode`, `ASAEVUSPlayerController` | game state, player state |
| `Character/` | `ASAEVUSCharacter`, `USAEVUSInputConfig` | |
| `Movement/` | `USAEVUSMovementComponent`, tuning structs, `USAEVUSMovementSettings`, `ESAEVUSGait` | movement abilities as custom modes |
| `Camera/` | `USAEVUSCameraComponent`, `FSAEVUSCameraTuning`, `USAEVUSCameraSettings` | |
| `Debug/` | `ASAEVUSDebugHUD`, debug CVars | |
| `Tests/` | `SAEVUS.Movement.*` automation tests | |
| `Abilities/`, `Animation/`, `Audio/`, `Bosses/`, `Combat/`, `Data/`, `Enemies/`, `Networking/`, `UI/`, `VFX/`, `Weapons/`, `World/` | empty | their own phases |

## Class map

```
ASAEVUSGameMode (AGameModeBase)          native fallbacks: ASAEVUSCharacter, ASAEVUSPlayerController, ASAEVUSDebugHUD
 └─ BP_SAEVUSGameMode                    swaps in BP_SAEVUSCharacter, BP_SAEVUSPlayerController

ASAEVUSPlayerController (APlayerController)
 ├─ adds mapping contexts: IMC_Default, IMC_MouseLook (engine template), IMC_SAEVUS
 └─ binds controller-level actions: IA_ToggleDebugHUD (F6)

ASAEVUSCharacter (ACharacter)            routes input; owns no gameplay rules
 ├─ USAEVUSMovementComponent             gait, tuning, saved moves      ← USAEVUSMovementSettings
 ├─ USpringArmComponent "CameraBoom"
 │   └─ USAEVUSCameraComponent           framing, lag, sprint FOV        ← USAEVUSCameraSettings
 └─ InputConfig                          ← USAEVUSInputConfig (which actions to bind)
 └─ BP_SAEVUSCharacter                   mesh, anim BP, data asset references

ASAEVUSDebugHUD (AHUD)                   draws while SAEVUS.Debug.HUD = 1
```

## Rules the code follows

- **Composition over inheritance.** The character is a composition root. Each behaviour lives in a component that owns one responsibility. Planned combat and weapon systems will be components too (`USAEVUSCombatComponent`, `USAEVUSWeaponComponent`), added in their phases.
- **Data-driven tuning.** Every gameplay value is a member initializer in a tuning struct (`FSAEVUSGroundMovementTuning`, `FSAEVUSAirMovementTuning`, `FSAEVUSCameraTuning`). Those initializers are the single source of defaults. A Data Asset holding the struct overrides them. Components fall back to a `const` default instance when no asset is assigned.
- **C++ holds no content references.** Meshes, anim BPs, input assets and Data Assets are assigned in Blueprint subclasses. `Tools/EditorScripts/saevus_bootstrap_assets.py` creates those Blueprints, so they can be rebuilt instead of hand-edited.
- **Replication-ready.** Player intent that changes movement (sprint, walk) travels through the CMC saved-move / compressed-flag path, so it can be client-predicted later. There's no networking yet.

## Input flow

1. `DefaultInput.ini` makes Enhanced Input the default (`EnhancedPlayerInput`, `EnhancedInputComponent`).
2. `ASAEVUSPlayerController::SetupInputComponent` adds the mapping contexts for the local player and binds the debug toggle.
3. `ASAEVUSCharacter::SetupPlayerInputComponent` binds Move, Look, MouseLook, Jump, Sprint and Walk from its `USAEVUSInputConfig`. Missing actions are skipped; a missing config logs an error.
4. Held actions (sprint, walk) are bound on `Triggered` (value true) and `Completed` (value false). They forward to `SetSprintRequested` / `SetWalkRequested` on the movement component.

| Action | Keys | Source |
|---|---|---|
| Move | WASD, left stick | template `IMC_Default` |
| Look | mouse (`IMC_MouseLook`), right stick | template |
| Jump | Space, gamepad bottom face | template |
| Sprint (hold) | Left Shift, L3 | `IMC_SAEVUS` |
| Walk (hold) | Left Alt | `IMC_SAEVUS` |
| Debug HUD | F6 | `IMC_SAEVUS` |

## Rendering profile

`Config/DefaultEngine.ini`: no Lumen, Nanite, ray tracing, Virtual Shadow Maps, MegaLights, distance fields or static lighting. Reflections use SSR; anti-aliasing uses TAA. Lighting is fully dynamic: one directional light, a sky light, sky atmosphere and height fog. DX12 with SM6, as in the engine template.
