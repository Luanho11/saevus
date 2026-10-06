"""
Ayudantes de modelado por script para los personajes de SAEVUS (protagonista, goblins, Rey Goblin):
esqueleto, piezas con bmesh (tubos, elipsoides, anillos, láminas con sección en rombo), pesos por
altura, unión y exportación a FBX con las convenciones de Documentos/pipeline-assets.md.

Se importa desde los scripts de personaje (blender_caballero.py, blender_goblins.py), que añaden su
carpeta a sys.path. Geometría simple y clara (REF-02, "Geometría recomendada para Blender").
"""

import math
import os

import bmesh
import bpy
from mathutils import Matrix, Quaternion, Vector

NOMBRE_ARMADURA = "Armature"  # UE trata "Armature" como nodo raíz implícito y no lo añade como hueso.


def limpiar_escena():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    unidades = bpy.context.scene.unit_settings
    unidades.system = "METRIC"
    unidades.scale_length = 1.0

# --- Materiales y esqueleto ----------------------------------------------------------------

# Colores (lineales) por nombre de material: cada script de personaje rellena el suyo con colores_materiales().
MATERIALES = {}


def colores_materiales(colores):
    MATERIALES.update(colores)


def material(nombre):
    existente = bpy.data.materials.get(nombre)
    if existente:
        return existente
    nuevo = bpy.data.materials.new(nombre)
    nuevo.diffuse_color = MATERIALES.get(nombre, (0.5, 0.5, 0.5, 1.0))
    return nuevo


def crear_armadura(huesos):
    """huesos: [(nombre, padre, cabeza, cola)]."""
    datos = bpy.data.armatures.new(NOMBRE_ARMADURA)
    objeto = bpy.data.objects.new(NOMBRE_ARMADURA, datos)
    bpy.context.scene.collection.objects.link(objeto)
    bpy.context.view_layer.objects.active = objeto
    bpy.ops.object.mode_set(mode="EDIT")
    for nombre, padre, cabeza, cola in huesos:
        hueso = datos.edit_bones.new(nombre)
        hueso.head = cabeza
        hueso.tail = cola
        if padre:
            hueso.parent = datos.edit_bones[padre]
            hueso.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    return objeto

# --- Piezas: construcción con bmesh -----------------------------------------------------

def pieza(bm, pesos, materiales, indices_material=None):
    """
    Convierte un bmesh en objeto. pesos: nombre de hueso (peso 1 a todo) o función posición → {hueso: peso}.
    materiales: nombre o lista de nombres; indices_material: índice de material por cara (si hay varios).
    """
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    if indices_material:
        for cara, indice in zip(bm.faces, indices_material):
            cara.material_index = indice
    malla = bpy.data.meshes.new("pieza")
    bm.to_mesh(malla)
    bm.free()
    objeto = bpy.data.objects.new("pieza", malla)
    bpy.context.scene.collection.objects.link(objeto)
    for nombre in [materiales] if isinstance(materiales, str) else materiales:
        malla.materials.append(material(nombre))
    if isinstance(pesos, str):
        grupo = objeto.vertex_groups.new(name=pesos)
        grupo.add(list(range(len(malla.vertices))), 1.0, "REPLACE")
    else:
        grupos = {}
        for vertice in malla.vertices:
            for hueso, peso in pesos(vertice.co).items():
                if peso <= 0.0:
                    continue
                if hueso not in grupos:
                    grupos[hueso] = objeto.vertex_groups.new(name=hueso)
                grupos[hueso].add([vertice.index], peso, "REPLACE")
    return objeto


def marco(tangente, lateral_deseado):
    """Ejes (lateral, normal) perpendiculares a la tangente, con el lateral lo más cerca posible del deseado."""
    t = tangente.normalized()
    lateral = lateral_deseado - t * lateral_deseado.dot(t)
    if lateral.length < 1e-4:
        lateral = Vector((0, 0, 1)) - t * t.z
    lateral.normalize()
    return lateral, t.cross(lateral).normalized()


def lofting(anillos, lados, cerrar=True):
    """
    Malla tubular por anillos elípticos: cada anillo es (centro, eje_u, eje_v, radio_u, radio_v).
    Devuelve (bmesh, caras). Con cerrar, tapa los extremos.
    """
    bm = bmesh.new()
    filas = []
    for centro, eje_u, eje_v, radio_u, radio_v in anillos:
        fila = []
        for k in range(lados):
            angulo = 2.0 * math.pi * k / lados
            fila.append(bm.verts.new(centro + eje_u * (radio_u * math.cos(angulo)) + eje_v * (radio_v * math.sin(angulo))))
        filas.append(fila)
    for a, b in zip(filas, filas[1:]):
        for k in range(lados):
            bm.faces.new((a[k], a[(k + 1) % lados], b[(k + 1) % lados], b[k]))
    if cerrar:
        bm.faces.new(list(reversed(filas[0])))
        bm.faces.new(filas[-1])
    return bm


def tubo(hueso, nombre_material, puntos, radios, lados=10, achatado=1.0, lateral=Vector((1, 0, 0))):
    """Tubo a lo largo de una polilínea con un radio por punto; achatado < 1 aplana el eje normal."""
    puntos = [Vector(p) for p in puntos]
    anillos = []
    for i, p in enumerate(puntos):
        tangente = (puntos[min(i + 1, len(puntos) - 1)] - puntos[max(i - 1, 0)])
        eje_u, eje_v = marco(tangente, lateral)
        anillos.append((p, eje_u, eje_v, radios[i], radios[i] * achatado))
    return pieza(lofting(anillos, lados), hueso, nombre_material)


