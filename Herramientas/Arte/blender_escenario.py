"""
Genera las piezas modulares boceto del escenario de SAEVUS en Blender y exporta cada una como FBX
estático (y un .blend con todas para editarlas a mano).

Tono: ruinas oscuras de un mundo caído. Formas simples y rotas, pocos triángulos, sin texturas
(el color lo pone el material de escenario en UE). Todo en metros, origen en la base centrada.

Ejecución sin interfaz (Generar-Escenario.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_escenario.py -- <directorio de salida>
"""

import math
import os
import random
import sys

import bpy

# Semilla fija: el escenario sale igual en cada generación.
SEMILLA = 7


def limpiar_escena():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.0


def material(nombre):
    return bpy.data.materials.get(nombre) or bpy.data.materials.new(nombre)


def pieza_base(objeto, nombre_material):
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    objeto.data.materials.append(material(nombre_material))
    return objeto


def cilindro(radio_abajo, radio_arriba, alto, base_z=0.0, lados=8, x=0.0, y=0.0, rotacion=(0, 0, 0)):
    bpy.ops.mesh.primitive_cone_add(vertices=lados, radius1=radio_abajo, radius2=radio_arriba, depth=alto,
                                    location=(x, y, base_z + alto / 2.0), rotation=rotacion)
    return pieza_base(bpy.context.active_object, "Piedra")


def bloque(tamano, centro, rotacion=(0, 0, 0)):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=centro, rotation=rotacion)
    objeto = bpy.context.active_object
    objeto.scale = tamano
    return pieza_base(objeto, "Piedra")


def unir(piezas, nombre):
    bpy.ops.object.select_all(action="DESELECT")
    for pieza in piezas:
        pieza.select_set(True)
    bpy.context.view_layer.objects.active = piezas[0]
    if len(piezas) > 1:
        bpy.ops.object.join()
    objeto = bpy.context.active_object
    objeto.name = nombre
    objeto.data.name = nombre
    return objeto


def columna_rota():
    """Columna de 6 m que se afila, con el remate partido e inclinado y un trozo caído a su pie."""
    piezas = [
        cilindro(0.55, 0.62, 0.4),
        cilindro(0.45, 0.38, 4.6, base_z=0.4),
        cilindro(0.38, 0.33, 0.9, base_z=5.0, rotacion=(math.radians(9), 0, 0)),
        bloque((0.5, 0.35, 0.3), (0.8, 0.3, 0.15), rotacion=(0, 0, math.radians(30))),
    ]
    return unir(piezas, "SM_ColumnaRota")


def arco_seccion():
    """Arco de 9 m de ancho que marca el paso de sección: dos pilares que bajan al vacío y un dintel roto."""
    piezas = []
    for lado in (-1, 1):
        piezas.append(bloque((0.8, 0.8, 11.0), (lado * 4.9, 0, -0.5)))
        piezas.append(bloque((1.1, 1.1, 0.35), (lado * 4.9, 0, 5.1)))
    # Dintel partido: dos tramos que no se tocan en el centro.
    piezas.append(bloque((4.2, 0.7, 0.6), (-2.9, 0, 5.6), rotacion=(0, math.radians(-3), 0)))
    piezas.append(bloque((3.4, 0.7, 0.6), (3.2, 0, 5.5), rotacion=(0, math.radians(5), 0)))
    return unir(piezas, "SM_ArcoSeccion")


def escombros():
    """Montón de bloques caídos, 1,5 m de ancho."""
    aleatorio = random.Random(SEMILLA)
    piezas = []
    for _ in range(6):
        lado = aleatorio.uniform(0.25, 0.6)
        piezas.append(bloque(
            (lado, lado * aleatorio.uniform(0.6, 1.2), lado * aleatorio.uniform(0.5, 0.9)),
            (aleatorio.uniform(-0.6, 0.6), aleatorio.uniform(-0.6, 0.6), lado * 0.3),
            rotacion=(aleatorio.uniform(-0.3, 0.3), aleatorio.uniform(-0.3, 0.3), aleatorio.uniform(0, math.pi))))
    return unir(piezas, "SM_Escombros")


def exportar(objeto, directorio):
    bpy.ops.object.select_all(action="DESELECT")
    objeto.select_set(True)
    bpy.context.view_layer.objects.active = objeto
    ruta = os.path.join(directorio, f"{objeto.name}.fbx")
    bpy.ops.export_scene.fbx(filepath=ruta, use_selection=True, object_types={"MESH"}, apply_unit_scale=True,
                             apply_scale_options="FBX_SCALE_NONE", axis_forward="-Z", axis_up="Y", mesh_smooth_type="FACE")
    triangulos = sum(len(p.vertices) - 2 for p in objeto.data.polygons)
    print(f"[SAEVUS arte] exportado {ruta}: {triangulos} triángulos")


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_escenario.py -- <directorio de salida>")
    directorio = argumentos[0]
    os.makedirs(directorio, exist_ok=True)
    limpiar_escena()
    piezas = [columna_rota(), arco_seccion(), escombros()]
    # Separadas en el .blend para verlas juntas.
    for indice, pieza in enumerate(piezas):
        exportar(pieza, directorio)
        pieza.location.x = indice * 12.0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(directorio, "Escenario.blend"))


main()
