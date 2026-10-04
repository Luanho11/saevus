"""
Genera los cuerpos boceto de los enemigos de SAEVUS en Blender: una malla estática por tipo, con
dos materiales ("Cuerpo" en la ranura 0, "Mascara" en la 1) que UE colorea por datos.

Cada cuerpo llena la cápsula de su tipo (FSAEVUSAjusteEnemigo): origen en el centro de la cápsula
(como el actor en UE), mirando a +X (llega a UE mirando a +X, el frente del actor). Formas simples
y rotundas en la línea de Hollow Knight, con identidad propia: rastrero bajo y ancho, saltador con
patas traseras grandes, escupidor con buche luminoso, revoloteador alado, Guardián acorazado con cuernos enormes.

Ejecución sin interfaz (Generar-Enemigos.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_enemigos.py -- <directorio de salida>
"""

import math
import os
import sys

import bpy
from mathutils import Vector


def limpiar_escena():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.0


def material(nombre):
    return bpy.data.materials.get(nombre) or bpy.data.materials.new(nombre)


def terminar(objeto, nombre_material):
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    objeto.data.materials.append(material(nombre_material))
    return objeto


def esfera(nombre_material, centro, radios, segmentos=14, anillos=9):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segmentos, ring_count=anillos, radius=1.0, location=centro)
    objeto = bpy.context.active_object
    objeto.scale = radios
    return terminar(objeto, nombre_material)


def segmento(nombre_material, a, b, radio_a, radio_b, lados=8):
    a, b = Vector(a), Vector(b)
    direccion = b - a
    bpy.ops.mesh.primitive_cone_add(vertices=lados, radius1=radio_a, radius2=radio_b, depth=direccion.length, location=(a + b) / 2.0)
    objeto = bpy.context.active_object
    objeto.rotation_mode = "QUATERNION"
    objeto.rotation_quaternion = direccion.to_track_quat("Z", "Y")
    return terminar(objeto, nombre_material)


def cuernos(base_x, base_z, separacion, largo, grosor, curva):
    piezas = []
    for lado in (-1, 1):
        medio = (base_x - curva * 0.3, lado * (separacion + largo * 0.3), base_z + largo * 0.6)
        punta = (base_x - curva, lado * (separacion + largo * 0.4), base_z + largo)
        piezas.append(segmento("Mascara", (base_x, lado * separacion, base_z), medio, grosor, grosor * 0.6))
        piezas.append(segmento("Mascara", medio, punta, grosor * 0.6, grosor * 0.1))
    return piezas


def rastrero():
    """Bajo y ancho (cápsula 0,40 × 0,40 m): caparazón, máscara al frente, cuatro patas cortas."""
    piezas = [esfera("Cuerpo", (0, 0, 0.0), (0.42, 0.38, 0.28)), esfera("Mascara", (0.26, 0, 0.06), (0.16, 0.22, 0.17))]
    for x in (0.18, -0.18):
        for lado in (-1, 1):
            piezas.append(segmento("Cuerpo", (x, lado * 0.25, -0.1), (x * 1.3, lado * 0.38, -0.4), 0.05, 0.02, 6))
    piezas += cuernos(0.2, 0.2, 0.08, 0.18, 0.035, 0.05)
    return piezas


def saltador():
    """Redondo con patas traseras grandes (cápsula 0,35 × 0,50 m)."""
    piezas = [esfera("Cuerpo", (0, 0, 0.0), (0.32, 0.3, 0.36)), esfera("Mascara", (0.2, 0, 0.18), (0.15, 0.2, 0.17))]
    for lado in (-1, 1):
        piezas.append(segmento("Cuerpo", (-0.12, lado * 0.22, -0.05), (-0.3, lado * 0.3, -0.25), 0.11, 0.07, 8))
        piezas.append(segmento("Cuerpo", (-0.3, lado * 0.3, -0.25), (0.0, lado * 0.3, -0.5), 0.07, 0.04, 8))
        piezas.append(segmento("Cuerpo", (0.15, lado * 0.15, -0.15), (0.2, lado * 0.17, -0.5), 0.045, 0.03, 6))
    piezas += cuernos(0.16, 0.36, 0.07, 0.3, 0.03, 0.12)
    return piezas


def escupidor():
    """Alto en forma de pera (cápsula 0,38 × 0,55 m) con un buche luminoso al frente."""
    piezas = [
        esfera("Cuerpo", (0, 0, -0.15), (0.36, 0.34, 0.4)),
        esfera("Cuerpo", (0.03, 0, 0.3), (0.2, 0.2, 0.22)),
        esfera("Mascara", (0.16, 0, 0.32), (0.11, 0.15, 0.13)),
        esfera("Mascara", (0.28, 0, -0.1), (0.13, 0.17, 0.15)),
    ]
    piezas += cuernos(0.08, 0.48, 0.06, 0.16, 0.03, 0.02)
    return piezas


