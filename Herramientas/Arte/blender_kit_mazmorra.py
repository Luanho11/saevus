"""
Genera el kit modular de la mazmorra de SAEVUS en Blender (REF-07, "kit modular"; ARTE_SAEVUS.md, Mazmorra):
sillería de piedra pizarra con aristas achaflanadas y marcos en relieve, piezas en una rejilla común.

Rejilla: 2 m en planta, 1 m en altura (muros de 4 m: caben la carrera y el salto de pared, pipeline-assets.md,
Métricas). Origen de cada pieza en el centro de su base; frente hacia -Y en Blender. Una malla estática por
pieza (FBX) y un .blend con todas. Materiales por nombre: Piedra, Borde (aristas y marcos, algo más claros),
Madera, Metal, Tela (estandartes rojos), Fuego (braseros). La colisión de los muros para correr es una sola
cara plana: el relieve no sobresale más de 6 cm del plano de colisión.

Ejecución sin interfaz (Generar-KitMazmorra.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_kit_mazmorra.py -- <directorio de salida>
"""

import math
import os
import random
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector

SEMILLA = 21
REJILLA = 2.0
ALTO_MURO = 4.0

# Colores lineales medidos en REF-07 (ARTE_SAEVUS.md, Paleta); en UE los pone el material de escenario.
COLORES = {
    "Piedra": (0.022, 0.034, 0.050, 1.0),   # pizarra azulada #263E54 apagada
    "Borde": (0.060, 0.072, 0.090, 1.0),    # aristas y marcos #3C4855 → algo más claros
    "Madera": (0.030, 0.022, 0.018, 1.0),
    "Metal": (0.040, 0.045, 0.052, 1.0),
    "Tela": (0.24, 0.07, 0.05, 1.0),        # estandartes #88503C
    "Fuego": (0.75, 0.40, 0.20, 1.0),       # brasas #E0A97E
}


def limpiar_escena():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.0


def material(nombre):
    existente = bpy.data.materials.get(nombre)
    if existente:
        return existente
    nuevo = bpy.data.materials.new(nombre)
    nuevo.diffuse_color = COLORES[nombre]
    return nuevo


def objeto(bm, nombre_material):
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    malla = bpy.data.meshes.new("pieza")
    bm.to_mesh(malla)
    bm.free()
    ob = bpy.data.objects.new("pieza", malla)
    bpy.context.scene.collection.objects.link(ob)
    malla.materials.append(material(nombre_material))
    return ob


def bloque(nombre_material, tamano, centro, chaflan=0.04, giro_z=0.0, giro_x=0.0):
    """Caja con las aristas achaflanadas (el chaflán atrapa la luz y dibuja el borde, como en REF-07)."""
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    bmesh.ops.scale(bm, vec=Vector(tamano), verts=bm.verts)
    if chaflan > 0:
        chaflan = min(chaflan, 0.45 * min(tamano))
        bmesh.ops.bevel(bm, geom=list(bm.edges), offset=chaflan, segments=1, affect="EDGES", profile=0.5)
    giro = Matrix.Rotation(giro_z, 4, "Z") @ Matrix.Rotation(giro_x, 4, "X")
    bmesh.ops.transform(bm, matrix=Matrix.Translation(Vector(centro)) @ giro, verts=bm.verts)
    return objeto(bm, nombre_material)


def prisma(nombre_material, contorno, alto, base_z=0.0, eje="Z", desplazar=(0, 0, 0)):
    """Prisma por extrusión de un contorno 2D (en XZ si eje == 'Y', en XY si eje == 'Z')."""
    bm = bmesh.new()
    verts = []
    for u, w in contorno:
        verts.append(bm.verts.new((u, 0, w) if eje == "Y" else (u, w, base_z)))
    cara = bm.faces.new(verts)
    direccion = Vector((0, alto, 0)) if eje == "Y" else Vector((0, 0, alto))
    resultado = bmesh.ops.extrude_face_region(bm, geom=[cara])
    nuevos = [g for g in resultado["geom"] if isinstance(g, bmesh.types.BMVert)]
    bmesh.ops.translate(bm, vec=direccion, verts=nuevos)
    bmesh.ops.translate(bm, vec=Vector(desplazar), verts=bm.verts)
    return objeto(bm, nombre_material)


