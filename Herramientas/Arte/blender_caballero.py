"""
Genera el caballero boceto de SAEVUS en Blender: malla de piezas simples + esqueleto propio,
y lo exporta como FBX (y guarda el .blend para editarlo a mano).

Diseño de boceto, inspirado en el tono de Hollow Knight pero propio: máscara clara grande con
dos cuernos y ojos oscuros, cuerpo y capa oscuros, extremidades finas, un aguijón (espada) y una
pistola. Cada pieza va rígida a un hueso (peso 1): el aspecto de "marioneta" encaja con un boceto
y la animación procedural de UE. Todo es modificable aquí: proporciones, piezas, colores.

Convenciones (Documentos/pipeline-assets.md): metros, el personaje mira a -Y en Blender (llega
mirando a +Y en UE, como el maniquí), origen en los pies. Derecha del personaje = -X en Blender.

Ejecución sin interfaz (Generar-Caballero.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_caballero.py -- <directorio de salida>
"""

import math
import os
import sys

import bpy
from mathutils import Vector

NOMBRE_MALLA = "SK_Caballero"
NOMBRE_ARMADURA = "Armature"  # UE trata "Armature" como nodo raíz implícito y no lo añade como hueso.

# Colores de boceto (lineales). Se repiten en los materiales de UE; aquí solo sirven para verlo en Blender.
MATERIALES = {
    "Cuerpo": (0.02, 0.02, 0.025, 1.0),
    "Mascara": (0.85, 0.85, 0.82, 1.0),
    "Capa": (0.04, 0.05, 0.08, 1.0),
    "Metal": (0.55, 0.58, 0.62, 1.0),
    "Ojos": (0.0, 0.0, 0.0, 1.0),
}

# Huesos: (nombre, padre, cabeza, cola). Pose de reposo en "A": brazos un poco abiertos y hacia abajo.
D = -1.0  # lado derecho del personaje (-X)
I = 1.0   # lado izquierdo (+X)
HUESOS = [
    ("raiz", None, (0, 0, 0), (0, 0, 0.12)),
    ("pelvis", "raiz", (0, 0, 0.86), (0, 0, 0.96)),
    ("columna", "pelvis", (0, 0, 0.96), (0, 0, 1.14)),
    ("pecho", "columna", (0, 0, 1.14), (0, 0, 1.32)),
    ("cuello", "pecho", (0, 0, 1.32), (0, 0, 1.38)),
    ("cabeza", "cuello", (0, 0, 1.38), (0, 0, 1.78)),
    ("capa_1", "pecho", (0, 0.11, 1.28), (0, 0.14, 0.96)),
    ("capa_2", "capa_1", (0, 0.14, 0.96), (0, 0.17, 0.56)),
    # Tercer tramo de la capa: no lo anima nadie, sigue a capa_2 (alarga la capa hasta las rodillas).
    ("capa_3", "capa_2", (0, 0.17, 0.56), (0, 0.19, 0.28)),
]
for lado, s in (("d", D), ("i", I)):
    HUESOS += [
        (f"hombro_{lado}", "pecho", (s * 0.06, 0, 1.29), (s * 0.17, 0, 1.29)),
        (f"brazo_{lado}", f"hombro_{lado}", (s * 0.17, 0, 1.29), (s * 0.21, 0, 1.03)),
        (f"antebrazo_{lado}", f"brazo_{lado}", (s * 0.21, 0, 1.03), (s * 0.23, -0.02, 0.80)),
        (f"mano_{lado}", f"antebrazo_{lado}", (s * 0.23, -0.02, 0.80), (s * 0.23, -0.03, 0.72)),
        (f"muslo_{lado}", "pelvis", (s * 0.09, 0, 0.88), (s * 0.10, 0, 0.48)),
        (f"pierna_{lado}", f"muslo_{lado}", (s * 0.10, 0, 0.48), (s * 0.10, 0.02, 0.09)),
        (f"pie_{lado}", f"pierna_{lado}", (s * 0.10, 0.02, 0.09), (s * 0.10, -0.12, 0.02)),
    ]
# Armas en la mano derecha: el aguijón cuelga hacia abajo en reposo; la pistola apunta al frente.
HUESOS += [
    ("arma", "mano_d", (D * 0.23, -0.04, 0.74), (D * 0.23, -0.04, 0.12)),
    ("arma_fuego", "mano_d", (D * 0.23, -0.05, 0.76), (D * 0.23, -0.32, 0.76)),
]


