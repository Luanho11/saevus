"""
Creates or updates the SAEVUS bootstrap assets: input assets, data assets,
Blueprint subclasses and the L_Sandbox test map.

Idempotent: existing assets are loaded and updated, never duplicated.
Mannequin, template input actions and prototype materials come from the
engine template content packs copied into /Game/Characters, /Game/Input and
/Game/LevelPrototyping.

Run headless:
  UnrealEditor-Cmd.exe SAEVUS.uproject -ExecutePythonScript="<this file>" -unattended -nullrhi
"""

import unreal

ROOT = "/Game/SAEVUS"
INPUT_DIR = f"{ROOT}/Input"
DATA_DIR = f"{ROOT}/Data"
BLUEPRINT_DIR = f"{ROOT}/Blueprints"
MAP_PATH = f"{ROOT}/Maps/L_Sandbox"

TEMPLATE_ACTIONS = "/Game/Input/Actions"
TEMPLATE_IMC_DEFAULT = "/Game/Input/IMC_Default"
TEMPLATE_IMC_MOUSELOOK = "/Game/Input/IMC_MouseLook"
MANNEQUIN_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
MANNEQUIN_ANIM_BP = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
GRID_MATERIAL = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
GRID_MATERIAL_DARK = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"
CUBE_MESH = "/Engine/BasicShapes/Cube"  # 100 cm, pivot at the centre

# Held keys, so the character code gets Triggered (true) / Completed (false).
SAEVUS_KEY_MAPPINGS = {
    "IA_Sprint": ["LeftShift", "Gamepad_LeftThumbstick"],
    "IA_Walk": ["LeftAlt"],
    # Pressed (Started). Shift and Alt are taken by sprint and walk.
    "IA_Dash": ["LeftControl", "ThumbMouseButton", "Gamepad_FaceButton_Right"],
    # F1-F5, F9 and F11 are engine debug bindings (BaseInput.ini).
    "IA_ToggleDebugHUD": ["F6"],
}

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def log(message):
    unreal.log(f"[SAEVUS bootstrap] {message}")


def load(path):
    asset = eal.load_asset(path)
    if asset is None:
        raise RuntimeError(f"Missing required asset: {path}")
    return asset


def get_or_create(name, directory, asset_class, factory):
    path = f"{directory}/{name}"
    if eal.does_asset_exist(path):
        return load(path)
    asset = asset_tools.create_asset(name, directory, asset_class, factory)
    if asset is None:
        raise RuntimeError(f"Could not create {path}")
    log(f"created {path}")
    return asset


def make_key(name):
    key = unreal.Key()
    if not key.import_text(name):
        raise RuntimeError(f"Unknown key name: {name}")
    return key


def get_or_create_blueprint(name, parent_class):
    path = f"{BLUEPRINT_DIR}/{name}"
    if eal.does_asset_exist(path):
        return load(path)
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    return get_or_create(name, BLUEPRINT_DIR, unreal.Blueprint, factory)


