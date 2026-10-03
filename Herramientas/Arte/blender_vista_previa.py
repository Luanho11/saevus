"""
Renderiza una vista previa (Workbench, colores de material) de un .blend: tres cuartos de frente.
Uso: blender.exe --background <archivo.blend> --python blender_vista_previa.py -- <salida.png>
"""

import math
import sys

import bpy
from mathutils import Vector

argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
salida = argumentos[0] if argumentos else "//vista_previa.png"

escena = bpy.context.scene
escena.render.engine = "BLENDER_WORKBENCH"
escena.display.shading.light = "STUDIO"
escena.display.shading.color_type = "MATERIAL"
escena.display.shading.show_shadows = True
escena.render.resolution_x = 640
escena.render.resolution_y = 800
escena.render.filepath = salida

mundo = bpy.data.worlds.new("Fondo") if not escena.world else escena.world
escena.world = mundo
mundo.color = (0.35, 0.37, 0.4)

camara_datos = bpy.data.cameras.new("Camara")
camara = bpy.data.objects.new("Camara", camara_datos)
escena.collection.objects.link(camara)
escena.camera = camara
# Tres cuartos de frente (el personaje mira a -Y), a la altura del pecho.
objetivo = Vector((0.0, 0.0, 1.0))
camara.location = Vector((-1.6, -2.6, 1.5))
camara.rotation_mode = "QUATERNION"
camara.rotation_quaternion = (objetivo - camara.location).to_track_quat("-Z", "Y")
camara_datos.lens = 50

bpy.ops.render.render(write_still=True)
print(f"[SAEVUS arte] vista previa en {salida}")
