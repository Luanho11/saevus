"""
Checks that the bootstrap assets exist and that the Blueprint defaults set by
saevus_bootstrap_assets.py survived saving (fresh editor process, so values
come from disk). Logs one line per check and "[SAEVUS verify] PASSED" or
"[SAEVUS verify] FAILED".

Run headless:
  UnrealEditor-Cmd.exe SAEVUS.uproject -ExecutePythonScript="<this file>" -unattended -nullrhi
"""

import unreal

eal = unreal.EditorAssetLibrary
failures = []


def check(description, condition):
    if condition:
        unreal.log(f"[SAEVUS verify] ok   {description}")
    else:
        unreal.log_error(f"[SAEVUS verify] FAIL {description}")
        failures.append(description)


def cdo(path):
    blueprint = eal.load_asset(path)
    check(f"{path} exists", blueprint is not None)
    return unreal.get_default_object(blueprint.generated_class()) if blueprint else None


def main():
    for path in (
        "/Game/SAEVUS/Input/IMC_SAEVUS",
        "/Game/SAEVUS/Input/Actions/IA_Sprint",
        "/Game/SAEVUS/Input/Actions/IA_Walk",
        "/Game/SAEVUS/Input/Actions/IA_Dash",
        "/Game/SAEVUS/Input/Actions/IA_ToggleDebugHUD",
        "/Game/SAEVUS/Data/DA_MovementSettings_Default",
        "/Game/SAEVUS/Data/DA_CameraSettings_Default",
        "/Game/SAEVUS/Maps/L_Sandbox",
    ):
        check(f"{path} exists", eal.does_asset_exist(path))

    input_config = eal.load_asset("/Game/SAEVUS/Data/DA_InputConfig")
    check("DA_InputConfig exists", input_config is not None)
    if input_config:
        for prop in ("move_action", "look_action", "mouse_look_action", "jump_action", "sprint_action", "walk_action", "dash_action"):
            check(f"DA_InputConfig.{prop} set", input_config.get_editor_property(prop) is not None)

    character = cdo("/Game/SAEVUS/Blueprints/BP_SAEVUSCharacter")
    if character:
        mesh = character.get_editor_property("mesh")
        check("character mesh asset set", mesh.get_editor_property("skeletal_mesh_asset") is not None)
        check("character anim class set", mesh.get_editor_property("anim_class") is not None)
        check("character input config set", character.get_editor_property("input_config") is not None)
        movement = character.get_editor_property("character_movement")
        check("movement component is SAEVUS", isinstance(movement, unreal.SAEVUSMovementComponent))
        check("movement settings set", movement.get_editor_property("movement_settings") is not None)
        camera = character.get_editor_property("follow_camera")
        check("camera settings set", camera.get_editor_property("camera_settings") is not None)

    controller = cdo("/Game/SAEVUS/Blueprints/BP_SAEVUSPlayerController")
    if controller:
        check("controller has 3 mapping contexts", len(controller.get_editor_property("default_mapping_contexts")) == 3)
        check("controller debug toggle action set", controller.get_editor_property("toggle_debug_hud_action") is not None)

    game_mode = cdo("/Game/SAEVUS/Blueprints/BP_SAEVUSGameMode")
    if game_mode:
        pawn_class = game_mode.get_editor_property("default_pawn_class")
        check("game mode pawn is BP_SAEVUSCharacter", pawn_class is not None and "BP_SAEVUSCharacter" in pawn_class.get_name())

    if failures:
        unreal.log_error(f"[SAEVUS verify] FAILED ({len(failures)} checks)")
    else:
        unreal.log("[SAEVUS verify] PASSED")


try:
    main()
except Exception as error:  # noqa: BLE001 - surface any failure in the log and still quit
    unreal.log_error(f"[SAEVUS verify] FAILED: {error}")
finally:
    unreal.SystemLibrary.quit_editor()
