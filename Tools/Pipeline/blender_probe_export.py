"""
Blender side of the pipeline probe: builds a character-sized test mesh and exports it as
FBX and glTF (GLB) with the settings proposed in Docs/asset-pipeline.md.

The probe is a 0.5 x 0.3 x 1.8 m box standing on its origin (feet at 0), with a small
"nose" on its front. Blender's convention: a character faces -Y (Front view looks along +Y).

Run headless (Run-PipelineProbe.ps1 does this):
  blender.exe --background --factory-startup --python blender_probe_export.py -- <output dir>
"""

import os
import sys

import bpy

BODY_SIZE = (0.5, 0.3, 1.8)  # metres: width (X), depth (Y), height (Z)
NOSE_SIZE = 0.1
NOSE_HEIGHT = 1.6
PROBE_NAME = "SM_PipelineProbe"


def build_probe():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    units = bpy.context.scene.unit_settings
    units.system = "METRIC"
    units.scale_length = 1.0

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, 0.0, BODY_SIZE[2] / 2.0))
    body = bpy.context.active_object
    body.scale = BODY_SIZE

    # Nose on the front (-Y), sticking out half its size past the body.
    nose_y = -(BODY_SIZE[1] / 2.0)
    bpy.ops.mesh.primitive_cube_add(size=NOSE_SIZE, location=(0.0, nose_y, NOSE_HEIGHT))
    nose = bpy.context.active_object

    bpy.ops.object.select_all(action="DESELECT")
    body.select_set(True)
    nose.select_set(True)
    bpy.context.view_layer.objects.active = body
    bpy.ops.object.join()
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    body.name = PROBE_NAME
    body.data.name = PROBE_NAME
    return body


def export(probe, out_dir):
    os.makedirs(out_dir, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    probe.select_set(True)

    # FBX: Blender's default axes; unit scale applied so 1 Blender metre = 100 UE cm.
    bpy.ops.export_scene.fbx(
        filepath=os.path.join(out_dir, f"{PROBE_NAME}.fbx"),
        use_selection=True,
        object_types={"MESH"},
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_NONE",
        axis_forward="-Z",
        axis_up="Y",
        mesh_smooth_type="FACE",
        use_mesh_modifiers=True,
    )
    bpy.ops.export_scene.gltf(
        filepath=os.path.join(out_dir, f"{PROBE_NAME}.glb"),
        export_format="GLB",
        use_selection=True,
    )
    print(f"[SAEVUS probe] exported {PROBE_NAME} to {out_dir}")


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argv:
        raise SystemExit("usage: blender --background --python blender_probe_export.py -- <output dir>")
    export(build_probe(), argv[0])


main()
