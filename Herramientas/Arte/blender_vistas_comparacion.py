"""
Renderiza un .blend en cuatro vistas para compararlo con las referencias (Documentos/REFERENCIAS_ARTE.md):
frente, perfil y espalda en ortográfica (proporciones, como REF-01 abajo y REF-02 "silueta y
proporciones") y tres cuartos de espaldas en perspectiva (la cámara del juego).

Uso: blender.exe --background <archivo.blend> --python blender_vistas_comparacion.py -- <directorio de salida> [altura_m]
Escribe <directorio>/vista_{frente,perfil,espalda,juego,tres_cuartos}.png.
"""

import math
import os
import sys

import bpy
from mathutils import Vector

argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
directorio = argumentos[0] if argumentos else "//"
altura = float(argumentos[1]) if len(argumentos) > 1 else 2.0
os.makedirs(directorio, exist_ok=True)

escena = bpy.context.scene
escena.render.engine = "BLENDER_WORKBENCH"
escena.display.shading.light = "STUDIO"
escena.display.shading.color_type = "MATERIAL"
escena.display.shading.show_shadows = False
escena.display.shading.show_cavity = True
escena.display.shading.cavity_type = "WORLD"
escena.display.shading.show_object_outline = True
escena.view_settings.view_transform = "Standard"
escena.render.film_transparent = False
escena.render.resolution_x = 600
escena.render.resolution_y = 900

mundo = escena.world or bpy.data.worlds.new("Fondo")
escena.world = mundo
mundo.color = (0.008, 0.009, 0.010)  # fondo de las hojas de referencia (#17181A)

camara_datos = bpy.data.cameras.new("Camara")
camara = bpy.data.objects.new("Camara", camara_datos)
escena.collection.objects.link(camara)
escena.camera = camara
camara.rotation_mode = "QUATERNION"

centro = Vector((0.0, 0.0, altura * 0.5))
VISTAS = {
    # El personaje mira a -Y en Blender.
    "frente": (Vector((0, -6, altura * 0.5)), True),
    "perfil": (Vector((-6, 0, altura * 0.5)), True),
    "espalda": (Vector((0, 6, altura * 0.5)), True),
    # Detrás, algo por encima y a un lado: como la cámara de tercera persona.
    "juego": (Vector((1.2 * altura / 2.0, 3.4 * altura / 2.0, 2.1 * altura / 2.0)), False),
    # Tres cuartos de frente, a la altura del pecho (como las hojas de personaje de las referencias).
    "tres_cuartos": (Vector((-1.5 * altura / 2.0, -2.6 * altura / 2.0, 1.2 * altura / 2.0)), False),
}
for nombre, (posicion, ortografica) in VISTAS.items():
    camara.location = posicion
    objetivo = centro if ortografica else Vector((0, 0, altura * 0.55))
    camara.rotation_quaternion = (objetivo - posicion).to_track_quat("-Z", "Y")
    camara_datos.type = "ORTHO" if ortografica else "PERSP"
    camara_datos.ortho_scale = altura * 1.12  # con ajuste automático, la escala va por el lado mayor (el alto)
    camara_datos.lens = 40
    escena.render.filepath = os.path.join(directorio, f"vista_{nombre}.png")
    bpy.ops.render.render(write_still=True)
    print(f"[SAEVUS arte] vista {nombre} en {escena.render.filepath}")
