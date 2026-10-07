"""
Hornea el color de las referencias de una malla densa (atributo de color por vértice) a una textura sobre una
copia reducida con UV: el modelo de juego lleva la pintura de la referencia nítida sin necesitar millones de
vértices. Cycles, en segundo plano.

Uso: blender.exe --background <reconstruccion.blend> --python blender_hornear_color.py -- <salida.blend> <textura.png> [triangulos] [resolucion]
El .blend de entrada tiene el objeto "Reconstruccion" (de blender_malla_desde_puntos.py, sin reducir).
"""

import sys

import bpy

argumentos = sys.argv[sys.argv.index("--") + 1:]
ruta_blend, ruta_textura = argumentos[0], argumentos[1]
triangulos = int(argumentos[2]) if len(argumentos) > 2 else 30000
resolucion = int(argumentos[3]) if len(argumentos) > 3 else 2048

densa = bpy.data.objects["Reconstruccion"]
# Copia con sus propios datos de malla (si los compartiera, cambiar sus materiales cambiaría los de la densa).
baja = densa.copy()
baja.data = densa.data.copy()
baja.name = "Modelo"
bpy.context.scene.collection.objects.link(baja)
bpy.ops.object.select_all(action="DESELECT")
baja.select_set(True)
bpy.context.view_layer.objects.active = baja
total = sum(len(p.vertices) - 2 for p in baja.data.polygons)
if total > triangulos:
    reduccion = baja.modifiers.new("Reduccion", "DECIMATE")
    reduccion.ratio = triangulos / total
    bpy.ops.object.modifier_apply(modifier=reduccion.name)
for atributo in list(baja.data.color_attributes):
    baja.data.color_attributes.remove(atributo)

# UV automáticas con márgenes para que el horneado no sangre entre islas.
bpy.ops.object.mode_set(mode="EDIT")
bpy.ops.mesh.select_all(action="SELECT")
bpy.ops.uv.smart_project(angle_limit=1.15, island_margin=0.004, area_weight=0.0)
bpy.ops.object.mode_set(mode="OBJECT")

imagen = bpy.data.images.new("T_Referencia", resolucion, resolucion)
material = bpy.data.materials.new("Referencia_Textura")
material.use_nodes = True
arbol = material.node_tree
nodo_imagen = arbol.nodes.new("ShaderNodeTexImage")
nodo_imagen.image = imagen
arbol.links.new(nodo_imagen.outputs["Color"], arbol.nodes["Principled BSDF"].inputs["Base Color"])
arbol.nodes.active = nodo_imagen
baja.data.materials.clear()
baja.data.materials.append(material)

# La densa emite su color por vértice: se hornea la emisión (sin luz que lo altere).
material_densa = next(m for m in densa.data.materials if m)
arbol_densa = material_densa.node_tree
emision = arbol_densa.nodes.new("ShaderNodeEmission")
color = next(n for n in arbol_densa.nodes if n.type == "VERTEX_COLOR")
arbol_densa.links.new(color.outputs["Color"], emision.inputs["Color"])
arbol_densa.links.new(emision.outputs["Emission"], arbol_densa.nodes["Material Output"].inputs["Surface"])

escena = bpy.context.scene
escena.render.engine = "CYCLES"
escena.cycles.device = "CPU"
escena.cycles.samples = 1
escena.render.bake.use_selected_to_active = True
escena.render.bake.cage_extrusion = 0.02
escena.render.bake.max_ray_distance = 0.05
escena.render.bake.margin = 8
bpy.ops.object.select_all(action="DESELECT")
densa.select_set(True)
baja.select_set(True)
bpy.context.view_layer.objects.active = baja
bpy.ops.object.bake(type="EMIT")
imagen.filepath_raw = ruta_textura
imagen.file_format = "PNG"
imagen.save()

bpy.data.objects.remove(densa)
bpy.ops.wm.save_as_mainfile(filepath=ruta_blend)
print(f"[SAEVUS arte] horneado: {sum(len(p.vertices) - 2 for p in baja.data.polygons)} triángulos, textura {resolucion} px → {ruta_textura}")
