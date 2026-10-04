"""
Renderiza en fila los cuerpos de enemigos pedidos de Enemigos.blend (Workbench con contorno y cavidades,
colores planos por material: cuerpo oscuro, arma y ojos cálidos), de tres cuartos. Es la "foto" de
presentación de los bocetos; no la usa el juego.

Uso (Render-Enemigos.ps1 lo hace):
  blender.exe --background <Enemigos.blend> --python blender_render_enemigos.py -- <salida.png> <nombre> [<nombre>...]
"""

import sys

import bpy
from mathutils import Vector

# Colores de presentación por material (en el juego los pone UE por datos).
COLORES = {"Cuerpo": (0.09, 0.13, 0.08, 1.0), "Mascara": (1.0, 0.72, 0.3, 1.0)}
SEPARACION = 2.6
FONDO = (0.09, 0.1, 0.14)

argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
if len(argumentos) < 2:
    raise SystemExit("uso: blender --background Enemigos.blend --python blender_render_enemigos.py -- <salida.png> <nombre>...")
salida, nombres = argumentos[0], argumentos[1:]

escena = bpy.context.scene
for objeto in list(escena.objects):
    objeto.hide_render = objeto.name not in nombres
for nombre, color in COLORES.items():
    if nombre in bpy.data.materials:
        bpy.data.materials[nombre].diffuse_color = color

# En fila a lo largo de Y, en el orden pedido, de cara a la cámara y apoyados en el suelo (el origen es el
# centro de la cápsula).
y = 0.0
for nombre in nombres:
    objeto = bpy.data.objects[nombre]
    abajo = min((objeto.matrix_world @ Vector(v)).z for v in objeto.bound_box) - objeto.location.z
    ancho = max(1.0, objeto.dimensions.y / 1.6)
    y += SEPARACION * ancho * 0.5
    objeto.location = Vector((0.0, y, -abajo))
    y += SEPARACION * ancho * 0.5

escena.render.engine = "BLENDER_WORKBENCH"
sombreado = escena.display.shading
sombreado.light = "STUDIO"
sombreado.color_type = "MATERIAL"
sombreado.show_shadows = True
sombreado.show_cavity = True
sombreado.show_object_outline = True
sombreado.object_outline_color = (0.0, 0.0, 0.0)
escena.render.resolution_x = 1600
escena.render.resolution_y = 800
escena.render.filepath = salida
mundo = escena.world or bpy.data.worlds.new("Fondo")
escena.world = mundo
mundo.color = FONDO

centro = Vector((0.0, y * 0.5, 0.0))
camara_datos = bpy.data.cameras.new("Camara")
camara = bpy.data.objects.new("Camara", camara_datos)
escena.collection.objects.link(camara)
escena.camera = camara
# Los cuerpos miran a +X: la cámara delante, un poco ladeada y algo por encima.
objetivo = centro + Vector((0.0, 0.0, 1.0))
camara.location = centro + Vector((13.0, -3.5, 3.2))
camara.rotation_mode = "QUATERNION"
camara.rotation_quaternion = (objetivo - camara.location).to_track_quat("-Z", "Y")
camara_datos.lens = 40

bpy.ops.render.render(write_still=True)
print(f"[SAEVUS arte] render en {salida}")
