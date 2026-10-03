"""
Parte Unreal de la sonda del pipeline: importa el FBX y el GLB que escribe
blender_sonda_exportar.py en una carpeta temporal, escribe los límites de cada malla estática y
borra la carpeta temporal (no queda nada en el proyecto).

Esperado en un pipeline correcto: alto (Z) de 180 cm desde Z = 0, ancho de 50 cm, y la nariz
(el lado cuyo límite llega más lejos) en el eje al que mira el maniquí del motor.

Ejecución sin interfaz (Ejecutar-SondaPipeline.ps1 lo hace):
  UnrealEditor-Cmd.exe SAEVUS.uproject -ExecutePythonScript="ue_sonda_importar.py" -unattended -nullrhi
"""

import os

import unreal

TEMPORAL = "/Game/_SondaPipeline"
DIR_SONDA = os.path.join(unreal.Paths.project_saved_dir(), "SondaPipeline")
ARCHIVOS = ("SM_SondaPipeline.fbx", "SM_SondaPipeline.glb")


def registrar(mensaje):
    unreal.log(f"[SAEVUS sonda] {mensaje}")


def importar(nombre_archivo):
    tarea = unreal.AssetImportTask()
    tarea.filename = os.path.join(DIR_SONDA, nombre_archivo)
    tarea.destination_path = f"{TEMPORAL}/{nombre_archivo.replace('.', '_')}"
    tarea.automated = True
    tarea.replace_existing = True
    tarea.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([tarea])
    return tarea.imported_object_paths


def describir(nombre_archivo, rutas):
    mallas = [unreal.load_asset(r) for r in rutas]
    mallas = [m for m in mallas if isinstance(m, unreal.StaticMesh)]
    if not mallas:
        unreal.log_error(f"[SAEVUS sonda] {nombre_archivo}: no se importó ninguna malla estática ({list(rutas)})")
        return
    for malla in mallas:
        caja = malla.get_bounding_box()
        minimo, maximo = caja.min, caja.max
        registrar(
            f"{nombre_archivo} -> {malla.get_name()}: "
            f"X {minimo.x:.1f}..{maximo.x:.1f}  Y {minimo.y:.1f}..{maximo.y:.1f}  Z {minimo.z:.1f}..{maximo.z:.1f} cm"
        )


def main():
    for nombre_archivo in ARCHIVOS:
        if not os.path.exists(os.path.join(DIR_SONDA, nombre_archivo)):
            unreal.log_error(f"[SAEVUS sonda] falta {nombre_archivo}; ejecuta primero la parte de Blender")
            continue
        describir(nombre_archivo, importar(nombre_archivo))


try:
    main()
finally:
    if unreal.EditorAssetLibrary.does_directory_exist(TEMPORAL):
        unreal.EditorAssetLibrary.delete_directory(TEMPORAL)
    registrar("hecho")
    unreal.SystemLibrary.quit_editor()
