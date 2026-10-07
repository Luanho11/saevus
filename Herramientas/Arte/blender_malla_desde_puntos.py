"""
Convierte los puntos de superficie de reconstruir_desde_vistas.py en una malla continua con el color de las
referencias (atributo de color por vértice "Color"), en Blender.

Puntos → volumen (nodos de geometría: Points to Volume) → malla (Volume to Mesh) → suavizado y reducción →
color de cada vértice desde el punto más cercano. Escribe un .blend con el objeto "Reconstruccion".

Uso: blender.exe --background --factory-startup --python blender_malla_desde_puntos.py -- <puntos.npz> <salida.blend> [triangulos]
"""

import sys

import bpy
import numpy as np
from mathutils import kdtree

argumentos = sys.argv[sys.argv.index("--") + 1:]
ruta_puntos, ruta_salida = argumentos[0], argumentos[1]
objetivo_triangulos = int(argumentos[2]) if len(argumentos) > 2 else 30000

datos = np.load(ruta_puntos)
bpy.ops.wm.read_factory_settings(use_empty=True)


def malla_de_pieza(nombre, puntos, colores, paso):
    """Puntos de superficie de una pieza → malla continua con su color por vértice."""
    malla = bpy.data.meshes.new(nombre)
    malla.vertices.add(len(puntos))
    malla.vertices.foreach_set("co", puntos.reshape(-1))
    malla.update()
    objeto = bpy.data.objects.new(nombre, malla)
    bpy.context.scene.collection.objects.link(objeto)

    # Nodos: puntos → volumen → malla.
    grupo = bpy.data.node_groups.new("PuntosAMalla", "GeometryNodeTree")
    grupo.interface.new_socket("Geometry", in_out="INPUT", socket_type="NodeSocketGeometry")
    grupo.interface.new_socket("Geometry", in_out="OUTPUT", socket_type="NodeSocketGeometry")
    nodos = grupo.nodes
    entrada, salida = nodos.new("NodeGroupInput"), nodos.new("NodeGroupOutput")
    a_puntos = nodos.new("GeometryNodeMeshToPoints")
    a_volumen = nodos.new("GeometryNodePointsToVolume")
    a_volumen.inputs["Density"].default_value = 1.0
    a_volumen.resolution_mode = "VOXEL_SIZE"
    a_volumen.inputs["Voxel Size"].default_value = paso
    a_volumen.inputs["Radius"].default_value = paso * 1.3
    a_malla = nodos.new("GeometryNodeVolumeToMesh")
    a_malla.resolution_mode = "VOXEL_SIZE"
    a_malla.inputs["Voxel Size"].default_value = paso
    a_malla.inputs["Threshold"].default_value = 0.3
    enlaces = grupo.links
    enlaces.new(entrada.outputs[0], a_puntos.inputs["Mesh"])
    enlaces.new(a_puntos.outputs["Points"], a_volumen.inputs["Points"])
    enlaces.new(a_volumen.outputs["Volume"], a_malla.inputs["Volume"])
    enlaces.new(a_malla.outputs["Mesh"], salida.inputs[0])
    modificador = objeto.modifiers.new("PuntosAMalla", "NODES")
    modificador.node_group = grupo
    bpy.ops.object.select_all(action="DESELECT")
    bpy.context.view_layer.objects.active = objeto
    objeto.select_set(True)
    bpy.ops.object.modifier_apply(modifier=modificador.name)

    # Suavizado ligero (quita el escalonado de los vóxeles).
    suavizado = objeto.modifiers.new("Suavizado", "SMOOTH")
    suavizado.factor, suavizado.iterations = 0.5, 3
    bpy.ops.object.modifier_apply(modifier=suavizado.name)

    # Color por vértice: el del punto de superficie más cercano (colores sRGB de la referencia).
    arbol = kdtree.KDTree(len(puntos))
    for indice, punto in enumerate(puntos):
        arbol.insert(punto, indice)
    arbol.balance()
    atributo = objeto.data.color_attributes.new("Color", "BYTE_COLOR", "POINT")
    for vertice in objeto.data.vertices:
        _, indice, _ = arbol.find(vertice.co)
        r, g, b = colores[indice]
        atributo.data[vertice.index].color_srgb = (float(r), float(g), float(b), 1.0)
    objeto.data.materials.clear()  # la malla del volumen trae una ranura vacía
    return objeto


objetos = [malla_de_pieza(str(n), datos[f"{n}_puntos"], datos[f"{n}_colores"], float(datos[f"{n}_paso"])) for n in datos["piezas"]]
bpy.ops.object.select_all(action="DESELECT")
for o in objetos:
    o.select_set(True)
bpy.context.view_layer.objects.active = objetos[0]
if len(objetos) > 1:
    bpy.ops.object.join()
objeto = bpy.context.view_layer.objects.active
objeto.name = "Reconstruccion"
objeto.data.color_attributes.active_color = objeto.data.color_attributes["Color"]
objeto.data.color_attributes.render_color_index = 0
triangulos = sum(len(p.vertices) - 2 for p in objeto.data.polygons)
if triangulos > objetivo_triangulos:
    reduccion = objeto.modifiers.new("Reduccion", "DECIMATE")
    reduccion.ratio = objetivo_triangulos / triangulos
    bpy.ops.object.modifier_apply(modifier=reduccion.name)

material = bpy.data.materials.new("Referencia")
material.use_nodes = True
nodo_color = material.node_tree.nodes.new("ShaderNodeVertexColor")
nodo_color.layer_name = "Color"
principio = material.node_tree.nodes["Principled BSDF"]
material.node_tree.links.new(nodo_color.outputs["Color"], principio.inputs["Base Color"])
principio.inputs["Roughness"].default_value = 1.0
objeto.data.materials.append(material)
bpy.ops.object.shade_smooth()
bpy.ops.wm.save_as_mainfile(filepath=ruta_salida)
print(f"[SAEVUS arte] reconstrucción: {len(objeto.data.vertices)} vértices, "
      f"{sum(len(p.vertices) - 2 for p in objeto.data.polygons)} triángulos → {ruta_salida}")