def marco_relieve(ancho, alto, z0, y_cara, grosor=0.12, relieve=0.05):
    """Moldura en relieve alrededor de un panel de muro (cuatro listones achaflanados)."""
    y = y_cara - relieve / 2
    return [
        bloque("Borde", (ancho, relieve, grosor), (0, y, z0 + grosor / 2), 0.015),
        bloque("Borde", (ancho, relieve, grosor), (0, y, z0 + alto - grosor / 2), 0.015),
        bloque("Borde", (grosor, relieve, alto), (-ancho / 2 + grosor / 2, y, z0 + alto / 2), 0.015),
        bloque("Borde", (grosor, relieve, alto), (ancho / 2 - grosor / 2, y, z0 + alto / 2), 0.015),
    ]

# --- Piezas --------------------------------------------------------------------------------

def suelo(tam=2):
    """Losa de suelo de tam × tam celdas: cuatro losas por celda y borde, como en REF-07."""
    lado = REJILLA * tam
    piezas = []
    paso = lado / (2 * tam)
    for i in range(2 * tam):
        for j in range(2 * tam):
            x = -lado / 2 + paso * (i + 0.5)
            y = -lado / 2 + paso * (j + 0.5)
            piezas.append(bloque("Piedra", (paso - 0.03, paso - 0.03, 0.25), (x, y, -0.125), 0.025))
    return piezas


def pared(ancho=2, alto=ALTO_MURO, grosor=0.6, panel=True):
    """Muro de ancho celdas: sillería en hiladas, zócalo y cornisa, con un panel enmarcado delante."""
    largo = REJILLA * ancho
    piezas = [
        bloque("Piedra", (largo, grosor, alto - 0.6), (0, 0, 0.3 + (alto - 0.6) / 2), 0.03),
        bloque("Borde", (largo, grosor + 0.12, 0.3), (0, 0, 0.15), 0.04),        # zócalo
        bloque("Borde", (largo, grosor + 0.16, 0.3), (0, 0, alto - 0.15), 0.04),  # cornisa
    ]
    # Hiladas: juntas finas en relieve negativo simuladas con listones apenas salientes.
    for z in (1.3, 2.3, 3.2):
        if z < alto - 0.5:
            piezas.append(bloque("Borde", (largo - 0.02, 0.02, 0.05), (0, -grosor / 2 - 0.005, z), 0.0))
    if panel:
        for k in range(ancho):
            x = -largo / 2 + REJILLA * (k + 0.5)
            for p in marco_relieve(REJILLA - 0.5, alto - 1.4, 0.6, -grosor / 2):
                p.location.x += x
                piezas.append(p)
    return piezas


def pared_carrera(ancho=4):
    """Muro largo para correr por la pared: cara plana con una banda guía horizontal a la altura del paso."""
    largo = REJILLA * ancho
    piezas = pared(ancho, panel=False)
    piezas.append(bloque("Borde", (largo, 0.05, 0.16), (0, -0.3 - 0.025, 1.6), 0.015))
    piezas.append(bloque("Borde", (largo, 0.05, 0.08), (0, -0.3 - 0.025, 2.4), 0.01))
    return piezas


def pared_curva(radio=4.0, grados=45, alto=ALTO_MURO, grosor=0.6):
    """Tramo de muro curvo (REF-07, muros de las salas octogonales y redondas)."""
    piezas = []
    pasos = 4
    for k in range(pasos):
        a = math.radians(-grados / 2 + grados * (k + 0.5) / pasos)
        cuerda = 2 * radio * math.sin(math.radians(grados / pasos) / 2) + 0.02
        centro = (math.sin(a) * radio, -math.cos(a) * radio + radio, 0)
        for tam, z, mat in (((cuerda, grosor, alto - 0.6), 0.3 + (alto - 0.6) / 2, "Piedra"), ((cuerda, grosor + 0.12, 0.3), 0.15, "Borde"),
                            ((cuerda, grosor + 0.16, 0.3), alto - 0.15, "Borde")):
            piezas.append(bloque(mat, tam, (centro[0], centro[1], z), 0.03, giro_z=a))
    return piezas


