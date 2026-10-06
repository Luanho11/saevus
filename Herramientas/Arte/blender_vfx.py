"""
Genera las formas de los efectos visuales de SAEVUS en Blender (REF-08 D–L; ARTE_SAEVUS.md, VFX).

Las partículas de SAEVUS son mallas instanciadas que mueve la CPU (USAEVUSComponenteParticulas); hasta ahora
eran cubos. Estas formas dan el lenguaje de la referencia con muy pocos triángulos:
- SM_VFX_Chispa: rombo alargado (impactos, bloqueo, parry): estrellas de chispas blancas o azules.
- SM_VFX_Gota y SM_VFX_Salpicadura: sangre estilizada (gota en lágrima y mancha plana de bordes irregulares).
- SM_VFX_Fragmento: esquirla angulosa (muertes que se deshacen, impactos fuertes, escombros).
- SM_VFX_Mota: mota de polvo/ceniza facetada.
- SM_VFX_MediaLuna: media luna fina que se afila en las puntas (estela de los tajos).
- SM_VFX_Anillo: anillo plano (telegrafiado en el suelo, polvo del aterrizaje, onda del parry).
- SM_VFX_Estela: lámina larga en punta (estelas del dash y de la carrera por pared).
Todas caben en un cubo de 1 m con el pivote en el centro, como el cubo del motor que sustituyen, y apuntan
a +X en UE (la dirección de la velocidad cuando la partícula se orienta). Un material por pieza ("Efecto").

Ejecución sin interfaz (Generar-VFX.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_vfx.py -- <directorio de salida>
"""

import math
import os
import random
import sys

import bmesh
import bpy
from mathutils import Vector

SEMILLA = 5


def limpiar_escena():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1.0


def objeto(nombre, bm):
    bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
    malla = bpy.data.meshes.new(nombre)
    bm.to_mesh(malla)
    bm.free()
    ob = bpy.data.objects.new(nombre, malla)
    bpy.context.scene.collection.objects.link(ob)
    material = bpy.data.materials.get("Efecto") or bpy.data.materials.new("Efecto")
    malla.materials.append(material)
    return ob

# En Blender el eje que llega a +X de UE es -Y (adelante -Z del FBX). Las formas se orientan en -Y.


def chispa():
    """Rombo alargado de 1 m de largo: estrella de chispas al golpear."""
    bm = bmesh.new()
    punta, cola = bm.verts.new((0, -0.5, 0)), bm.verts.new((0, 0.5, 0))
    anillo = [bm.verts.new((math.cos(a) * 0.09, -0.12, math.sin(a) * 0.09)) for a in (0, math.pi / 2, math.pi, 1.5 * math.pi)]
    for k in range(4):
        bm.faces.new((punta, anillo[k], anillo[(k + 1) % 4]))
        bm.faces.new((cola, anillo[(k + 1) % 4], anillo[k]))
    return bm


def gota():
    """Gota en lágrima: redonda detrás, en punta delante (la sangre que sale despedida)."""
    bm = bmesh.new()
    perfil = [(-0.5, 0.0), (-0.3, 0.12), (0.0, 0.26), (0.25, 0.28), (0.42, 0.18), (0.5, 0.0)]
    lados = 8
    filas = []
    for y, r in perfil:
        if r == 0.0:
            filas.append([bm.verts.new((0, y, 0))])
        else:
            filas.append([bm.verts.new((math.cos(2 * math.pi * k / lados) * r, y, math.sin(2 * math.pi * k / lados) * r)) for k in range(lados)])
    for a, b in zip(filas, filas[1:]):
        if len(a) == 1:
            for k in range(lados):
                bm.faces.new((a[0], b[k], b[(k + 1) % lados]))
        elif len(b) == 1:
            for k in range(lados):
                bm.faces.new((a[k], b[0], a[(k + 1) % lados]))
        else:
            for k in range(lados):
                bm.faces.new((a[k], b[k], b[(k + 1) % lados], a[(k + 1) % lados]))
    return bm


def salpicadura():
    """Mancha plana de bordes irregulares con lóbulos (REF-08 G, salpicadura y mancha)."""
    azar = random.Random(SEMILLA)
    bm = bmesh.new()
    centro = bm.verts.new((0, 0, 0.0))
    puntos = 18
    borde = []
    for k in range(puntos):
        a = 2 * math.pi * k / puntos
        r = 0.32 + (0.18 if k % 3 == 0 else 0.0) * azar.uniform(0.6, 1.0) + azar.uniform(-0.05, 0.05)
        borde.append(bm.verts.new((math.cos(a) * r, math.sin(a) * r, 0.0)))
    for k in range(puntos):
        bm.faces.new((centro, borde[k], borde[(k + 1) % puntos]))
    # Grosor mínimo para que se vea de los dos lados.
    resultado = bmesh.ops.extrude_face_region(bm, geom=list(bm.faces))
    bmesh.ops.translate(bm, vec=Vector((0, 0, 0.02)), verts=[g for g in resultado["geom"] if isinstance(g, bmesh.types.BMVert)])
    return bm