def limpiar_escena():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    unidades = bpy.context.scene.unit_settings
    unidades.system = "METRIC"
    unidades.scale_length = 1.0


def material(nombre):
    existente = bpy.data.materials.get(nombre)
    if existente:
        return existente
    nuevo = bpy.data.materials.new(nombre)
    nuevo.diffuse_color = MATERIALES[nombre]
    return nuevo


def crear_armadura():
    datos = bpy.data.armatures.new(NOMBRE_ARMADURA)
    objeto = bpy.data.objects.new(NOMBRE_ARMADURA, datos)
    bpy.context.scene.collection.objects.link(objeto)
    bpy.context.view_layer.objects.active = objeto
    bpy.ops.object.mode_set(mode="EDIT")
    for nombre, padre, cabeza, cola in HUESOS:
        hueso = datos.edit_bones.new(nombre)
        hueso.head = cabeza
        hueso.tail = cola
        if padre:
            hueso.parent = datos.edit_bones[padre]
            hueso.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    return objeto


def terminar_pieza(objeto, hueso, nombre_material):
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    grupo = objeto.vertex_groups.new(name=hueso)
    grupo.add(list(range(len(objeto.data.vertices))), 1.0, "REPLACE")
    objeto.data.materials.append(material(nombre_material))
    return objeto


def esfera(hueso, nombre_material, centro, radios, segmentos=16, anillos=10):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segmentos, ring_count=anillos, radius=1.0, location=centro)
    objeto = bpy.context.active_object
    objeto.scale = radios
    return terminar_pieza(objeto, hueso, nombre_material)


def caja(hueso, nombre_material, centro, tamano, rotacion=(0, 0, 0)):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=centro, rotation=rotacion)
    objeto = bpy.context.active_object
    objeto.scale = tamano
    return terminar_pieza(objeto, hueso, nombre_material)


def segmento(hueso, nombre_material, a, b, radio_a, radio_b=None, lados=10):
    """Tronco de cono entre los puntos a y b."""
    a, b = Vector(a), Vector(b)
    direccion = b - a
    bpy.ops.mesh.primitive_cone_add(vertices=lados, radius1=radio_a, radius2=radio_b if radio_b is not None else radio_a,
                                    depth=direccion.length, location=(a + b) / 2.0)
    objeto = bpy.context.active_object
    objeto.rotation_mode = "QUATERNION"
    objeto.rotation_quaternion = direccion.to_track_quat("Z", "Y")
    return terminar_pieza(objeto, hueso, nombre_material)