def blueprint_cdo(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return unreal.get_default_object(blueprint.generated_class())


def create_input_assets():
    actions = {}
    for name in SAEVUS_KEY_MAPPINGS:
        action = get_or_create(name, f"{INPUT_DIR}/Actions", unreal.InputAction, unreal.InputAction_Factory())
        action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
        actions[name] = action

    imc = get_or_create("IMC_SAEVUS", INPUT_DIR, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    imc.unmap_all()
    for name, keys in SAEVUS_KEY_MAPPINGS.items():
        for key in keys:
            imc.map_key(actions[name], make_key(key))
    return actions, imc


def create_data_assets(actions):
    def data_asset(name, asset_class):
        factory = unreal.DataAssetFactory()
        factory.set_editor_property("data_asset_class", asset_class)
        return get_or_create(name, DATA_DIR, asset_class, factory)

    input_config = data_asset("DA_InputConfig", unreal.SAEVUSInputConfig)
    input_config.set_editor_property("move_action", load(f"{TEMPLATE_ACTIONS}/IA_Move"))
    input_config.set_editor_property("look_action", load(f"{TEMPLATE_ACTIONS}/IA_Look"))
    input_config.set_editor_property("mouse_look_action", load(f"{TEMPLATE_ACTIONS}/IA_MouseLook"))
    input_config.set_editor_property("jump_action", load(f"{TEMPLATE_ACTIONS}/IA_Jump"))
    input_config.set_editor_property("sprint_action", actions["IA_Sprint"])
    input_config.set_editor_property("walk_action", actions["IA_Walk"])
    input_config.set_editor_property("dash_action", actions["IA_Dash"])

    # Values stay at the code defaults until tuned in the editor.
    movement = data_asset("DA_MovementSettings_Default", unreal.SAEVUSMovementSettings)
    camera = data_asset("DA_CameraSettings_Default", unreal.SAEVUSCameraSettings)
    return input_config, movement, camera


def create_blueprints(actions, imc, input_config, movement, camera):
    character_bp = get_or_create_blueprint("BP_SAEVUSCharacter", unreal.SAEVUSCharacter)
    character = blueprint_cdo(character_bp)
    character.set_editor_property("input_config", input_config)
    mesh = character.get_editor_property("mesh")
    # SkeletalMeshAsset is transient; only the setter updates the serialized skinned asset.
    if hasattr(mesh, "set_skeletal_mesh_asset"):
        mesh.set_skeletal_mesh_asset(load(MANNEQUIN_MESH))
    else:
        mesh.set_skeletal_mesh(load(MANNEQUIN_MESH), False)
    mesh.set_editor_property("anim_class", load(MANNEQUIN_ANIM_BP).generated_class())
    character.get_editor_property("character_movement").set_editor_property("movement_settings", movement)
    character.get_editor_property("follow_camera").set_editor_property("camera_settings", camera)

    controller_bp = get_or_create_blueprint("BP_SAEVUSPlayerController", unreal.SAEVUSPlayerController)
    controller = blueprint_cdo(controller_bp)
    controller.set_editor_property(
        "default_mapping_contexts", [load(TEMPLATE_IMC_DEFAULT), load(TEMPLATE_IMC_MOUSELOOK), imc]
    )
    controller.set_editor_property("toggle_debug_hud_action", actions["IA_ToggleDebugHUD"])

    game_mode_bp = get_or_create_blueprint("BP_SAEVUSGameMode", unreal.SAEVUSGameMode)
    game_mode = blueprint_cdo(game_mode_bp)
    game_mode.set_editor_property("default_pawn_class", character_bp.generated_class())
    game_mode.set_editor_property("player_controller_class", controller_bp.generated_class())

    for blueprint in (character_bp, controller_bp, game_mode_bp):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return game_mode_bp


# --- Sandbox map --------------------------------------------------------------

def spawn_block(actors, label, location, size_cm, material, rotation=unreal.Rotator(0, 0, 0)):
    """Box of size_cm (x, y, z) centred on location, built from the 100 cm engine cube."""
    actor = actors.spawn_actor_from_object(load(CUBE_MESH), location, rotation)
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(size_cm[0] / 100.0, size_cm[1] / 100.0, size_cm[2] / 100.0))
    actor.static_mesh_component.set_material(0, material)
    actor.set_folder_path("Geometry")
    return actor


def build_sandbox(game_mode_bp):
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    if eal.does_asset_exist(MAP_PATH):
        level_editor.load_level(MAP_PATH)
        for actor in actors.get_all_level_actors():
            if not isinstance(actor, unreal.WorldSettings):
                actors.destroy_actor(actor)
    else:
        level_editor.new_level(MAP_PATH)

    grid = load(GRID_MATERIAL)
    dark = load(GRID_MATERIAL_DARK)

    # Ground: 100 m x 100 m, top surface at Z = 0.
    spawn_block(actors, "Floor", unreal.Vector(0, 0, -50), (10000, 10000, 100), grid)

    # Jump height steps: 50 cm to 250 cm.
    for index, height in enumerate((50, 100, 150, 200, 250)):
        spawn_block(actors, f"Step_{height}cm", unreal.Vector(800, -1000 + index * 400, height / 2.0), (200, 200, height), dark)

    # Slopes either side of the default walkable limit (~44.8 deg).
    for index, angle in enumerate((20, 35, 50)):
        spawn_block(
            actors, f"Slope_{angle}deg", unreal.Vector(-1200, -1200 + index * 700, 0), (800, 400, 40), dark,
            unreal.Rotator(roll=0, pitch=angle, yaw=0),
        )

    # Gap jumps from a 200 cm deck: gaps of 2, 3, 4, 5 and 6 m.
    deck_height = 200
    x = 2000
    for gap in (0, 200, 300, 400, 500, 600):
        x += gap
        spawn_block(actors, f"Deck_gap{gap}cm", unreal.Vector(x + 200, 1500, deck_height / 2.0), (400, 400, deck_height), dark)
        x += 400

    # Long, tall wall for later wall-movement experiments.
    spawn_block(actors, "Wall_Long", unreal.Vector(0, 3000, 300), (3000, 100, 600), dark)

    # Wall-jump chimney: two 7 m walls with a 3 m gap, climbed by alternating wall jumps,
    # and a 4.5 m ledge at the far end to finish on.
    for side, y in (("L", -200), ("R", 200)):
        spawn_block(actors, f"WallJump_Chimney{side}", unreal.Vector(-3000, y, 350), (600, 100, 700), dark)
    spawn_block(actors, "WallJump_Ledge450cm", unreal.Vector(-3500, 0, 225), (400, 400, 450), grid)

    # Wall-run corridor: two 20 m x 6 m walls, 5 m apart, to chain wall runs from side to side.
    for side, y in (("A", -3300), ("B", -2700)):
        spawn_block(actors, f"WallRun_Corridor{side}", unreal.Vector(0, y, 300), (2000, 100, 600), dark)

    # Wall climb: a 3.5 m block to climb onto, and a 10 m wall that ends every climb in a flip.
    spawn_block(actors, "WallClimb_Block350cm", unreal.Vector(3000, -1500, 175), (400, 800, 350), grid)
    spawn_block(actors, "WallClimb_Tall", unreal.Vector(3000, -2600, 500), (100, 800, 1000), dark)

    # Lighting: fully dynamic, no Lumen.
    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-50, yaw=-35))
    sun.set_actor_label("Sun")
    sun_light = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_light.set_editor_property("atmosphere_sun_light", True)
    sun_light.set_mobility(unreal.ComponentMobility.MOVABLE)

    sky_light = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1200))
    sky_light.set_actor_label("SkyLight")
    sky_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_editor_property("real_time_capture", True)

    actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0)).set_actor_label("SkyAtmosphere")
    actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0)).set_actor_label("HeightFog")
    for label in ("Sun", "SkyLight", "SkyAtmosphere", "HeightFog"):
        for actor in actors.get_all_level_actors():
            if actor.get_actor_label() == label:
                actor.set_folder_path("Lighting")

    actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100)).set_actor_label("PlayerStart")

    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", game_mode_bp.generated_class())

    level_editor.save_current_level()


def main():
    actions, imc = create_input_assets()
    input_config, movement, camera = create_data_assets(actions)
    game_mode_bp = create_blueprints(actions, imc, input_config, movement, camera)
    build_sandbox(game_mode_bp)
    if not eal.save_directory(ROOT, only_if_is_dirty=False, recursive=True):
        raise RuntimeError("Saving /Game/SAEVUS failed")
    log("done")


try:
    main()
except Exception as error:  # noqa: BLE001 - surface any failure in the log and still quit
    unreal.log_error(f"[SAEVUS bootstrap] FAILED: {error}")
    raise
finally:
    unreal.SystemLibrary.quit_editor()