def fragmento():
    """Esquirla angulosa irregular: prisma triangular torcido y afilado."""
    azar = random.Random(SEMILLA + 1)
    bm = bmesh.new()
    base = [bm.verts.new((math.cos(a) * azar.uniform(0.18, 0.3), 0.25, math.sin(a) * azar.uniform(0.18, 0.3)))
            for a in (0.3, 2.4, 4.3)]
    medio = [bm.verts.new((v.co.x * 0.8 + 0.05, -0.1, v.co.z * 0.8)) for v in base]
    punta = bm.verts.new((0.06, -0.5, 0.04))
    bm.faces.new(list(reversed(base)))
    for k in range(3):
        bm.faces.new((base[k], base[(k + 1) % 3], medio[(k + 1) % 3], medio[k]))
        bm.faces.new((medio[k], medio[(k + 1) % 3], punta))
    return bm


def mota():
    """Mota de polvo o ceniza: icosaedro achatado (facetas que atrapan la luz)."""
    bm = bmesh.new()
    bmesh.ops.create_icosphere(bm, subdivisions=1, radius=0.5)
    bmesh.ops.scale(bm, vec=Vector((1.0, 1.0, 0.7)), verts=bm.verts)
    return bm


def media_luna(grados=150, segmentos=16):
    """Media luna fina en el plano horizontal, gruesa en el centro y afilada en las puntas (estela del tajo)."""
    bm = bmesh.new()
    radio = 0.5
    filas = []
    for k in range(segmentos + 1):
        t = k / segmentos
        a = math.radians(-grados / 2 + grados * t)
        ancho = 0.16 * math.sin(math.pi * t) ** 0.8
        fuera = (math.sin(a) * radio, -math.cos(a) * radio, 0)
        dentro = (math.sin(a) * (radio - ancho - 0.003), -math.cos(a) * (radio - ancho - 0.003), 0)
        filas.append((bm.verts.new(fuera), bm.verts.new(dentro)))
    for (a1, a2), (b1, b2) in zip(filas, filas[1:]):
        bm.faces.new((a1, b1, b2, a2))
    resultado = bmesh.ops.extrude_face_region(bm, geom=list(bm.faces))
    bmesh.ops.translate(bm, vec=Vector((0, 0, 0.015)), verts=[g for g in resultado["geom"] if isinstance(g, bmesh.types.BMVert)])
    bmesh.ops.translate(bm, vec=Vector((0, 0.25, -0.0075)), verts=bm.verts)
    return bm


def anillo(segmentos=32, grosor=0.06):
    """Anillo plano de 1 m de diámetro (telegrafiado rojo en el suelo, polvo del aterrizaje, onda del parry)."""
    bm = bmesh.new()
    fuera = [bm.verts.new((math.cos(2 * math.pi * k / segmentos) * 0.5, math.sin(2 * math.pi * k / segmentos) * 0.5, 0)) for k in range(segmentos)]
    dentro = [bm.verts.new((math.cos(2 * math.pi * k / segmentos) * (0.5 - grosor), math.sin(2 * math.pi * k / segmentos) * (0.5 - grosor), 0))
              for k in range(segmentos)]
    for k in range(segmentos):
        bm.faces.new((fuera[k], fuera[(k + 1) % segmentos], dentro[(k + 1) % segmentos], dentro[k]))
    resultado = bmesh.ops.extrude_face_region(bm, geom=list(bm.faces))
    bmesh.ops.translate(bm, vec=Vector((0, 0, 0.01)), verts=[g for g in resultado["geom"] if isinstance(g, bmesh.types.BMVert)])
    return bm


def estela():
    """Lámina larga en punta (cola hacia +Y): estela del dash, de la carrera por pared y del salto."""
    bm = bmesh.new()
    contorno = [(0, -0.5), (0.07, -0.3), (0.05, 0.2), (0.0, 0.5), (-0.05, 0.2), (-0.07, -0.3)]
    arriba = [bm.verts.new((x, y, 0.01)) for x, y in contorno]
    abajo = [bm.verts.new((x, y, -0.01)) for x, y in contorno]
    bm.faces.new(arriba)
    bm.faces.new(list(reversed(abajo)))
    n = len(contorno)
    for k in range(n):
        bm.faces.new((arriba[k], abajo[k], abajo[(k + 1) % n], arriba[(k + 1) % n]))
    return bm


FORMAS = {
    "SM_VFX_Chispa": chispa,
    "SM_VFX_Gota": gota,
    "SM_VFX_Salpicadura": salpicadura,
    "SM_VFX_Fragmento": fragmento,
    "SM_VFX_Mota": mota,
    "SM_VFX_MediaLuna": media_luna,
    "SM_VFX_Anillo": anillo,
    "SM_VFX_Estela": estela,
}


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_vfx.py -- <directorio de salida>")
    directorio = argumentos[0]
    os.makedirs(directorio, exist_ok=True)
    limpiar_escena()
    for indice, (nombre, crear) in enumerate(FORMAS.items()):
        ob = objeto(nombre, crear())
        bpy.ops.object.select_all(action="DESELECT")
        ob.select_set(True)
        bpy.context.view_layer.objects.active = ob
        ruta = os.path.join(directorio, f"{nombre}.fbx")
        bpy.ops.export_scene.fbx(filepath=ruta, use_selection=True, object_types={"MESH"}, apply_unit_scale=True,
                                 apply_scale_options="FBX_SCALE_NONE", axis_forward="-Z", axis_up="Y", mesh_smooth_type="FACE")
        print(f"[SAEVUS arte] {nombre}: {sum(len(p.vertices) - 2 for p in ob.data.polygons)} triángulos")
        ob.location.x = indice * 1.5
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(directorio, "VFX.blend"))


main()