def crear_piezas():
    piezas = []
    # Cabeza: máscara grande y redonda, dos cuernos, ojos oscuros hundidos al frente (-Y).
    piezas.append(esfera("cabeza", "Mascara", (0, 0, 1.56), (0.17, 0.16, 0.19), 20, 14))
    for s in (D, I):
        # Cuernos curvos en dos tramos: salen hacia fuera y se cierran hacia arriba y atrás.
        piezas.append(segmento("cabeza", "Mascara", (s * 0.08, 0, 1.70), (s * 0.14, 0.0, 1.88), 0.04, 0.024, 8))
        piezas.append(segmento("cabeza", "Mascara", (s * 0.14, 0.0, 1.88), (s * 0.16, 0.04, 2.03), 0.024, 0.005, 8))
        piezas.append(esfera("cabeza", "Ojos", (s * 0.06, -0.145, 1.53), (0.04, 0.02, 0.065), 12, 8))
    piezas.append(segmento("cuello", "Cuerpo", (0, 0, 1.31), (0, 0, 1.40), 0.045))
    # Tronco: abdomen estrecho y pecho más ancho.
    piezas.append(segmento("columna", "Cuerpo", (0, 0, 0.90), (0, 0, 1.15), 0.11, 0.13))
    piezas.append(segmento("pecho", "Cuerpo", (0, 0, 1.15), (0, 0, 1.33), 0.13, 0.15))
    # Capa: dos tramos que se abren hacia abajo, detrás (+Y).
    piezas.append(segmento("capa_1", "Capa", (0, 0.10, 1.32), (0, 0.13, 0.96), 0.15, 0.19, 12))
    piezas.append(segmento("capa_2", "Capa", (0, 0.13, 0.96), (0, 0.16, 0.56), 0.19, 0.24, 12))
    piezas.append(segmento("capa_3", "Capa", (0, 0.16, 0.56), (0, 0.18, 0.28), 0.24, 0.27, 12))
    for lado, s in (("d", D), ("i", I)):
        piezas.append(esfera(f"hombro_{lado}", "Capa", (s * 0.16, 0, 1.29), (0.06, 0.06, 0.05)))
        piezas.append(segmento(f"brazo_{lado}", "Cuerpo", (s * 0.17, 0, 1.29), (s * 0.21, 0, 1.03), 0.035, 0.03))
        piezas.append(segmento(f"antebrazo_{lado}", "Cuerpo", (s * 0.21, 0, 1.03), (s * 0.23, -0.02, 0.80), 0.03, 0.026))
        piezas.append(esfera(f"mano_{lado}", "Cuerpo", (s * 0.23, -0.03, 0.76), (0.04, 0.04, 0.05)))
        piezas.append(segmento(f"muslo_{lado}", "Cuerpo", (s * 0.09, 0, 0.90), (s * 0.10, 0, 0.48), 0.055, 0.042))
        piezas.append(segmento(f"pierna_{lado}", "Cuerpo", (s * 0.10, 0, 0.48), (s * 0.10, 0.02, 0.09), 0.04, 0.03))
        piezas.append(esfera(f"pie_{lado}", "Cuerpo", (s * 0.10, -0.05, 0.05), (0.045, 0.10, 0.045)))
    # Aguijón: hoja fina y clara con guarda, colgando de la mano derecha.
    # Hoja que se afila hacia la punta, guarda ancha y pomo.
    piezas.append(segmento("arma", "Metal", (D * 0.23, -0.04, 0.70), (D * 0.23, -0.04, 0.10), 0.016, 0.003, 4))
    piezas.append(caja("arma", "Metal", (D * 0.23, -0.04, 0.72), (0.11, 0.022, 0.02)))
    piezas.append(esfera("arma", "Metal", (D * 0.23, -0.04, 0.80), (0.018, 0.018, 0.018), 8, 6))
    # Pistola: caja oscura apuntando al frente.
    piezas.append(caja("arma_fuego", "Metal", (D * 0.23, -0.18, 0.78), (0.035, 0.24, 0.05)))
    piezas.append(caja("arma_fuego", "Cuerpo", (D * 0.23, -0.07, 0.73), (0.03, 0.04, 0.08)))
    return piezas


def unir(piezas, armadura):
    bpy.ops.object.select_all(action="DESELECT")
    for pieza in piezas:
        pieza.select_set(True)
    bpy.context.view_layer.objects.active = piezas[0]
    bpy.ops.object.join()
    malla = bpy.context.active_object
    malla.name = NOMBRE_MALLA
    malla.data.name = NOMBRE_MALLA
    bpy.ops.object.shade_smooth()
    malla.parent = armadura
    modificador = malla.modifiers.new("Armadura", "ARMATURE")
    modificador.object = armadura
    return malla


def exportar(malla, armadura, directorio):
    os.makedirs(directorio, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    malla.select_set(True)
    armadura.select_set(True)
    bpy.context.view_layer.objects.active = armadura
    ruta_fbx = os.path.join(directorio, f"{NOMBRE_MALLA}.fbx")
    bpy.ops.export_scene.fbx(
        filepath=ruta_fbx,
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        apply_unit_scale=True,
        apply_scale_options="FBX_SCALE_NONE",
        axis_forward="-Z",
        axis_up="Y",
        mesh_smooth_type="FACE",
        add_leaf_bones=False,
        use_armature_deform_only=True,
        bake_anim=False,
    )
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(directorio, f"{NOMBRE_MALLA}.blend"))
    triangulos = sum(len(p.vertices) - 2 for p in malla.data.polygons)
    print(f"[SAEVUS arte] exportado {ruta_fbx}: {len(HUESOS)} huesos, {triangulos} triángulos")


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_caballero.py -- <directorio de salida>")
    limpiar_escena()
    armadura = crear_armadura()
    malla = unir(crear_piezas(), armadura)
    exportar(malla, armadura, argumentos[0])


main()