def columna(alto=6.0, ancho=0.8):
    """Columna cuadrada con basa escalonada y capitel (REF-07)."""
    return [
        bloque("Borde", (ancho + 0.3, ancho + 0.3, 0.3), (0, 0, 0.15), 0.04),
        bloque("Borde", (ancho + 0.15, ancho + 0.15, 0.25), (0, 0, 0.42), 0.03),
        bloque("Piedra", (ancho, ancho, alto - 1.1), (0, 0, 0.55 + (alto - 1.1) / 2), 0.05),
        bloque("Borde", (ancho + 0.15, ancho + 0.15, 0.25), (0, 0, alto - 0.42), 0.03),
        bloque("Borde", (ancho + 0.35, ancho + 0.35, 0.3), (0, 0, alto - 0.15), 0.04),
    ]


def esquina(alto=ALTO_MURO):
    """Pilastra de esquina en L que tapa la unión de dos muros."""
    return [
        bloque("Borde", (1.0, 1.0, 0.3), (0, 0, 0.15), 0.04),
        bloque("Piedra", (0.8, 0.8, alto - 0.6), (0, 0, 0.3 + (alto - 0.6) / 2), 0.05),
        bloque("Borde", (1.0, 1.0, 0.3), (0, 0, alto - 0.15), 0.04),
    ]


def arco(ancho=2, alto=ALTO_MURO + 1.0, grosor=0.8, hueco=2.4, alto_hueco=3.4):
    """Muro con arco apuntado (REF-07): jambas, dovelas en la clave y muro encima."""
    largo = REJILLA * ancho
    lado = (largo - hueco) / 2
    piezas = [
        bloque("Piedra", (lado, grosor, alto), (-largo / 2 + lado / 2, 0, alto / 2), 0.04),
        bloque("Piedra", (lado, grosor, alto), (largo / 2 - lado / 2, 0, alto / 2), 0.04),
        bloque("Borde", (largo + 0.1, grosor + 0.2, 0.35), (0, 0, alto - 0.175), 0.04),
    ]
    # Arco: dovelas en semicírculo apuntado sobre el hueco, y relleno de muro encima.
    radio = hueco * 0.62
    arranque = alto_hueco - hueco * 0.55
    for k in range(9):
        t = (k + 0.5) / 9
        lado_izq = t < 0.5
        u = t * 2 if lado_izq else (1 - t) * 2
        angulo = math.radians(90 * u)
        x0 = -hueco / 2 + radio if lado_izq else hueco / 2 - radio
        x = x0 - radio * math.cos(angulo) if lado_izq else x0 + radio * math.cos(angulo)
        z = arranque + radio * math.sin(angulo) * 0.9
        giro = (math.pi / 2 - angulo) * (1 if lado_izq else -1)
        piezas.append(bloque("Borde", (0.34, grosor + 0.06, 0.24), (x, 0, z), 0.03, giro_z=0, giro_x=0))
        piezas[-1].rotation_euler = (0, giro, 0)
    piezas.append(bloque("Piedra", (hueco, grosor, alto - alto_hueco - 0.35), (0, 0, alto_hueco + (alto - alto_hueco - 0.35) / 2), 0.03))
    # Enjutas: rellenan entre las jambas y el arco.
    for s in (-1, 1):
        piezas.append(bloque("Piedra", (hueco * 0.3, grosor - 0.04, alto_hueco - arranque + 0.1), (s * hueco * 0.36, 0, (arranque + alto_hueco) / 2), 0.02))
    return piezas


def escalera(ancho=1, subida=2.0, peldanos=8):
    """Escalera de ancho celdas que sube 'subida' metros en 2 m por metro (REF-07, escalera encajonada con zancas)."""
    largo = REJILLA * ancho
    fondo = subida * 2.0
    huella = fondo / peldanos
    tabica = subida / peldanos
    piezas = []
    for k in range(peldanos):
        piezas.append(bloque("Piedra", (largo - 0.4, huella + 0.02, tabica * (k + 1)), (0, -fondo / 2 + huella * (k + 0.5), tabica * (k + 1) / 2), 0.02))
        piezas.append(bloque("Borde", (largo - 0.4, 0.06, 0.04), (0, -fondo / 2 + huella * k + 0.03, tabica * (k + 1) - 0.02), 0.0))
    for s in (-1, 1):
        # Zancas inclinadas a los lados.
        zanca = bloque("Borde", (0.2, math.hypot(fondo, subida) + 0.3, 0.5), (s * (largo / 2 - 0.1), 0, subida / 2 + 0.15), 0.03)
        zanca.rotation_euler = (math.atan2(subida, fondo), 0, 0)
        piezas.append(zanca)
    return piezas


