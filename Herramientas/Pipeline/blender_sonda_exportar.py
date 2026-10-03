"""
Parte Blender de la sonda del pipeline: construye una malla de prueba del tamaño de un personaje
y la exporta como FBX y glTF (GLB) con los ajustes de Documentos/pipeline-assets.md.

La sonda es una caja de 0,5 x 0,3 x 1,8 m de pie sobre su origen (pies en 0), con una pequeña
"nariz" en el frente. Convención de Blender: un personaje mira a -Y (la vista Front mira hacia +Y).

Ejecución sin interfaz (Ejecutar-SondaPipeline.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_sonda_exportar.py -- <directorio de salida>
"""

import os
import sys

import bpy

TAMANO_CUERPO = (0.5, 0.3, 1.8)  # metros: ancho (X), fondo (Y), alto (Z)
TAMANO_NARIZ = 0.1
ALTURA_NARIZ = 1.6
NOMBRE_SONDA = "SM_SondaPipeline"


def construir_sonda():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    unidades = bpy.context.scene.unit_settings
    unidades.system = "METRIC"
    unidades.scale_length = 1.0

    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0.0, 0.0, TAMANO_CUERPO[2] / 2.0))
    cuerpo = bpy.context.active_object
    cuerpo.scale = TAMANO_CUERPO

    # Nariz en el frente (-Y), sobresaliendo la mitad de su tamaño del cuerpo.
    nariz_y = -(TAMANO_CUERPO[1] / 2.0)
    bpy.ops.mesh.primitive_cube_add(size=TAMANO_NARIZ, location=(0.0, nariz_y, ALTURA_NARIZ))
    nariz = bpy.context.active_object

    bpy.ops.object.select_all(action="DESELECT")
    cuerpo.select_set(True)
    nariz.select_set(True)
    bpy.context.view_layer.objects.active = cuerpo
    bpy.ops.object.join()
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    cuerpo.name = NOMBRE_SONDA
    cuerpo.data.name = NOMBRE_SONDA
    return cuerpo


def exportar(sonda, directorio):
    os.makedirs(directorio, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    sonda.select_set(True)

    # FBX: ejes por defecto de Blender; escala de unidades aplicada, así 1 metro de Blender = 100 cm de UE.
    bpy.ops.export_scene.fbx(
        filepath=os.path.join(directorio, f"{NOMBRE_SONDA}.fbx"),
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
        filepath=os.path.join(directorio, f"{NOMBRE_SONDA}.glb"),
        export_format="GLB",
        use_selection=True,
    )
    print(f"[SAEVUS sonda] exportado {NOMBRE_SONDA} a {directorio}")


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_sonda_exportar.py -- <directorio de salida>")
    exportar(construir_sonda(), argumentos[0])


main()
