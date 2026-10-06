"""
Renderiza todas las piezas de un .blend de escenario en dos filas, en vista isométrica, para compararlas
con el kit modular de REF-07 (Documentos/REFERENCIAS_ARTE.md).
Uso: blender.exe --background <archivo.blend> --python blender_vista_kit.py -- <salida.png>
"""

import math
import sys

import bpy
from mathutils import Vector

salida = sys.argv[sys.argv.index("--") + 1] if "--" in sys.argv else "//kit.png"
escena = bpy.context.scene
piezas = [o for o in escena.objects if o.type == "MESH"]
# Dos filas: las piezas se reparten por anchura acumulada.
mitad = math.ceil(len(piezas) / 2)
for fila, grupo in enumerate((piezas[:mitad], piezas[mitad:])):
    x = 0.0
    for ob in grupo:
        ob.location = (x + ob.dimensions.x / 2, fila * 9.0, 0)
        x += ob.dimensions.x + 1.5

escena.render.engine = "BLENDER_WORKBENCH"
escena.display.shading.light = "STUDIO"
escena.display.shading.color_type = "MATERIAL"
escena.display.shading.show_cavity = True
escena.display.shading.show_object_outline = True
escena.view_settings.view_transform = "Standard"
escena.view_settings.exposure = 2.0
escena.render.resolution_x = 1800
escena.render.resolution_y = 700
mundo = escena.world or bpy.data.worlds.new("Fondo")
escena.world = mundo
mundo.color = (0.006, 0.008, 0.012)

minimo = Vector((min(o.location.x - o.dimensions.x / 2 for o in piezas), -2, 0))
maximo = Vector((max(o.location.x + o.dimensions.x / 2 for o in piezas), 11, 6))
centro = (minimo + maximo) / 2
datos = bpy.data.cameras.new("Camara")
camara = bpy.data.objects.new("Camara", datos)
escena.collection.objects.link(camara)
escena.camera = camara
datos.type = "ORTHO"
datos.ortho_scale = (maximo.x - minimo.x) * 1.05
camara.location = centro + Vector((-20, -40, 30))
camara.rotation_mode = "QUATERNION"
camara.rotation_quaternion = (centro - camara.location).to_track_quat("-Z", "Y")
escena.render.filepath = salida
bpy.ops.render.render(write_still=True)
print(f"[SAEVUS arte] kit en {salida}")