def balcon(ancho=2, fondo=1):
    """Balcón voladizo: losa con canecillos debajo y antepecho delante (REF-07)."""
    largo, profundo = REJILLA * ancho, REJILLA * fondo
    piezas = [bloque("Piedra", (largo, profundo, 0.35), (0, -profundo / 2, -0.175), 0.04),
              bloque("Borde", (largo, 0.25, 0.9), (0, -profundo + 0.125, 0.45), 0.04),
              bloque("Borde", (largo + 0.05, 0.35, 0.12), (0, -profundo + 0.125, 0.96), 0.03)]
    # Canecillos escalonados bajo la losa, del muro hacia fuera.
    for k in range(ancho * 2 + 1):
        x = -largo / 2 + 0.15 + (largo - 0.3) * k / (ancho * 2)
        piezas.append(bloque("Borde", (0.25, profundo * 0.75, 0.25), (x, -profundo * 0.375, -0.475), 0.03))
        piezas.append(bloque("Borde", (0.25, profundo * 0.4, 0.3), (x, -profundo * 0.2, -0.75), 0.03))
    return piezas


def plataforma(ancho=2, fondo=2, alto=2.0):
    """Plataforma tipo mesa sobre cuatro pilares (REF-07, primera fila del kit)."""
    largo, profundo = REJILLA * ancho, REJILLA * fondo
    piezas = suelo(min(ancho, fondo))
    for p in piezas:
        p.location.z += alto
    piezas.append(bloque("Borde", (largo + 0.1, profundo + 0.1, 0.2), (0, 0, alto - 0.35), 0.04))
    for sx in (-1, 1):
        for sy in (-1, 1):
            piezas.append(bloque("Piedra", (0.45, 0.45, alto - 0.45), (sx * (largo / 2 - 0.3), sy * (profundo / 2 - 0.3), (alto - 0.45) / 2), 0.04))
    return piezas


def marco_puerta(ancho=3.0, alto=4.0, grosor=0.9):
    """Marco de puerta: jambas con basa y dintel con cornisa."""
    return [
        bloque("Borde", (0.5, grosor, alto), (-ancho / 2 - 0.25, 0, alto / 2), 0.04),
        bloque("Borde", (0.5, grosor, alto), (ancho / 2 + 0.25, 0, alto / 2), 0.04),
        bloque("Borde", (ancho + 1.3, grosor + 0.15, 0.5), (0, 0, alto + 0.25), 0.05),
        bloque("Borde", (0.7, grosor + 0.1, 0.35), (-ancho / 2 - 0.25, 0, 0.175), 0.04),
        bloque("Borde", (0.7, grosor + 0.1, 0.35), (ancho / 2 + 0.25, 0, 0.175), 0.04),
    ]


def puerta(ancho=3.0, alto=4.0):
    """Puerta de dos hojas de madera con herrajes dentro del marco."""
    piezas = marco_puerta(ancho, alto)
    for s in (-1, 1):
        piezas.append(bloque("Madera", (ancho / 2 - 0.03, 0.18, alto - 0.05), (s * ancho / 4, 0, alto / 2), 0.02))
        for z in (0.8, alto - 0.8):
            piezas.append(bloque("Metal", (ancho / 2 - 0.2, 0.22, 0.1), (s * ancho / 4, 0, z), 0.01))
    return piezas


def techo(ancho=2, fondo=2):
    """Losa de techo con vigas de piedra por debajo."""
    largo, profundo = REJILLA * ancho, REJILLA * fondo
    piezas = [bloque("Piedra", (largo, profundo, 0.4), (0, 0, 0.2), 0.03)]
    for k in range(ancho + 1):
        x = -largo / 2 + largo * k / ancho
        piezas.append(bloque("Borde", (0.35, profundo, 0.45), (x, 0, -0.2), 0.04))
    return piezas


def escombros(semilla, cantidad=9, radio=0.9):
    """Montón de piedras rotas (REF-07): bloques achaflanados torcidos."""
    azar = random.Random(semilla)
    piezas = []
    for k in range(cantidad):
        t = azar.uniform(0.2, 0.55) * (1.2 - k / cantidad * 0.5)
        a = azar.uniform(0, 2 * math.pi)
        r = azar.uniform(0, radio) * (k / cantidad)
        p = bloque("Piedra", (t * azar.uniform(0.8, 1.4), t, t * azar.uniform(0.6, 1.0)), (math.cos(a) * r, math.sin(a) * r, t * 0.35), 0.05)
        p.rotation_euler = (azar.uniform(-0.4, 0.4), azar.uniform(-0.4, 0.4), azar.uniform(0, math.pi))
        piezas.append(p)
    return piezas


