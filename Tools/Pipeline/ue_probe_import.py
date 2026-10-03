"""
Unreal side of the pipeline probe: imports the FBX and GLB written by
blender_probe_export.py into a scratch folder, logs each static mesh's bounds, then
deletes the scratch folder (nothing is left in the project).

Expected for a correct pipeline: height (Z) 180 cm from Z = 0, width 50 cm, and the
nose (the side whose bound reaches further) on the axis the engine mannequin faces.

Run headless (Run-PipelineProbe.ps1 does this):
  UnrealEditor-Cmd.exe SAEVUS.uproject -ExecutePythonScript="ue_probe_import.py" -unattended -nullrhi
"""

import os

import unreal

SCRATCH = "/Game/_PipelineProbe"
PROBE_DIR = os.path.join(unreal.Paths.project_saved_dir(), "PipelineProbe")
FILES = ("SM_PipelineProbe.fbx", "SM_PipelineProbe.glb")


def log(message):
    unreal.log(f"[SAEVUS probe] {message}")


def import_file(file_name):
    task = unreal.AssetImportTask()
    task.filename = os.path.join(PROBE_DIR, file_name)
    task.destination_path = f"{SCRATCH}/{file_name.replace('.', '_')}"
    task.automated = True
    task.replace_existing = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return task.imported_object_paths


def describe(file_name, paths):
    meshes = [unreal.load_asset(p) for p in paths]
    meshes = [m for m in meshes if isinstance(m, unreal.StaticMesh)]
    if not meshes:
        unreal.log_error(f"[SAEVUS probe] {file_name}: no static mesh imported ({list(paths)})")
        return
    for mesh in meshes:
        box = mesh.get_bounding_box()
        lo, hi = box.min, box.max
        log(
            f"{file_name} -> {mesh.get_name()}: "
            f"X {lo.x:.1f}..{hi.x:.1f}  Y {lo.y:.1f}..{hi.y:.1f}  Z {lo.z:.1f}..{hi.z:.1f} cm"
        )


def main():
    for file_name in FILES:
        if not os.path.exists(os.path.join(PROBE_DIR, file_name)):
            unreal.log_error(f"[SAEVUS probe] missing {file_name}; run the Blender side first")
            continue
        describe(file_name, import_file(file_name))


try:
    main()
finally:
    if unreal.EditorAssetLibrary.does_directory_exist(SCRATCH):
        unreal.EditorAssetLibrary.delete_directory(SCRATCH)
    log("done")
    unreal.SystemLibrary.quit_editor()