def elipsoide(hueso, nombre_material, centro, radios, segmentos=14, anillos=9):
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=segmentos, v_segments=anillos, radius=1.0)
    bmesh.ops.transform(bm, matrix=Matrix.Translation(Vector(centro)) @ Matrix.Diagonal((*radios, 1.0)), verts=bm.verts)
    return pieza(bm, hueso, nombre_material)


def anillo_metalico(hueso, nombre_material, centro, eje, radio, grosor):
    """Anillo (toro) con el agujero según eje: broche, guarda, hebillas."""
    bm = bmesh.new()
    lados_mayor, lados_menor = 16, 6
    eje = Vector(eje).normalized()
    u, v = marco(eje, Vector((0, 0, 1)) if abs(eje.z) < 0.9 else Vector((1, 0, 0)))
    filas = []
    for i in range(lados_mayor):
        a = 2.0 * math.pi * i / lados_mayor
        radial = u * math.cos(a) + v * math.sin(a)
        fila = []
        for j in range(lados_menor):
            b = 2.0 * math.pi * j / lados_menor
            fila.append(bm.verts.new(Vector(centro) + radial * (radio + grosor * math.cos(b)) + eje * (grosor * math.sin(b))))
        filas.append(fila)
    for i in range(lados_mayor):
        for j in range(lados_menor):
            a, b = filas[i], filas[(i + 1) % lados_mayor]
            bm.faces.new((a[j], b[j], b[(j + 1) % lados_menor], a[(j + 1) % lados_menor]))
    return pieza(bm, hueso, nombre_material)


def perfil_hoja(t):
    """Hoja de árbol: base estrecha, ancha al 30 %, punta afilada."""
    return 0.45 + 0.55 * t / 0.3 if t < 0.3 else (1.0 - t) / 0.7


def perfil_rombo(t):
    """Rombo: punta abajo y arriba, ancho al 55 %."""
    return max(0.06, t / 0.55 if t < 0.55 else (1.0 - t) / 0.45)


def perfil_tira(t):
    """Tira de tela rota: ancho casi constante y punta irregular al final."""
    return 1.0 if t < 0.72 else max(0.0, (1.0 - t) / 0.28)


def perfil_pua(t):
    return 1.0 - t


def lamina(pesos, nombre_material, base, direccion, lateral, largo, ancho, grosor, perfil=perfil_hoja,
           curva=0.0, eje_curva=None, segmentos=6, desvio_punta=0.0):
    """
    Lámina con sección en rombo (filo a los lados, lomo en el centro) a lo largo de una espina curva:
    placas del casco, aletas, tiras de capa y faldón, garras, púas. curva: grados que gira la espina
    de la base a la punta alrededor de eje_curva; desvio_punta: la punta se aparta de lado (tiras rotas).
    """
    direccion = Vector(direccion).normalized()
    eje_curva = Vector(eje_curva).normalized() if eje_curva is not None else direccion.cross(Vector(lateral)).normalized()
    paso = largo / segmentos
    punto = Vector(base)
    anillos = []
    for i in range(segmentos + 1):
        t = i / segmentos
        giro = Quaternion(eje_curva, math.radians(curva * t))
        tangente = giro @ direccion
        eje_u, eje_v = marco(tangente, Vector(lateral))
        mitad = max(1e-4, 0.5 * ancho * perfil(t))
        centro = punto + eje_u * (desvio_punta * t * t)
        anillos.append((centro, eje_u, eje_v, mitad, max(1e-4, 0.5 * grosor * (0.35 + 0.65 * perfil(t)))))
        punto = punto + tangente * paso
    return pieza(lofting(anillos, 4), pesos, nombre_material)

# --- Pesos repartidos ---------------------------------------------------------------------

def interpolar_por_altura(z, tramos):
    """tramos: [(altura, hueso)] de arriba abajo; reparte el peso entre los dos huesos que rodean z."""
    if z >= tramos[0][0]:
        return {tramos[0][1]: 1.0}
    for (z_a, hueso_a), (z_b, hueso_b) in zip(tramos, tramos[1:]):
        if z >= z_b:
            f = (z_a - z) / (z_a - z_b)
            return {hueso_a: 1.0 - f, hueso_b: f} if hueso_a != hueso_b else {hueso_a: 1.0}
    return {tramos[-1][1]: 1.0}


def unir(piezas, armadura, nombre_malla):
    bpy.ops.object.select_all(action="DESELECT")
    for p in piezas:
        p.select_set(True)
    bpy.context.view_layer.objects.active = piezas[0]
    bpy.ops.object.join()
    malla = bpy.context.active_object
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    malla.name = nombre_malla
    malla.data.name = nombre_malla
    # Sombreado plano con bordes suaves donde la superficie es curva: facetas de placa sin perder volumen.
    bpy.ops.object.shade_smooth_by_angle(angle=math.radians(40))
    malla.parent = armadura
    modificador = malla.modifiers.new("Armadura", "ARMATURE")
    modificador.object = armadura
    return malla


def exportar(malla, armadura, directorio, numero_huesos):
    os.makedirs(directorio, exist_ok=True)
    bpy.ops.object.select_all(action="DESELECT")
    malla.select_set(True)
    armadura.select_set(True)
    bpy.context.view_layer.objects.active = armadura
    ruta_fbx = os.path.join(directorio, f"{malla.name}.fbx")
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
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(directorio, f"{malla.name}.blend"))
    triangulos = sum(len(p.vertices) - 2 for p in malla.data.polygons)
    alto = max(v.co.z for v in malla.data.vertices)
    print(f"[SAEVUS arte] exportado {ruta_fbx}: {numero_huesos} huesos, {triangulos} triángulos, "
          f"{len(malla.data.materials)} materiales, alto {alto:.2f} m")