def estandarte(alto=3.2, ancho=1.0, semilla=3):
    """Estandarte rojo colgante con la parte baja rota en tiras (REF-07)."""
    azar = random.Random(semilla)
    piezas = [bloque("Metal", (ancho + 0.3, 0.06, 0.06), (0, 0, alto), 0.0)]
    tiras = 4
    for k in range(tiras):
        largo = alto * azar.uniform(0.75, 1.0)
        x = -ancho / 2 + ancho * (k + 0.5) / tiras
        contorno = [(-ancho / tiras / 2, 0), (ancho / tiras / 2, 0), (ancho / tiras / 2 * azar.uniform(0.2, 0.8), -largo),
                    (azar.uniform(-0.05, 0.05), -largo - 0.25), (-ancho / tiras / 2 * azar.uniform(0.2, 0.8), -largo + 0.1)]
        p = prisma("Tela", [(u + x, w + alto - 0.03) for u, w in contorno], 0.02, eje="Y")
        piezas.append(p)
    return piezas


def brasero():
    """Brasero de pie: copa de metal sobre un pie de piedra, con brasas."""
    return [
        bloque("Borde", (0.5, 0.5, 0.2), (0, 0, 0.1), 0.03),
        bloque("Piedra", (0.25, 0.25, 0.9), (0, 0, 0.65), 0.03),
        bloque("Metal", (0.7, 0.7, 0.25), (0, 0, 1.2), 0.06),
        bloque("Fuego", (0.5, 0.5, 0.1), (0, 0, 1.33), 0.04),
    ]


PIEZAS = {
    "SM_Kit_Suelo": lambda: suelo(1),
    "SM_Kit_SueloGrande": lambda: suelo(2),
    "SM_Kit_Pared": lambda: pared(2),
    "SM_Kit_ParedCorta": lambda: pared(1),
    "SM_Kit_ParedCarrera": lambda: pared_carrera(4),
    "SM_Kit_ParedCurva": pared_curva,
    "SM_Kit_Esquina": esquina,
    "SM_Kit_Columna": columna,
    "SM_Kit_Arco": arco,
    "SM_Kit_Escalera": escalera,
    "SM_Kit_Balcon": balcon,
    "SM_Kit_Plataforma": plataforma,
    "SM_Kit_Puerta": puerta,
    "SM_Kit_Marco": marco_puerta,
    "SM_Kit_Techo": techo,
    "SM_Kit_Escombros": lambda: escombros(SEMILLA),
    "SM_Kit_EscombrosGrandes": lambda: escombros(SEMILLA + 1, 14, 1.4),
    "SM_Kit_Estandarte": estandarte,
    "SM_Kit_Brasero": brasero,
}


def unir(piezas, nombre):
    bpy.ops.object.select_all(action="DESELECT")
    for p in piezas:
        p.select_set(True)
    bpy.context.view_layer.objects.active = piezas[0]
    bpy.ops.object.join()
    ob = bpy.context.active_object
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    ob.name = nombre
    ob.data.name = nombre
    return ob


def exportar(ob, directorio):
    bpy.ops.object.select_all(action="DESELECT")
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    ruta = os.path.join(directorio, f"{ob.name}.fbx")
    bpy.ops.export_scene.fbx(filepath=ruta, use_selection=True, object_types={"MESH"}, apply_unit_scale=True,
                             apply_scale_options="FBX_SCALE_NONE", axis_forward="-Z", axis_up="Y", mesh_smooth_type="FACE")
    triangulos = sum(len(p.vertices) - 2 for p in ob.data.polygons)
    print(f"[SAEVUS arte] {ob.name}: {triangulos} triángulos, materiales {[m.name for m in ob.data.materials]}")


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_kit_mazmorra.py -- <directorio de salida>")
    directorio = argumentos[0]
    os.makedirs(directorio, exist_ok=True)
    limpiar_escena()
    x = 0.0
    for nombre, crear in PIEZAS.items():
        ob = unir(crear(), nombre)
        exportar(ob, directorio)
        ancho = ob.dimensions.x
        ob.location.x = x + ancho / 2
        x += ancho + 1.0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(directorio, "KitMazmorra.blend"))


main()