def revoloteador():
    """Volador (cápsula 0,35 × 0,35 m): cuerpo redondo, máscara, dos alas membranosas y un aguijón colgante."""
    piezas = [esfera("Cuerpo", (0, 0, 0.0), (0.3, 0.27, 0.27)), esfera("Mascara", (0.2, 0, 0.06), (0.13, 0.18, 0.15))]
    for lado in (-1, 1):
        # Ala: elipsoide plano que sale en diagonal hacia arriba y atrás, con una segunda lámina en la punta.
        piezas.append(esfera("Cuerpo", (-0.05, lado * 0.36, 0.14), (0.2, 0.24, 0.025), 12, 6))
        piezas.append(esfera("Cuerpo", (-0.14, lado * 0.62, 0.26), (0.14, 0.16, 0.02), 10, 6))
    piezas.append(segmento("Cuerpo", (-0.05, 0, -0.2), (-0.12, 0, -0.45), 0.06, 0.01, 6))
    piezas += cuernos(0.12, 0.22, 0.06, 0.14, 0.025, 0.04)
    return piezas


def guardian():
    """Mini jefe acorazado (cápsula 0,90 × 1,20 m): torso de barril, hombreras, máscara y cuernos enormes."""
    piezas = [
        segmento("Cuerpo", (0, 0, -1.2), (0, 0, -0.4), 0.55, 0.75, 10),
        segmento("Cuerpo", (0, 0, -0.4), (0, 0, 0.55), 0.8, 0.62, 10),
        esfera("Mascara", (0.42, 0, 0.75), (0.32, 0.42, 0.45), 16, 10),
    ]
    for lado in (-1, 1):
        piezas.append(esfera("Cuerpo", (0, lado * 0.8, 0.45), (0.38, 0.32, 0.3)))
        piezas.append(segmento("Cuerpo", (0, lado * 0.85, 0.3), (0.35, lado * 0.95, -0.55), 0.22, 0.16, 8))
    piezas += cuernos(0.3, 1.05, 0.25, 0.75, 0.11, 0.35)
    return piezas


TIPOS = {
    "SM_Enemigo_Rastrero": rastrero,
    "SM_Enemigo_Saltador": saltador,
    "SM_Enemigo_Escupidor": escupidor,
    "SM_Enemigo_Revoloteador": revoloteador,
    "SM_Jefe_Guardian": guardian,
}


def unir(piezas, nombre):
    bpy.ops.object.select_all(action="DESELECT")
    for pieza in piezas:
        pieza.select_set(True)
    bpy.context.view_layer.objects.active = piezas[0]
    bpy.ops.object.join()
    objeto = bpy.context.active_object
    objeto.name = nombre
    objeto.data.name = nombre
    bpy.ops.object.shade_smooth()
    # Ranuras en orden fijo: 0 = Cuerpo, 1 = Mascara (las asigna UE por índice).
    ranuras = [m.material.name for m in objeto.material_slots]
    orden = ["Cuerpo", "Mascara"]
    for indice, nombre_material in enumerate(orden):
        actual = ranuras.index(nombre_material)
        if actual != indice:
            objeto.active_material_index = actual
            for _ in range(actual - indice):
                bpy.ops.object.material_slot_move(direction="UP")
            ranuras = [m.material.name for m in objeto.material_slots]
    return objeto


def exportar(objeto, directorio):
    bpy.ops.object.select_all(action="DESELECT")
    objeto.select_set(True)
    bpy.context.view_layer.objects.active = objeto
    ruta = os.path.join(directorio, f"{objeto.name}.fbx")
    bpy.ops.export_scene.fbx(filepath=ruta, use_selection=True, object_types={"MESH"}, apply_unit_scale=True,
                             apply_scale_options="FBX_SCALE_NONE", axis_forward="-Z", axis_up="Y", mesh_smooth_type="FACE")
    triangulos = sum(len(p.vertices) - 2 for p in objeto.data.polygons)
    print(f"[SAEVUS arte] exportado {ruta}: {triangulos} triángulos, materiales {[m.material.name for m in objeto.material_slots]}")


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_enemigos.py -- <directorio de salida>")
    directorio = argumentos[0]
    os.makedirs(directorio, exist_ok=True)
    limpiar_escena()
    for indice, (nombre, crear) in enumerate(TIPOS.items()):
        objeto = unir(crear(), nombre)
        exportar(objeto, directorio)
        objeto.location.x = indice * 3.0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(directorio, "Enemigos.blend"))


main()
