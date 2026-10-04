"""
Crea o actualiza los assets de SAEVUS: acciones y contexto de entrada, Data Assets,
subclases Blueprint y el mapa de pruebas L_Pruebas.

Idempotente: los assets existentes se cargan y actualizan, nunca se duplican.
El maniquí, las acciones de entrada de plantilla y los materiales de prototipo vienen del
contenido de plantilla del motor copiado en /Game/Characters, /Game/Input y /Game/LevelPrototyping.

Ejecución sin interfaz:
  UnrealEditor-Cmd.exe SAEVUS.uproject -ExecutePythonScript="<este archivo>" -unattended -nullrhi
"""

import os

import unreal

RAIZ = "/Game/SAEVUS"
DIR_ENTRADA = f"{RAIZ}/Entrada"
DIR_DATOS = f"{RAIZ}/Datos"
DIR_BLUEPRINTS = f"{RAIZ}/Blueprints"
DIR_MATERIALES = f"{RAIZ}/Arte/Materiales"
DIR_CABALLERO = f"{RAIZ}/Arte/Personajes/Caballero"
# FBX que genera Herramientas/Arte/Generar-Caballero.ps1 (malla + esqueleto propio).
FBX_CABALLERO = os.path.join(unreal.Paths.project_dir(), "ArteFuente", "Caballero", "SK_Caballero.fbx")

# Piezas del caballero boceto por ranura de material (nombres de material de Blender) y sus
# parámetros de M_SAEVUS_PersonajeDibujado. Tono Hollow Knight: cuerpo casi negro con borde frío,
# máscara clara que brilla un poco para leerse en la oscuridad, capa azul oscuro, metal pálido.
COLOR_BORDE_FRIO = (0.45, 0.6, 0.85, 1.0)
PIEZAS_CABALLERO = {
    "Cuerpo": {"Color": (0.02, 0.02, 0.025, 1.0), "ColorBorde": COLOR_BORDE_FRIO, "IntensidadBorde": 0.5, "AutoIluminacion": 0.0},
    "Mascara": {"Color": (0.85, 0.85, 0.82, 1.0), "ColorBorde": COLOR_BORDE_FRIO, "IntensidadBorde": 0.15, "AutoIluminacion": 0.3},
    "Capa": {"Color": (0.04, 0.05, 0.08, 1.0), "ColorBorde": COLOR_BORDE_FRIO, "IntensidadBorde": 0.35, "AutoIluminacion": 0.0},
    "Metal": {"Color": (0.55, 0.58, 0.62, 1.0), "ColorBorde": COLOR_BORDE_FRIO, "IntensidadBorde": 0.2, "AutoIluminacion": 0.05},
    "Ojos": {"Color": (0.0, 0.0, 0.0, 1.0), "ColorBorde": COLOR_BORDE_FRIO, "IntensidadBorde": 0.0, "AutoIluminacion": 0.0},
}
DIR_ESTILO = f"{RAIZ}/Arte/Estilo"
RUTA_MAPA_PRUEBAS = f"{RAIZ}/Mapas/L_Pruebas"

ACCIONES_PLANTILLA = "/Game/Input/Actions"
IMC_PLANTILLA = "/Game/Input/IMC_Default"
IMC_PLANTILLA_RATON = "/Game/Input/IMC_MouseLook"
MALLA_MANIQUI = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
ANIM_BP_MANIQUI = "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"
MATERIAL_REJILLA = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
MATERIAL_REJILLA_OSCURA = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"
MALLA_CUBO = "/Engine/BasicShapes/Cube"  # 100 cm, pivote en el centro

# Teclas por acción. Las mantenidas dan Triggered (cierto) / Completed (falso) al personaje.
ASIGNACIONES_SAEVUS = {
    "IA_Esprintar": ["LeftShift", "Gamepad_LeftThumbstick"],
    "IA_Caminar": ["LeftAlt"],
    # En teclado el dash es doble toque de WASD (lo detecta el personaje); aquí solo el mando.
    "IA_Dash": ["Gamepad_FaceButton_Right"],
    "IA_Atacar": ["LeftMouseButton", "Gamepad_FaceButton_Left"],
    # Mantener para la guardia con el arma cuerpo a cuerpo (GunZ).
    "IA_Guardia": ["RightMouseButton", "Gamepad_LeftTrigger"],
    # Mantener para gastar alma y curar (Hollow Knight).
    "IA_Curar": ["Q", "Gamepad_LeftShoulder"],
    # Usar lo que hay cerca (bancos).
    "IA_Interactuar": ["E", "Gamepad_DPad_Up"],
    # F1-F5, F9 y F11 son atajos de depuración del motor (BaseInput.ini).
    "IA_AlternarHUD": ["F6"],
    # Esc detiene el Play en el editor: la pausa va en P (y Start en el mando).
    "IA_Pausa": ["P", "Gamepad_Special_Right"],
}

# Arsenal, en orden de ranura (teclas 1-4). Valores de primera pasada; se ajustan en los Data Assets.
# Los campos de arma de fuego son propiedades de FSAEVUSAjusteArmaFuego.
ARMAS = {
    "DA_Arma_Espada": {"nombre_visible": "Espada", "tipo": "cuerpo_a_cuerpo", "tiempo_equipar": 0.2},
    "DA_Arma_Pistola": {
        "nombre_visible": "Pistola", "tipo": "arma_fuego", "tiempo_equipar": 0.25,
        "arma_fuego": {"danio": 20.0, "intervalo_disparo": 0.25, "automatica": False, "tamano_cargador": 12,
                       "tiempo_recarga": 1.2, "alcance": 5000.0, "dispersion_grados": 0.5, "perdigones_por_disparo": 1,
                       "municion_reserva": 24, "reserva_por_golpe_cuerpo_a_cuerpo": 2},
    },
    "DA_Arma_Rifle": {
        "nombre_visible": "Rifle", "tipo": "arma_fuego", "tiempo_equipar": 0.4,
        "arma_fuego": {"danio": 9.0, "intervalo_disparo": 0.1, "automatica": True, "tamano_cargador": 30,
                       "tiempo_recarga": 1.8, "alcance": 6000.0, "dispersion_grados": 1.5, "perdigones_por_disparo": 1,
                       "municion_reserva": 60, "reserva_por_golpe_cuerpo_a_cuerpo": 5},
    },
    "DA_Arma_Escopeta": {
        "nombre_visible": "Escopeta", "tipo": "arma_fuego", "tiempo_equipar": 0.45,
        "arma_fuego": {"danio": 7.0, "intervalo_disparo": 0.8, "automatica": False, "tamano_cargador": 6,
                       "tiempo_recarga": 2.0, "alcance": 2000.0, "dispersion_grados": 6.0, "perdigones_por_disparo": 8,
                       "municion_reserva": 12, "reserva_por_golpe_cuerpo_a_cuerpo": 1},
    },
}

herramientas_assets = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


def registrar(mensaje):
    unreal.log(f"[SAEVUS assets] {mensaje}")


def cargar(ruta):
    asset = eal.load_asset(ruta)
    if asset is None:
        raise RuntimeError(f"Falta un asset necesario: {ruta}")
    return asset


def obtener_o_crear(nombre, directorio, clase_asset, fabrica):
    ruta = f"{directorio}/{nombre}"
    if eal.does_asset_exist(ruta):
        return cargar(ruta)
    asset = herramientas_assets.create_asset(nombre, directorio, clase_asset, fabrica)
    if asset is None:
        raise RuntimeError(f"No se pudo crear {ruta}")
    registrar(f"creado {ruta}")
    return asset


def crear_tecla(nombre):
    tecla = unreal.Key()
    if not tecla.import_text(nombre):
        raise RuntimeError(f"Nombre de tecla desconocido: {nombre}")
    return tecla


def valor_enum(clase_enum, *candidatos):
    """Primer nombre de valor que exista en el enum (la conversión a MAYÚSCULAS de Python varía)."""
    for candidato in candidatos:
        if hasattr(clase_enum, candidato):
            return getattr(clase_enum, candidato)
    raise RuntimeError(f"{clase_enum} no tiene ninguno de {candidatos}")


def obtener_o_crear_blueprint(nombre, clase_padre):
    ruta = f"{DIR_BLUEPRINTS}/{nombre}"
    if eal.does_asset_exist(ruta):
        return cargar(ruta)
    fabrica = unreal.BlueprintFactory()
    fabrica.set_editor_property("parent_class", clase_padre)
    return obtener_o_crear(nombre, DIR_BLUEPRINTS, unreal.Blueprint, fabrica)


def cdo_blueprint(blueprint):
    unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return unreal.get_default_object(blueprint.generated_class())


def crear_material_color(nombre, color, sin_luz=False, directorio=DIR_MATERIALES):
    """
    Material de color plano con un parámetro "Color". Sin luz: emisivo (partículas). Con luz:
    rugosidad 1 y poco especular, para un aspecto dibujado y nada plástico.
    """
    ruta = f"{directorio}/{nombre}"
    if eal.does_asset_exist(ruta):
        material = cargar(ruta)
    else:
        material = herramientas_assets.create_asset(nombre, directorio, unreal.Material, unreal.MaterialFactoryNew())
        registrar(f"creado {ruta}")
    edicion = unreal.MaterialEditingLibrary
    edicion.delete_all_material_expressions(material)
    material.set_editor_property("used_with_instanced_static_meshes", True)
    material.set_editor_property("used_with_skeletal_mesh", True)
    parametro = edicion.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
    parametro.set_editor_property("parameter_name", "Color")
    parametro.set_editor_property("default_value", unreal.LinearColor(*color))
    if sin_luz:
        material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
        edicion.connect_material_property(parametro, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    else:
        material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
        edicion.connect_material_property(parametro, "", unreal.MaterialProperty.MP_BASE_COLOR)
        rugosidad = edicion.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 200)
        rugosidad.set_editor_property("r", 1.0)
        edicion.connect_material_property(rugosidad, "", unreal.MaterialProperty.MP_ROUGHNESS)
        especular = edicion.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 300)
        especular.set_editor_property("r", 0.2)
        edicion.connect_material_property(especular, "", unreal.MaterialProperty.MP_SPECULAR)
    edicion.recompile_material(material)
    return material


def importar_caballero():
    """Importa (o reimporta) el caballero boceto y le pone los materiales de SAEVUS."""
    if not os.path.exists(FBX_CABALLERO):
        raise RuntimeError(f"Falta {FBX_CABALLERO}; ejecuta Herramientas/Arte/Generar-Caballero.ps1")
    tarea = unreal.AssetImportTask()
    tarea.filename = FBX_CABALLERO
    tarea.destination_path = DIR_CABALLERO
    tarea.automated = True
    tarea.replace_existing = True
    tarea.save = False
    herramientas_assets.import_asset_tasks([tarea])
    mallas = [unreal.load_asset(r) for r in tarea.imported_object_paths]
    malla = next((m for m in mallas if isinstance(m, unreal.SkeletalMesh)), None)
    if malla is None:
        raise RuntimeError(f"La importación de {FBX_CABALLERO} no produjo una malla esquelética ({list(tarea.imported_object_paths)})")

    registrar(f"caballero importado: {malla.get_path_name()}")
    return malla


def materiales_caballero(malla, maestro):
    """
    Una instancia de M_SAEVUS_PersonajeDibujado por ranura de la malla, en orden de ranura.
    Se aplican como override del componente: Interchange rehace los materiales del asset al reimportar.
    """
    resultado = []
    for ranura in malla.get_editor_property("materials"):
        nombre_ranura = str(ranura.get_editor_property("material_slot_name"))
        clave = next((c for c in PIEZAS_CABALLERO if nombre_ranura.lower().startswith(c.lower())), None)
        if clave is None:
            raise RuntimeError(f"Ranura de material sin pieza conocida en el caballero: {nombre_ranura}")
        resultado.append(crear_instancia_material(f"MI_Caballero_{clave}", DIR_CABALLERO, maestro, PIEZAS_CABALLERO[clave]))
    registrar(f"materiales del caballero: {[m.get_name() for m in resultado]}")
    return resultado


# --- Estilo visual --------------------------------------------------------------

def nodo(material, clase, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(material, clase, x, y)


def parametro_escalar(material, nombre, valor, x, y):
    expresion = nodo(material, unreal.MaterialExpressionScalarParameter, x, y)
    expresion.set_editor_property("parameter_name", nombre)
    expresion.set_editor_property("default_value", valor)
    return expresion


def parametro_vector(material, nombre, color, x, y):
    expresion = nodo(material, unreal.MaterialExpressionVectorParameter, x, y)
    expresion.set_editor_property("parameter_name", nombre)
    expresion.set_editor_property("default_value", unreal.LinearColor(*color))
    return expresion


def material_vacio(nombre, directorio):
    ruta = f"{directorio}/{nombre}"
    if eal.does_asset_exist(ruta):
        material = cargar(ruta)
    else:
        material = herramientas_assets.create_asset(nombre, directorio, unreal.Material, unreal.MaterialFactoryNew())
        registrar(f"creado {ruta}")
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    return material


def crear_material_personaje():
    """
    M_SAEVUS_PersonajeDibujado: color plano sin brillo (rugosidad 1, poco especular), borde de luz
    fría por Fresnel y un poco de autoiluminación opcional. Barato: sin texturas, un Fresnel.
    """
    edicion = unreal.MaterialEditingLibrary
    material = material_vacio("M_SAEVUS_PersonajeDibujado", DIR_ESTILO)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    material.set_editor_property("used_with_skeletal_mesh", True)

    color = parametro_vector(material, "Color", (0.5, 0.5, 0.5, 1.0), -900, 0)
    edicion.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    edicion.connect_material_property(parametro_escalar(material, "Rugosidad", 1.0, -900, 150), "", unreal.MaterialProperty.MP_ROUGHNESS)
    # Casi sin especular: en un dibujo no hay brillos de luz.
    edicion.connect_material_property(parametro_escalar(material, "Especular", 0.04, -900, 250), "", unreal.MaterialProperty.MP_SPECULAR)

    fresnel = nodo(material, unreal.MaterialExpressionFresnel, -600, 400)
    edicion.connect_material_expressions(parametro_escalar(material, "ExponenteBorde", 3.0, -900, 400), "", fresnel, "ExponentIn")
    borde = nodo(material, unreal.MaterialExpressionMultiply, -400, 400)
    edicion.connect_material_expressions(fresnel, "", borde, "A")
    edicion.connect_material_expressions(parametro_vector(material, "ColorBorde", COLOR_BORDE_FRIO, -900, 500), "", borde, "B")
    borde_intensidad = nodo(material, unreal.MaterialExpressionMultiply, -250, 400)
    edicion.connect_material_expressions(borde, "", borde_intensidad, "A")
    edicion.connect_material_expressions(parametro_escalar(material, "IntensidadBorde", 0.4, -900, 650), "", borde_intensidad, "B")

    propio = nodo(material, unreal.MaterialExpressionMultiply, -400, 150)
    edicion.connect_material_expressions(color, "", propio, "A")
    edicion.connect_material_expressions(parametro_escalar(material, "AutoIluminacion", 0.0, -900, 750), "", propio, "B")
    emisivo = nodo(material, unreal.MaterialExpressionAdd, -100, 300)
    edicion.connect_material_expressions(propio, "", emisivo, "A")
    edicion.connect_material_expressions(borde_intensidad, "", emisivo, "B")
    edicion.connect_material_property(emisivo, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edicion.recompile_material(material)
    return material


def crear_instancia_material(nombre, directorio, padre, parametros):
    edicion = unreal.MaterialEditingLibrary
    instancia = obtener_o_crear(nombre, directorio, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    edicion.set_material_instance_parent(instancia, padre)
    for nombre_parametro, valor in parametros.items():
        if isinstance(valor, tuple):
            edicion.set_material_instance_vector_parameter_value(instancia, nombre_parametro, unreal.LinearColor(*valor))
        else:
            edicion.set_material_instance_scalar_parameter_value(instancia, nombre_parametro, valor)
    edicion.update_material_instance(instancia)
    return instancia


# Post-proceso dibujado (Hollow Knight en 3D), una sola pasada después del DOF (color lineal, antes
# del tonemapper), 10 lecturas de textura:
# - Bandas de luz: la luz recibida (color / albedo difuso) se cuantiza en bandas planas con bordes
#   suaves (en raíz cuadrada, más cerca de lo que ve el ojo); las bandas bajas se enfrían.
# - Tinta: laplaciano de la inversa de la profundidad (siluetas; una superficie plana da 0, así el
#   suelo no se raya) y cambios de normal (pliegues), a Grosor píxeles.
# - Niebla en capas: la distancia se reparte en capas planas que se aclaran hacia un azul gris, como
#   los fondos pintados; lo cercano queda en silueta oscura.
# - Grano de papel fijo en pantalla.
CODIGO_DIBUJADO = """
float d = max(Profundidad.r, 1.0);
float cerca = saturate(1.0 - d / DistanciaMaxima);
float3 c = Escena.rgb;
const float3 Luma = float3(0.299, 0.587, 0.114);

// Bandas de luz planas y sombra fría.
float3 albedo = SceneTextureFetch(PPI_DiffuseColor, float2(0, 0)).rgb;
float la = dot(albedo, Luma);
float luz = sqrt(max(dot(c, Luma), 0.0) / max(la, 0.03));
float escalon = luz * Bandas;
float banda = (floor(escalon) + smoothstep(0.5 - SuavidadBandas, 0.5 + SuavidadBandas, frac(escalon))) / Bandas;
float peso = saturate(la * 12.0) * cerca * IntensidadBandas;
float3 bandeado = c * (banda * banda) / max(luz * luz, 1e-4);
float frio = saturate(1.0 - banda * 2.0);
bandeado = lerp(bandeado, bandeado * ColorSombra.rgb, frio);
c = lerp(c, bandeado, peso);

// Niebla en capas.
float f = saturate((d - InicioNiebla) / RangoNiebla);
float capa = f * CapasNiebla;
float fq = (floor(capa) + smoothstep(0.3, 0.7, frac(capa))) / CapasNiebla;
c = lerp(c, ColorNiebla.rgb, fq * IntensidadNiebla);

// Tinta: siluetas por profundidad y pliegues por normales.
float il = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(-Grosor, 0)).r, 1.0);
float ir = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(Grosor, 0)).r, 1.0);
float iu = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(0, -Grosor)).r, 1.0);
float id = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(0, Grosor)).r, 1.0);
float laplaciano = (abs(il + ir - 2.0 / d) + abs(iu + id - 2.0 / d)) * d;
float silueta = saturate((laplaciano - Umbral) * Nitidez);
float3 n = SceneTextureFetch(PPI_WorldNormal, float2(0, 0)).xyz;
float dn = (1.0 - dot(n, SceneTextureFetch(PPI_WorldNormal, float2(Grosor, 0)).xyz))
         + (1.0 - dot(n, SceneTextureFetch(PPI_WorldNormal, float2(0, Grosor)).xyz));
float pliegue = saturate((dn - UmbralNormal) * NitidezNormal);
float tinta = max(silueta, pliegue * IntensidadPliegue) * cerca * (1.0 - fq * 0.7);
c = lerp(c, ColorTinta.rgb, tinta * IntensidadTinta);

// Grano de papel (fijo en pantalla, en celdas de 2 píxeles).
float2 celda = floor(Parameters.SvPosition.xy * 0.5);
float ruido = frac(sin(dot(celda, float2(12.9898, 78.233))) * 43758.5453);
c *= 1.0 + (ruido - 0.5) * Grano;
return c;
"""

# Valores de partida de M_PP_Dibujado (todos parámetros del material: se ajustan sin código).
PARAMETROS_DIBUJADO = (
    ("DistanciaMaxima", 7000.0), ("Bandas", 3.0), ("SuavidadBandas", 0.08), ("IntensidadBandas", 0.85),
    ("InicioNiebla", 900.0), ("RangoNiebla", 5000.0), ("CapasNiebla", 4.0), ("IntensidadNiebla", 0.55),
    ("Grosor", 2.0), ("Umbral", 0.06), ("Nitidez", 6.0), ("UmbralNormal", 0.35), ("NitidezNormal", 4.0),
    ("IntensidadPliegue", 0.7), ("IntensidadTinta", 0.9), ("Grano", 0.06),
)
COLORES_DIBUJADO = (
    ("ColorSombra", (0.55, 0.65, 0.95, 1.0)),
    ("ColorNiebla", (0.10, 0.13, 0.19, 1.0)),
    ("ColorTinta", (0.006, 0.006, 0.012, 1.0)),
)


def entrada_personalizada(nombre):
    entrada = unreal.CustomInput()
    entrada.set_editor_property("input_name", nombre)
    return entrada


def crear_material_dibujado():
    edicion = unreal.MaterialEditingLibrary
    material = material_vacio("M_PP_Dibujado", DIR_ESTILO)
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    material.set_editor_property("blendable_location", valor_enum(unreal.BlendableLocation, "BL_SCENE_COLOR_AFTER_DOF", "BL_AFTER_DOF"))

    personalizado = nodo(material, unreal.MaterialExpressionCustom, -300, 0)
    personalizado.set_editor_property("code", CODIGO_DIBUJADO)
    personalizado.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    personalizado.set_editor_property("description", "Dibujado")
    entradas = ["Escena", "Profundidad"] + [nombre for nombre, _ in PARAMETROS_DIBUJADO] + [nombre for nombre, _ in COLORES_DIBUJADO]
    personalizado.set_editor_property("inputs", [entrada_personalizada(nombre) for nombre in entradas])

    escena = nodo(material, unreal.MaterialExpressionSceneTexture, -800, -200)
    escena.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
    profundidad = nodo(material, unreal.MaterialExpressionSceneTexture, -800, -50)
    profundidad.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_SCENE_DEPTH)
    edicion.connect_material_expressions(escena, "Color", personalizado, "Escena")
    edicion.connect_material_expressions(profundidad, "Color", personalizado, "Profundidad")
    for indice, (nombre, valor) in enumerate(PARAMETROS_DIBUJADO):
        edicion.connect_material_expressions(parametro_escalar(material, nombre, valor, -800, 100 + indice * 100), "", personalizado, nombre)
    for indice, (nombre, color) in enumerate(COLORES_DIBUJADO):
        y = 100 + (len(PARAMETROS_DIBUJADO) + indice) * 100
        edicion.connect_material_expressions(parametro_vector(material, nombre, color, -800, y), "", personalizado, nombre)
    edicion.connect_material_property(personalizado, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edicion.recompile_material(material)
    return material


def borrar_materiales_obsoletos():
    """Los M_Caballero_* de color plano los sustituyen las instancias MI_Caballero_*; M_PP_Contorno, M_PP_Dibujado."""
    rutas = [f"{DIR_CABALLERO}/M_Caballero_{clave}" for clave in PIEZAS_CABALLERO] + [f"{DIR_ESTILO}/M_PP_Contorno"]
    for ruta in rutas:
        if eal.does_asset_exist(ruta) and eal.delete_asset(ruta):
            registrar(f"borrado {ruta}")


def crear_assets_entrada():
    acciones = {}
    for nombre in ASIGNACIONES_SAEVUS:
        accion = obtener_o_crear(nombre, f"{DIR_ENTRADA}/Acciones", unreal.InputAction, unreal.InputAction_Factory())
        accion.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
        # La pausa tiene que poder cerrarse estando en pausa.
        accion.set_editor_property("trigger_when_paused", nombre == "IA_Pausa")
        acciones[nombre] = accion

    imc = obtener_o_crear("IMC_SAEVUS", DIR_ENTRADA, unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    imc.unmap_all()
    for nombre, teclas in ASIGNACIONES_SAEVUS.items():
        for tecla in teclas:
            imc.map_key(acciones[nombre], crear_tecla(tecla))
    return acciones, imc


def crear_data_assets(acciones):
    def data_asset(nombre, clase_asset):
        fabrica = unreal.DataAssetFactory()
        fabrica.set_editor_property("data_asset_class", clase_asset)
        return obtener_o_crear(nombre, DIR_DATOS, clase_asset, fabrica)

    config_entrada = data_asset("DA_ConfigEntrada", unreal.SAEVUSConfigEntrada)
    config_entrada.set_editor_property("accion_mover", cargar(f"{ACCIONES_PLANTILLA}/IA_Move"))
    config_entrada.set_editor_property("accion_mirar", cargar(f"{ACCIONES_PLANTILLA}/IA_Look"))
    config_entrada.set_editor_property("accion_mirar_raton", cargar(f"{ACCIONES_PLANTILLA}/IA_MouseLook"))
    config_entrada.set_editor_property("accion_saltar", cargar(f"{ACCIONES_PLANTILLA}/IA_Jump"))
    config_entrada.set_editor_property("accion_esprint", acciones["IA_Esprintar"])
    config_entrada.set_editor_property("accion_caminar", acciones["IA_Caminar"])
    config_entrada.set_editor_property("accion_dash", acciones["IA_Dash"])
    config_entrada.set_editor_property("accion_atacar", acciones["IA_Atacar"])
    # Armas de fuego retiradas por ahora (decisión del dueño, 2026-10-04): sin recarga ni cambio de arma.
    config_entrada.set_editor_property("accion_recargar", None)
    config_entrada.set_editor_property("accion_curar", acciones["IA_Curar"])
    config_entrada.set_editor_property("accion_interactuar", acciones["IA_Interactuar"])
    config_entrada.set_editor_property("accion_guardia", acciones["IA_Guardia"])
    config_entrada.set_editor_property("accion_siguiente_arma", None)
    config_entrada.set_editor_property("acciones_ranura_arma", [])

    # Los valores se quedan en los del código hasta ajustarlos en el editor.
    movimiento = data_asset("DA_AjustesMovimiento_Base", unreal.SAEVUSAjustesMovimiento)
    camara = data_asset("DA_AjustesCamara_Base", unreal.SAEVUSAjustesCamara)
    ajuste_camara = camara.get_editor_property("camara")
    ajuste_camara.set_editor_property("material_dibujado", crear_material_dibujado())
    camara.set_editor_property("camara", ajuste_camara)
    repertorio = data_asset("DA_RepertorioMelee_Base", unreal.SAEVUSDatosRepertorioMelee)
    # Copia del repertorio del código (su constructor): mientras no se ajuste a mano, el asset sigue
    # al código y recibe los campos nuevos (p. ej. ventanas de guardia) con sus valores de diseño.
    repertorio.set_editor_property("repertorio", unreal.SAEVUSRepertorioMelee())
    armas = [crear_arma(data_asset, nombre, datos, repertorio) for nombre, datos in ARMAS.items()]
    return config_entrada, movimiento, camara, repertorio, armas


def crear_arma(data_asset, nombre, datos, repertorio):
    arma = data_asset(nombre, unreal.SAEVUSDefinicionArma)
    arma.set_editor_property("nombre_visible", datos["nombre_visible"])
    arma.set_editor_property("tiempo_equipar", datos["tiempo_equipar"])
    if datos["tipo"] == "cuerpo_a_cuerpo":
        arma.set_editor_property("tipo", valor_enum(unreal.SAEVUSTipoArma, "CUERPO_A_CUERPO", "CUERPO_ACUERPO"))
        arma.set_editor_property("repertorio", repertorio)
        return arma
    arma.set_editor_property("tipo", valor_enum(unreal.SAEVUSTipoArma, "ARMA_FUEGO"))
    arma_fuego = arma.get_editor_property("arma_fuego")
    for campo, valor in datos["arma_fuego"].items():
        arma_fuego.set_editor_property(campo, valor)
    arma.set_editor_property("arma_fuego", arma_fuego)
    return arma


def crear_blueprints(acciones, imc, config_entrada, movimiento, camara, repertorio, armas):
    bp_personaje = obtener_o_crear_blueprint("BP_SAEVUSPersonaje", unreal.SAEVUSPersonaje)
    personaje = cdo_blueprint(bp_personaje)
    personaje.set_editor_property("config_entrada", config_entrada)
    # El maniquí de plantilla ya no se usa: el cuerpo visible es el caballero en la malla poseable.
    malla = personaje.get_editor_property("mesh")
    if hasattr(malla, "set_skeletal_mesh_asset"):
        malla.set_skeletal_mesh_asset(None)
    malla.set_editor_property("anim_class", None)
    cuerpo = personaje.get_editor_property("cuerpo")
    caballero = importar_caballero()
    try:
        cuerpo.set_editor_property("skinned_asset", caballero)
    except Exception:  # noqa: BLE001 - según versión, la propiedad solo se cambia con su setter
        cuerpo.set_skinned_asset_and_update(caballero, True)
    cuerpo.set_editor_property("override_materials", materiales_caballero(caballero, crear_material_personaje()))
    borrar_materiales_obsoletos()
    personaje.get_editor_property("character_movement").set_editor_property("ajustes_movimiento", movimiento)
    personaje.get_editor_property("camara").set_editor_property("ajustes_camara", camara)
    personaje.get_editor_property("combate").set_editor_property("datos_repertorio", repertorio)
    espadas = [arma for arma in armas if arma.get_editor_property("tipo") == valor_enum(unreal.SAEVUSTipoArma, "CUERPO_A_CUERPO", "CUERPO_ACUERPO")]
    personaje.get_editor_property("armas").set_editor_property("arsenal", espadas)
    # Polvo claro y frío para la estela del dash: resalta sobre el escenario oscuro.
    material_estela = crear_material_color("M_ParticulaPolvo", (0.75, 0.78, 0.85, 1.0), sin_luz=True)
    personaje.get_editor_property("estela_dash").set_editor_property("override_materials", [material_estela])
    # Alma: blanco azulado, el mismo tono frío que el borde del caballero.
    material_alma = crear_material_color("M_ParticulaAlma", (0.7, 0.85, 1.0, 1.0), sin_luz=True)
    personaje.get_editor_property("efectos_combate").set_editor_property("override_materials", [material_alma])

    bp_controlador = obtener_o_crear_blueprint("BP_SAEVUSControladorJugador", unreal.SAEVUSControladorJugador)
    controlador = cdo_blueprint(bp_controlador)
    controlador.set_editor_property("contextos_entrada", [cargar(IMC_PLANTILLA), cargar(IMC_PLANTILLA_RATON), imc])
    controlador.set_editor_property("accion_alternar_hud", acciones["IA_AlternarHUD"])
    controlador.set_editor_property("accion_pausa", acciones["IA_Pausa"])

    bp_modo_juego = obtener_o_crear_blueprint("BP_SAEVUSModoJuego", unreal.SAEVUSModoJuego)
    modo_juego = cdo_blueprint(bp_modo_juego)
    modo_juego.set_editor_property("default_pawn_class", bp_personaje.generated_class())
    modo_juego.set_editor_property("player_controller_class", bp_controlador.generated_class())

    for blueprint in (bp_personaje, bp_controlador, bp_modo_juego):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return bp_modo_juego


# --- Enemigos ---------------------------------------------------------------------
# Tipos de enemigo (FSAEVUSAjusteEnemigo). Valores de primera pasada; se ajustan en los Data Assets.
# Ataques: tipo, distancias, tiempos de aviso/duración/recuperación, daño, velocidades, esfera de golpe.
ENEMIGOS = {
    "DA_Enemigo_Rastrero": {
        "nombre_visible": "Rastrero", "vida_maxima": 30.0, "radio_capsula": 40.0, "media_altura_capsula": 40.0,
        "color_cuerpo": (0.05, 0.03, 0.03), "color_mascara": (0.9, 0.45, 0.1),
        "velocidad_movimiento": 280.0, "radio_deteccion": 900.0, "radio_perdida": 1800.0, "radio_patrulla": 250.0,
        "danio_contacto": 10.0,
        "ataques": [
            {"nombre": "Embestida", "tipo": "EMBESTIDA", "distancia_minima": 0.0, "distancia_maxima": 450.0,
             "tiempo_telegrafiar": 0.55, "duracion": 0.4, "tiempo_recuperacion": 0.8, "danio": 15.0, "velocidad": 950.0},
        ],
    },
    "DA_Enemigo_Saltador": {
        "nombre_visible": "Saltador", "vida_maxima": 25.0, "radio_capsula": 35.0, "media_altura_capsula": 50.0,
        "color_cuerpo": (0.04, 0.05, 0.03), "color_mascara": (0.65, 0.85, 0.3),
        "velocidad_movimiento": 220.0, "radio_deteccion": 1100.0, "radio_perdida": 2000.0, "radio_patrulla": 150.0,
        "danio_contacto": 10.0,
        "ataques": [
            {"nombre": "Salto", "tipo": "SALTO", "distancia_minima": 150.0, "distancia_maxima": 700.0,
             "tiempo_telegrafiar": 0.45, "duracion": 1.5, "tiempo_recuperacion": 0.7, "danio": 15.0,
             "velocidad": 500.0, "velocidad_vertical": 750.0},
        ],
    },
    "DA_Enemigo_Escupidor": {
        "nombre_visible": "Escupidor", "vida_maxima": 20.0, "radio_capsula": 38.0, "media_altura_capsula": 55.0,
        "color_cuerpo": (0.05, 0.03, 0.06), "color_mascara": (0.7, 0.4, 0.95),
        "velocidad_movimiento": 200.0, "radio_deteccion": 1300.0, "radio_perdida": 2200.0, "radio_patrulla": 120.0,
        "danio_contacto": 10.0, "distancia_retirada": 400.0,
        "ataques": [
            {"nombre": "Escupitajo", "tipo": "PROYECTIL", "distancia_minima": 400.0, "distancia_maxima": 1200.0,
             "tiempo_telegrafiar": 0.6, "duracion": 0.3, "tiempo_recuperacion": 1.1, "danio": 15.0, "velocidad": 900.0},
        ],
    },
    "DA_Enemigo_Revoloteador": {
        # Volador (Hollow Knight): flota sobre el jugador y cae en picado tras avisar; se le da con el tajo hacia
        # arriba al saltar o rebotando encima. Frágil.
        "nombre_visible": "Revoloteador", "vida_maxima": 15.0, "radio_capsula": 35.0, "media_altura_capsula": 35.0,
        "color_cuerpo": (0.03, 0.04, 0.06), "color_mascara": (0.4, 0.8, 1.0),
        "velocidad_movimiento": 260.0, "radio_deteccion": 1000.0, "radio_perdida": 1800.0, "radio_patrulla": 200.0,
        "danio_contacto": 10.0, "volador": True, "altura_vuelo": 220.0,
        "ataques": [
            {"nombre": "Picado", "tipo": "PICADO", "distancia_minima": 0.0, "distancia_maxima": 450.0,
             "tiempo_telegrafiar": 0.6, "duracion": 0.55, "tiempo_recuperacion": 0.9, "danio": 15.0, "velocidad": 950.0},
        ],
    },
    "DA_Jefe_Guardian": {
        "nombre_visible": "Guardián", "vida_maxima": 250.0, "es_jefe": True, "radio_capsula": 90.0, "media_altura_capsula": 120.0,
        "color_cuerpo": (0.06, 0.05, 0.07), "color_mascara": (1.0, 0.55, 0.15),
        "velocidad_movimiento": 260.0, "radio_deteccion": 1500.0, "radio_perdida": 4000.0, "radio_patrulla": 0.0,
        "danio_contacto": 15.0, "escala_empuje_recibido": 0.15, "duracion_muerte": 1.5,
        "ataques": [
            {"nombre": "Golpe", "tipo": "GOLPE", "distancia_minima": 0.0, "distancia_maxima": 260.0,
             "tiempo_telegrafiar": 0.6, "duracion": 0.25, "tiempo_recuperacion": 0.7, "danio": 25.0,
             "radio_golpe": 160.0, "alcance_golpe": 150.0},
            {"nombre": "Embestida", "tipo": "EMBESTIDA", "distancia_minima": 300.0, "distancia_maxima": 1200.0,
             "tiempo_telegrafiar": 0.7, "duracion": 0.7, "tiempo_recuperacion": 1.0, "danio": 25.0, "velocidad": 1300.0},
            {"nombre": "Salto", "tipo": "SALTO", "distancia_minima": 200.0, "distancia_maxima": 900.0,
             "tiempo_telegrafiar": 0.6, "duracion": 2.0, "tiempo_recuperacion": 0.9, "danio": 30.0,
             "velocidad": 600.0, "velocidad_vertical": 900.0},
        ],
        # Fase 2 a media vida: más rápido, avisos más cortos y añade el salto.
        "fases": [
            {"fraccion_vida": 1.0, "escala_velocidad": 1.0, "escala_tiempos": 1.0, "ataques": [0, 1]},
            {"fraccion_vida": 0.5, "escala_velocidad": 1.3, "escala_tiempos": 0.7, "ataques": [0, 1, 2]},
        ],
    },
}
ENEMIGOS["DA_Enemigo_Acorazado"] = {
    # Con escudo: bloquea de frente; se rodea, se salta por encima (pogo) o se rompe con el giro cargado.
    "nombre_visible": "Acorazado", "vida_maxima": 40.0, "radio_capsula": 45.0, "media_altura_capsula": 60.0,
    "color_cuerpo": (0.05, 0.045, 0.04), "color_mascara": (0.75, 0.6, 0.35),
    "velocidad_movimiento": 200.0, "radio_deteccion": 900.0, "radio_perdida": 1800.0, "radio_patrulla": 150.0,
    "danio_contacto": 10.0, "angulo_escudo": 120.0, "danio_rompe_escudo": 28.0, "tiempo_escudo_roto": 2.5,
    "ataques": [
        {"nombre": "Escudazo", "tipo": "GOLPE", "distancia_minima": 0.0, "distancia_maxima": 220.0,
         "tiempo_telegrafiar": 0.6, "duracion": 0.25, "tiempo_recuperacion": 1.0, "danio": 20.0,
         "radio_golpe": 140.0, "alcance_golpe": 120.0},
    ],
}
ENEMIGOS["DA_Jefe_MadreAlada"] = {
    # Jefe de la Zona 1: volador grande. Picado y abanico de proyectiles; a media vida, más rápido y un abanico más amplio.
    "nombre_visible": "Madre Alada", "vida_maxima": 300.0, "es_jefe": True, "radio_capsula": 70.0, "media_altura_capsula": 70.0,
    "color_cuerpo": (0.03, 0.03, 0.06), "color_mascara": (0.55, 0.85, 1.0),
    "velocidad_movimiento": 300.0, "radio_deteccion": 1600.0, "radio_perdida": 4000.0, "radio_patrulla": 0.0,
    "danio_contacto": 15.0, "escala_empuje_recibido": 0.2, "duracion_muerte": 1.5, "volador": True, "altura_vuelo": 320.0,
    "ataques": [
        {"nombre": "Picado", "tipo": "PICADO", "distancia_minima": 0.0, "distancia_maxima": 700.0,
         "tiempo_telegrafiar": 0.75, "duracion": 0.8, "tiempo_recuperacion": 1.1, "danio": 25.0, "velocidad": 1100.0},
        {"nombre": "Abanico", "tipo": "PROYECTIL", "distancia_minima": 300.0, "distancia_maxima": 1600.0,
         "tiempo_telegrafiar": 0.8, "duracion": 0.4, "tiempo_recuperacion": 1.0, "danio": 20.0, "velocidad": 750.0,
         "numero_proyectiles": 5, "apertura_abanico": 60.0},
        {"nombre": "Abanico amplio", "tipo": "PROYECTIL", "distancia_minima": 0.0, "distancia_maxima": 1600.0,
         "tiempo_telegrafiar": 0.6, "duracion": 0.3, "tiempo_recuperacion": 0.9, "danio": 20.0, "velocidad": 950.0,
         "numero_proyectiles": 7, "apertura_abanico": 90.0},
    ],
    "fases": [
        {"fraccion_vida": 1.0, "escala_velocidad": 1.0, "escala_tiempos": 1.0, "ataques": [0, 1]},
        {"fraccion_vida": 0.5, "escala_velocidad": 1.25, "escala_tiempos": 0.75, "ataques": [0, 1, 2]},
    ],
}
DIR_ENEMIGOS = f"{RAIZ}/Datos/Enemigos"
DIR_ARTE_ENEMIGOS = f"{RAIZ}/Arte/Enemigos"
# Cuerpo generado en Blender por tipo (Herramientas/Arte/Generar-Enemigos.ps1 → ArteFuente/Enemigos).
CUERPOS_ENEMIGOS = {
    "DA_Enemigo_Rastrero": "SM_Enemigo_Rastrero",
    "DA_Enemigo_Saltador": "SM_Enemigo_Saltador",
    "DA_Enemigo_Escupidor": "SM_Enemigo_Escupidor",
    "DA_Enemigo_Revoloteador": "SM_Enemigo_Revoloteador",
    "DA_Enemigo_Acorazado": "SM_Enemigo_Acorazado",
    "DA_Jefe_Guardian": "SM_Jefe_Guardian",
    "DA_Jefe_MadreAlada": "SM_Jefe_MadreAlada",
}


def importar_estatica(nombre, carpeta_fuente, destino):
    fbx = os.path.join(unreal.Paths.project_dir(), "ArteFuente", carpeta_fuente, f"{nombre}.fbx")
    if not os.path.exists(fbx):
        raise RuntimeError(f"Falta {fbx}; genera el arte con Herramientas/Arte/")
    tarea = unreal.AssetImportTask()
    tarea.filename = fbx
    tarea.destination_path = destino
    tarea.automated = True
    tarea.replace_existing = True
    tarea.save = False
    herramientas_assets.import_asset_tasks([tarea])
    malla = next((m for m in (unreal.load_asset(r) for r in tarea.imported_object_paths) if isinstance(m, unreal.StaticMesh)), None)
    if malla is None:
        raise RuntimeError(f"La importación de {fbx} no produjo una malla estática")
    return malla


def crear_datos_enemigos(material_base, material_particulas):
    resultado = {}
    for nombre, valores in ENEMIGOS.items():
        fabrica = unreal.DataAssetFactory()
        fabrica.set_editor_property("data_asset_class", unreal.SAEVUSDatosEnemigo)
        datos = obtener_o_crear(nombre, DIR_ENEMIGOS, unreal.SAEVUSDatosEnemigo, fabrica)
        ajuste = datos.get_editor_property("enemigo")
        for campo, valor in valores.items():
            if campo == "ataques":
                ataques = []
                for definicion in valor:
                    ataque = unreal.SAEVUSAtaqueEnemigo()
                    for campo_ataque, valor_ataque in definicion.items():
                        if campo_ataque == "tipo":
                            valor_ataque = valor_enum(unreal.SAEVUSTipoAtaqueEnemigo, valor_ataque)
                        ataque.set_editor_property(campo_ataque, valor_ataque)
                    ataques.append(ataque)
                ajuste.set_editor_property("ataques", ataques)
            elif campo == "fases":
                fases = []
                for definicion in valor:
                    fase = unreal.SAEVUSFaseEnemigo()
                    for campo_fase, valor_fase in definicion.items():
                        fase.set_editor_property(campo_fase, valor_fase)
                    fases.append(fase)
                ajuste.set_editor_property("fases", fases)
            elif campo.startswith("color_"):
                ajuste.set_editor_property(campo, unreal.LinearColor(*valor, 1.0))
            elif campo == "nombre_visible":
                ajuste.set_editor_property(campo, unreal.Text(valor))
            elif campo == "es_jefe":
                ajuste.set_editor_property("es_jefe", valor)
            else:
                ajuste.set_editor_property(campo, valor)
        datos.set_editor_property("enemigo", ajuste)
        datos.set_editor_property("material_base", material_base)
        datos.set_editor_property("material_particulas", material_particulas)
        if nombre in CUERPOS_ENEMIGOS:
            datos.set_editor_property("malla_cuerpo", importar_estatica(CUERPOS_ENEMIGOS[nombre], "Enemigos", DIR_ARTE_ENEMIGOS))
        resultado[nombre] = datos
    return resultado


# Decisión del dueño (2026-10-04): sin enemigos ni jefes en los mapas hasta los goblins de GunZ.
ENEMIGOS_EN_MAPAS = False


def crear_enemigo(actores, etiqueta, datos, posicion):
    """De pie en el suelo (posicion.z = altura del suelo); los voladores, a su altura de vuelo sobre él.
    Con ENEMIGOS_EN_MAPAS apagado no crea nada y devuelve None."""
    if not ENEMIGOS_EN_MAPAS:
        return None
    ajuste = datos.get_editor_property("enemigo")
    altura = ajuste.get_editor_property("media_altura_capsula")
    if ajuste.get_editor_property("volador"):
        altura += ajuste.get_editor_property("altura_vuelo")
    enemigo = actores.spawn_actor_from_class(unreal.SAEVUSEnemigo, unreal.Vector(posicion.x, posicion.y, posicion.z + altura + 5.0), unreal.Rotator(0, 0, 180))
    enemigo.set_actor_label(etiqueta)
    enemigo.set_editor_property("datos", datos)
    enemigo.set_folder_path("Enemigos")
    return enemigo


# --- Mapa de pruebas -----------------------------------------------------------

def crear_bloque(actores, etiqueta, posicion, tamano_cm, material, rotacion=unreal.Rotator(0, 0, 0)):
    """Caja de tamano_cm (x, y, z) centrada en posicion, hecha con el cubo de 100 cm del motor."""
    actor = actores.spawn_actor_from_object(cargar(MALLA_CUBO), posicion, rotacion)
    actor.set_actor_label(etiqueta)
    actor.set_actor_scale3d(unreal.Vector(tamano_cm[0] / 100.0, tamano_cm[1] / 100.0, tamano_cm[2] / 100.0))
    actor.static_mesh_component.set_material(0, material)
    actor.set_folder_path("Geometria")
    return actor


def crear_muneco(actores, etiqueta, posicion, rotacion=unreal.Rotator(0, 0, 0)):
    muneco = actores.spawn_actor_from_class(unreal.SAEVUSMunecoEntrenamiento, posicion, rotacion)
    muneco.set_actor_label(etiqueta)
    muneco.set_folder_path("Combate")
    return muneco


def crear_iluminacion(actores):
    """Iluminación totalmente dinámica, sin Lumen."""
    sol = actores.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-50, yaw=-35))
    sol.set_actor_label("Sol")
    luz_sol = sol.get_component_by_class(unreal.DirectionalLightComponent)
    luz_sol.set_editor_property("atmosphere_sun_light", True)
    luz_sol.set_mobility(unreal.ComponentMobility.MOVABLE)

    luz_cielo = actores.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1200))
    luz_cielo.set_actor_label("LuzCielo")
    componente_cielo = luz_cielo.get_component_by_class(unreal.SkyLightComponent)
    componente_cielo.set_mobility(unreal.ComponentMobility.MOVABLE)
    componente_cielo.set_editor_property("real_time_capture", True)

    actores.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0)).set_actor_label("Atmosfera")
    actores.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0)).set_actor_label("Niebla")
    for etiqueta in ("Sol", "LuzCielo", "Atmosfera", "Niebla"):
        for actor in actores.get_all_level_actors():
            if actor.get_actor_label() == etiqueta:
                actor.set_folder_path("Iluminacion")


def abrir_mapa_limpio(ruta):
    editor_niveles = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actores = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if eal.does_asset_exist(ruta):
        editor_niveles.load_level(ruta)
        for actor in actores.get_all_level_actors():
            if not isinstance(actor, unreal.WorldSettings):
                actores.destroy_actor(actor)
    else:
        editor_niveles.new_level(ruta)
    return editor_niveles, actores


def construir_mapa_pruebas(bp_modo_juego, datos_enemigos):
    editor_niveles, actores = abrir_mapa_limpio(RUTA_MAPA_PRUEBAS)

    rejilla = cargar(MATERIAL_REJILLA)
    oscuro = cargar(MATERIAL_REJILLA_OSCURA)

    # Suelo: 100 m x 100 m, superficie en Z = 0.
    crear_bloque(actores, "Suelo", unreal.Vector(0, 0, -50), (10000, 10000, 100), rejilla)

    # Escalones de altura de salto: de 50 cm a 250 cm.
    for indice, altura in enumerate((50, 100, 150, 200, 250)):
        crear_bloque(actores, f"Escalon_{altura}cm", unreal.Vector(800, -1000 + indice * 400, altura / 2.0), (200, 200, altura), oscuro)

    # Rampas a ambos lados del límite caminable por defecto (~44,8 grados).
    for indice, angulo in enumerate((20, 35, 50)):
        crear_bloque(
            actores, f"Rampa_{angulo}grados", unreal.Vector(-1200, -1200 + indice * 700, 0), (800, 400, 40), oscuro,
            unreal.Rotator(roll=0, pitch=angulo, yaw=0),
        )

    # Saltos de hueco desde plataformas de 200 cm: huecos de 2, 3, 4, 5 y 6 m.
    altura_plataforma = 200
    x = 2000
    for hueco in (0, 200, 300, 400, 500, 600):
        x += hueco
        crear_bloque(actores, f"Plataforma_hueco{hueco}cm", unreal.Vector(x + 200, 1500, altura_plataforma / 2.0), (400, 400, altura_plataforma), oscuro)
        x += 400

    # Pared larga y alta para la carrera por pared.
    crear_bloque(actores, "Pared_Larga", unreal.Vector(0, 3000, 300), (3000, 100, 600), oscuro)

    # Chimenea de salto de pared: dos paredes de 7 m con 3 m de hueco, se sube alternando saltos de pared,
    # y una cornisa de 4,5 m al fondo para terminar.
    for lado, y in (("I", -200), ("D", 200)):
        crear_bloque(actores, f"SaltoPared_Chimenea{lado}", unreal.Vector(-3000, y, 350), (600, 100, 700), oscuro)
    crear_bloque(actores, "SaltoPared_Cornisa450cm", unreal.Vector(-3500, 0, 225), (400, 400, 450), rejilla)

    # Pasillo de carrera por pared: dos paredes de 20 m x 6 m, a 5 m, para encadenar carreras de lado a lado.
    for lado, y in (("A", -3300), ("B", -2700)):
        crear_bloque(actores, f"CarreraPared_Pasillo{lado}", unreal.Vector(0, y, 300), (2000, 100, 600), oscuro)

    # Escalada: un bloque de 3,5 m al que subirse, y una pared de 10 m que acaba cada escalada en voltereta.
    crear_bloque(actores, "Escalada_Bloque350cm", unreal.Vector(3000, -1500, 175), (400, 800, 350), rejilla)
    crear_bloque(actores, "Escalada_Alta", unreal.Vector(3000, -2600, 500), (100, 800, 1000), oscuro)

    # Muñecos cerca del inicio: uno solo, y dos juntos para comprobar golpes a un solo objetivo.
    crear_muneco(actores, "Muneco_A", unreal.Vector(600, -400, 96), unreal.Rotator(0, 0, 180))
    crear_muneco(actores, "Muneco_B1", unreal.Vector(600, 300, 96), unreal.Rotator(0, 0, 180))
    crear_muneco(actores, "Muneco_B2", unreal.Vector(600, 420, 96), unreal.Rotator(0, 0, 180))

    # Galería de tiro: muñecos a 10, 20 y 30 m del inicio, a un lado (+Y).
    for distancia in (1000, 2000, 3000):
        crear_muneco(actores, f"Muneco_Tiro{distancia // 100}m", unreal.Vector(-distancia, 1000, 96))

    # Zona de enemigos, lejos de la ruta de la prueba de humo (que va hacia +X cerca de Y = 0).
    crear_enemigo(actores, "Enemigo_Rastrero", datos_enemigos["DA_Enemigo_Rastrero"], unreal.Vector(-1500, -4300, 0))
    crear_enemigo(actores, "Enemigo_Saltador", datos_enemigos["DA_Enemigo_Saltador"], unreal.Vector(0, -4300, 0))
    crear_enemigo(actores, "Enemigo_Revoloteador", datos_enemigos["DA_Enemigo_Revoloteador"], unreal.Vector(1000, -4300, 0))
    crear_enemigo(actores, "Jefe_Guardian", datos_enemigos["DA_Jefe_Guardian"], unreal.Vector(2000, -4300, 0))

    crear_iluminacion(actores)
    actores.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100)).set_actor_label("Inicio")

    mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mundo.get_world_settings().set_editor_property("default_game_mode", bp_modo_juego.generated_class())
    editor_niveles.save_current_level()


# --- Mapa tutorial ------------------------------------------------------------------
# Recorrido lineal hacia +X: cada sección enseña una mecánica con un aviso en pantalla y un cartel.
# Medidas pensadas para las del personaje (movimiento.md): salto ≈ 143 cm de alto y 450 cm de largo
# corriendo, doble salto ≈ +120 cm, dash ≈ 290 cm. Iluminación oscura (tono Hollow Knight).
RUTA_MAPA_TUTORIAL = f"{RAIZ}/Mapas/L_Tutorial"
ANCHO_PASILLO = 800.0
GROSOR_SUELO = 100.0
FONDO_FOSO = -700.0

# Paleta del escenario: piedra azul casi negra, acentos un poco más claros, puertas con brillo cálido.
PIEZAS_ESCENARIO = {
    "Piedra": {"Color": (0.035, 0.04, 0.055, 1.0), "ColorBorde": (0.3, 0.4, 0.6, 1.0), "IntensidadBorde": 0.08, "AutoIluminacion": 0.0},
    "Acento": {"Color": (0.09, 0.1, 0.13, 1.0), "ColorBorde": (0.4, 0.5, 0.7, 1.0), "IntensidadBorde": 0.12, "AutoIluminacion": 0.0},
    "Puerta": {"Color": (0.25, 0.12, 0.05, 1.0), "ColorBorde": (1.0, 0.6, 0.3, 1.0), "IntensidadBorde": 0.3, "AutoIluminacion": 0.6},
}

# Avisos por sala: (x inicio del volumen, título, texto). El cartel 3D repite el título. Cada aviso dice
# qué pulsar y qué hacer con ello en esa sala, en frases cortas.
AVISOS_TUTORIAL = [
    (100, "Moverse", "[W] [A] [S] [D]  moverte en ocho direcciones. Tu cuerpo mira siempre hacia la cámara.\n"
                     "[Ratón]  mirar.     Mantén [Mayús]  esprintar.     [Alt]  caminar."),
    (1450, "Saltar", "[Espacio]  saltar. Sube al escalón.\n"
                     "En el aire, [Espacio] otra vez: doble salto. Así llegas al bloque alto."),
    (3650, "Dash", "Toca dos veces seguidas una dirección ([W] [W], [A] [A], [S] [S] o [D] [D]): dash.\n"
                   "Los dashes se encadenan sin espera, también en el aire.\n"
                   "Cruza el foso: corre, salta, doble salto y dash hacia delante."),
    (5850, "Paredes", "Salta hacia una pared y pulsa [Espacio] al tocarla: rebotas en ella.\n"
                      "Entre las dos paredes del fondo, rebota de una a otra para subir.\n"
                      "Corre de frente contra la cornisa y salta: la escalas."),
    (7650, "Carrera por pared", "Corre junto a la pared de la derecha y salta hacia ella en diagonal.\n"
                                "Mantén [W]: corres por la pared. [Espacio]: saltas desde ella.\n"
                                "Cruza así el foso."),
    (9850, "Combate", "[Clic izq.]  tajo. Tres seguidos: combo de tres tajos.\n"
                      "Mira hacia arriba al atacar: tajo hacia arriba (para el muñeco alto).\n"
                      "Mantén [Clic der.]  guardia: bloquea los golpes que llegan de frente."),
    (10950, "Técnicas", "Mantén [Clic izq.] y suelta: ataque cargado en giro.\n"
                        "Ataca durante un dash: tajo en carrera.   En el aire: hasta tres tajos.\n"
                        "Un tajo se corta con dash, salto o guardia: encadénalos."),
    (12650, "Alma", "Cada golpe llena el alma (orbe arriba a la izquierda).\n"
                    "Mantén [Q] en el suelo para gastarla y curarte.\n"
                    "Siéntate en el banco con [E]: te cura del todo y guarda tu avance."),
    (14450, "Gran salón", "Sala libre: sube por las escaleras a los balcones,\n"
                          "corre por las paredes y salta entre columnas.\n"
                          "Todo se combina con todo: practica antes de seguir."),
    (20450, "Reto (opcional)", "Foso con pinchos, carrera por pared y cornisa, sin descanso.\n"
                               "Arriba espera un fragmento de máscara."),
]


def crear_texto(actores, etiqueta, texto, posicion, tamano=60.0, color=unreal.Color(r=200, g=215, b=255, a=255)):
    """Cartel de texto 3D mirando hacia -X (hacia el jugador que avanza)."""
    actor = actores.spawn_actor_from_class(unreal.TextRenderActor, posicion, unreal.Rotator(0, 0, 180))
    actor.set_actor_label(etiqueta)
    componente = actor.text_render
    componente.set_editor_property("text", unreal.Text(texto))
    componente.set_editor_property("world_size", tamano)
    componente.set_editor_property("horizontal_alignment", unreal.HorizTextAligment.EHTA_CENTER)
    componente.set_editor_property("text_render_color", color)
    actor.set_folder_path("Carteles")
    return actor


def crear_volumen(actores, clase, etiqueta, centro, extension, carpeta):
    volumen = actores.spawn_actor_from_class(clase, centro)
    volumen.set_actor_label(etiqueta)
    volumen.get_component_by_class(unreal.BoxComponent).set_box_extent(unreal.Vector(*extension))
    volumen.set_folder_path(carpeta)
    return volumen


def crear_aviso(actores, x, titulo, texto, altura=0.0, ancho=ANCHO_PASILLO):
    """Volumen que publica el aviso al cruzarlo (de pared a pared) y cartel 3D con el título."""
    volumen = crear_volumen(actores, unreal.SAEVUSVolumenTutorial, f"Aviso_{titulo}", unreal.Vector(x + 150, 0, altura + 150),
                            (150, ancho / 2, 300), "Tutorial")
    volumen.set_editor_property("titulo", unreal.Text(titulo))
    volumen.set_editor_property("texto", unreal.Text(texto))
    crear_texto(actores, f"Cartel_{titulo}", titulo, unreal.Vector(x + 300, -ancho / 2 + 40, altura + 260))


def crear_punto_control(actores, etiqueta, x, y=0.0, z=0.0, ancho=ANCHO_PASILLO):
    volumen = crear_volumen(actores, unreal.SAEVUSPuntoControl, etiqueta, unreal.Vector(x, y, z + 100), (100, ancho / 2, 200), "Tutorial")
    return volumen


def crear_foso(actores, etiqueta, x_inicio, x_fin, retorno, materiales, altura=0.0):
    """Hueco entre x_inicio y x_fin con fondo y volumen de caída que devuelve a retorno."""
    largo = x_fin - x_inicio
    centro_x = (x_inicio + x_fin) / 2.0
    fondo = crear_bloque(actores, f"{etiqueta}_Fondo", unreal.Vector(centro_x, 0, altura + FONDO_FOSO - 50), (largo, ANCHO_PASILLO * 2, 100), materiales["Piedra"])
    # El fondo no sale en el mapa de la pausa (SAEVUSMapa::EtiquetaSinMapa): el foso se lee como hueco.
    fondo.set_editor_property("tags", [unreal.Name("SinMapa")])
    caida = crear_volumen(actores, unreal.SAEVUSVolumenCaida, f"{etiqueta}_Caida", unreal.Vector(centro_x, 0, altura + FONDO_FOSO + 150),
                          (largo / 2, ANCHO_PASILLO, 150), "Tutorial")
    caida.set_editor_property("punto_retorno", retorno - caida.get_actor_location())
    return caida


def crear_peligro(actores, etiqueta, centro, extension, retorno, material):
    """Pinchos: tocarlos quita vida y devuelve a retorno; el tajo hacia abajo rebota en ellos."""
    peligro = crear_volumen(actores, unreal.SAEVUSPeligro, etiqueta, centro, extension, "Tutorial")
    peligro.set_editor_property("punto_retorno", retorno - peligro.get_actor_location())
    peligro.get_component_by_class(unreal.InstancedStaticMeshComponent).set_material(0, material)
    return peligro


# Piezas modulares del escenario (Herramientas/Arte/Generar-Escenario.ps1 → ArteFuente/Escenario).
DIR_ESCENARIO = f"{RAIZ}/Arte/Escenario"
PIEZAS_DECORADO = ("SM_ColumnaRota", "SM_ArcoSeccion", "SM_Escombros")


def importar_decorado(material):
    """Importa las piezas modulares y les pone el material de escenario. Devuelve {nombre: malla}."""
    mallas = {}
    for nombre in PIEZAS_DECORADO:
        fbx = os.path.join(unreal.Paths.project_dir(), "ArteFuente", "Escenario", f"{nombre}.fbx")
        if not os.path.exists(fbx):
            raise RuntimeError(f"Falta {fbx}; ejecuta Herramientas/Arte/Generar-Escenario.ps1")
        tarea = unreal.AssetImportTask()
        tarea.filename = fbx
        tarea.destination_path = DIR_ESCENARIO
        tarea.automated = True
        tarea.replace_existing = True
        tarea.save = False
        herramientas_assets.import_asset_tasks([tarea])
        malla = next((m for m in (unreal.load_asset(r) for r in tarea.imported_object_paths) if isinstance(m, unreal.StaticMesh)), None)
        if malla is None:
            raise RuntimeError(f"La importación de {fbx} no produjo una malla estática")
        malla.set_material(0, material)
        mallas[nombre] = malla
    registrar(f"decorado importado: {list(mallas)}")
    return mallas


def crear_decorado(actores, etiqueta, malla, posicion, guinada=0.0, escala=1.0):
    """Pieza de decorado sin colisión: no cambia las medidas probadas del recorrido."""
    actor = actores.spawn_actor_from_object(malla, posicion, unreal.Rotator(0, 0, guinada))
    actor.set_actor_label(etiqueta)
    actor.set_actor_scale3d(unreal.Vector(escala, escala, escala))
    actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_folder_path("Decorado")
    return actor


# --- Piezas de mazmorra (referencia de estructura: el mapa Dungeon de GunZ) --------------------
# Salas cerradas de muros altos sin techo (la luna entra y la cámara no choca con un techo), unidas por
# puertas; dentro, escaleras, balcones y columnas para correr por las paredes. Sin texturas.
FONDO_MUROS = FONDO_FOSO - 100.0
ANCHO_PUERTA = 500.0
ALTO_PUERTA = 600.0
ALTURA_PELDANO_MAX = 40.0  # por debajo del escalón que sube el movimiento: las escaleras se andan


def crear_muro_con_puerta(actores, etiqueta, x, ancho, alto, materiales, hueco=ANCHO_PUERTA, alto_hueco=ALTO_PUERTA):
    """Muro transversal en x (de pared a pared) con una puerta centrada; hueco 0 = muro ciego."""
    alto_total = alto - FONDO_MUROS
    if hueco <= 0:
        crear_bloque(actores, f"{etiqueta}", unreal.Vector(x, 0, FONDO_MUROS + alto_total / 2), (100, ancho + 200, alto_total), materiales["Acento"])
        return
    tramo = (ancho + 200 - hueco) / 2.0
    for lado, signo in (("I", -1), ("D", 1)):
        y = signo * (hueco / 2.0 + tramo / 2.0)
        crear_bloque(actores, f"{etiqueta}_{lado}", unreal.Vector(x, y, FONDO_MUROS + alto_total / 2), (100, tramo, alto_total), materiales["Acento"])
    dintel = alto - alto_hueco
    crear_bloque(actores, f"{etiqueta}_Dintel", unreal.Vector(x, 0, alto_hueco + dintel / 2), (100, hueco, dintel), materiales["Acento"])


def crear_sala(actores, etiqueta, x_inicio, x_fin, ancho, alto, materiales, suelo=True, hueco_entrada=ANCHO_PUERTA, hueco_salida=ANCHO_PUERTA):
    """Sala cerrada: suelo opcional, muros laterales (bajan hasta el fondo de los fosos) y muros de entrada y
    salida con puerta (hueco 0 = ciego). Cada sala pone los dos con su propio ancho: entre salas de anchos
    distintos el muro común tapa hasta la pared de la más ancha."""
    largo = x_fin - x_inicio
    centro_x = (x_inicio + x_fin) / 2.0
    if suelo:
        crear_suelo(actores, f"{etiqueta}_Suelo", x_inicio, x_fin, materiales, ancho=ancho)
    alto_total = alto - FONDO_MUROS
    for lado, signo in (("I", -1), ("D", 1)):
        crear_bloque(actores, f"{etiqueta}_Muro{lado}", unreal.Vector(centro_x, signo * (ancho / 2 + 50), FONDO_MUROS + alto_total / 2),
                     (largo, 100, alto_total), materiales["Acento"])
    crear_muro_con_puerta(actores, f"{etiqueta}_Entrada", x_inicio, ancho, alto, materiales, hueco=hueco_entrada)
    crear_muro_con_puerta(actores, f"{etiqueta}_Salida", x_fin, ancho, alto, materiales, hueco=hueco_salida)


def crear_escalera(actores, etiqueta, x_inicio, x_fin, z_inicio, z_fin, ancho, materiales, y=0.0, eje="x"):
    """Escalera entre dos alturas con peldaños macizos que se suben andando. Por defecto a lo largo de X
    (x_inicio..x_fin, centrada en y); con eje="y", a lo largo de Y (los mismos argumentos con X e Y cambiados)."""
    peldanos = max(1, int((abs(z_fin - z_inicio) + ALTURA_PELDANO_MAX - 1e-3) // ALTURA_PELDANO_MAX))
    largo = (x_fin - x_inicio) / peldanos
    for indice in range(peldanos):
        # Altura de la cara superior del peldaño: sube hacia z_fin según se avanza en X.
        tope = z_inicio + (z_fin - z_inicio) * (indice + 1) / peldanos if z_fin > z_inicio else z_inicio + (z_fin - z_inicio) * indice / peldanos
        if tope <= 0:
            continue
        a = x_inicio + largo * (indice + 0.5)
        if eje == "y":
            crear_bloque(actores, f"{etiqueta}_{indice}", unreal.Vector(y, a, tope / 2), (ancho, abs(largo), tope), materiales["Piedra"])
        else:
            crear_bloque(actores, f"{etiqueta}_{indice}", unreal.Vector(a, y, tope / 2), (abs(largo), ancho, tope), materiales["Piedra"])


def crear_columna(actores, etiqueta, x, y, alto, materiales, grosor=160.0):
    """Columna maciza (se puede correr por ella y rebotar entre dos)."""
    crear_bloque(actores, etiqueta, unreal.Vector(x, y, alto / 2), (grosor, grosor, alto), materiales["Acento"])


def crear_antorcha(actores, etiqueta, posicion, materiales):
    """Brasero en la pared: bloque cálido que brilla y luz puntual sin sombras (barata)."""
    crear_bloque(actores, f"{etiqueta}_Brasero", posicion, (30, 30, 40), materiales["Puerta"])
    luz = actores.spawn_actor_from_class(unreal.PointLight, unreal.Vector(posicion.x, posicion.y, posicion.z + 60))
    luz.set_actor_label(f"{etiqueta}_Luz")
    componente = luz.get_component_by_class(unreal.PointLightComponent)
    componente.set_mobility(unreal.ComponentMobility.MOVABLE)
    componente.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    componente.set_editor_property("intensity", 25.0)
    componente.set_editor_property("attenuation_radius", 1100.0)
    componente.set_editor_property("light_color", unreal.Color(r=255, g=160, b=80, a=255))
    componente.set_editor_property("cast_shadows", False)
    luz.set_folder_path("Iluminacion")


def antorchas_puerta(actores, etiqueta, x, ancho, materiales, lado=1):
    """Un brasero junto a la puerta de salida, por dentro de la sala, en el lado indicado."""
    crear_antorcha(actores, etiqueta, unreal.Vector(x - 80, lado * (ANCHO_PUERTA / 2 + 120), 320), materiales)


def crear_suelo(actores, etiqueta, x_inicio, x_fin, materiales, altura=0.0, ancho=ANCHO_PASILLO):
    largo = x_fin - x_inicio
    return crear_bloque(actores, etiqueta, unreal.Vector((x_inicio + x_fin) / 2.0, 0, altura - GROSOR_SUELO / 2), (largo, ancho, GROSOR_SUELO),
                        materiales["Piedra"])


def crear_puerta(actores, etiqueta, x, ancho, alto, enemigos, material, titulo, texto, altura=0.0, y=0.0):
    """Puerta de arena (se abre al morir sus enemigos) o, sin enemigos, compuerta de una palanca.
    Una puerta de arena cuyos enemigos no se crearon no se pone (se quedaría cerrada para siempre)."""
    vivos = [enemigo for enemigo in enemigos if enemigo is not None]
    if enemigos and not vivos:
        return None
    enemigos = vivos
    puerta = actores.spawn_actor_from_class(unreal.SAEVUSPuertaArena, unreal.Vector(x, y, altura + alto / 2.0))
    puerta.set_actor_label(etiqueta)
    puerta.set_actor_scale3d(unreal.Vector(0.6, ancho / 100.0, alto / 100.0))
    puerta.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
    puerta.set_editor_property("enemigos", enemigos)
    puerta.set_editor_property("titulo_al_abrir", unreal.Text(titulo))
    puerta.set_editor_property("texto_al_abrir", unreal.Text(texto))
    puerta.set_folder_path("Tutorial")
    return puerta


def crear_iluminacion_oscura(actores):
    """Noche fría sin atmósfera: luz de luna azulada, cielo apagado, niebla densa. Exposición fija."""
    luna = actores.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-55, yaw=-40))
    luna.set_actor_label("Luna")
    luz = luna.get_component_by_class(unreal.DirectionalLightComponent)
    luz.set_mobility(unreal.ComponentMobility.MOVABLE)
    luz.set_editor_property("intensity", 2.5)
    luz.set_editor_property("light_color", unreal.Color(r=170, g=190, b=255, a=255))
    # La luna manda en el sombreado forward/translúcido; el relleno no compite con ella.
    luz.set_editor_property("forward_shading_priority", 1)

    # Relleno desde el lado contrario, sin sombras: sustituye a la luz de cielo (sin atmósfera el cielo es
    # negro y no aportaría luz ambiente) y deja leer las formas en sombra.
    cielo = actores.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-30, yaw=140))
    cielo.set_actor_label("Relleno")
    relleno = cielo.get_component_by_class(unreal.DirectionalLightComponent)
    relleno.set_mobility(unreal.ComponentMobility.MOVABLE)
    relleno.set_editor_property("intensity", 0.8)
    relleno.set_editor_property("light_color", unreal.Color(r=90, g=110, b=170, a=255))
    relleno.set_editor_property("cast_shadows", False)

    niebla = actores.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, -200))
    niebla.set_actor_label("Niebla")
    componente_niebla = niebla.get_component_by_class(unreal.ExponentialHeightFogComponent)
    componente_niebla.set_editor_property("fog_density", 0.04)
    componente_niebla.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.02, 0.03, 0.06, 1.0))

    # Exposición manual: sin adaptación automática, la oscuridad se queda oscura.
    volumen = actores.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0))
    volumen.set_actor_label("PostProceso")
    volumen.set_editor_property("unbound", True)
    ajustes = volumen.get_editor_property("settings")
    ajustes.set_editor_property("override_auto_exposure_method", True)
    ajustes.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_MANUAL)
    ajustes.set_editor_property("override_auto_exposure_apply_physical_camera_exposure", True)
    ajustes.set_editor_property("auto_exposure_apply_physical_camera_exposure", False)
    ajustes.set_editor_property("override_auto_exposure_bias", True)
    ajustes.set_editor_property("auto_exposure_bias", 0.0)
    volumen.set_editor_property("settings", ajustes)
    for actor in (luna, cielo, niebla, volumen):
        actor.set_folder_path("Iluminacion")


def construir_mapa_tutorial(bp_modo_juego, datos_enemigos, material_base):
    """
    Tutorial como mazmorra (estructura de referencia: el mapa Dungeon de GunZ): salas cerradas unidas por
    puertas hacia +X. Las medidas de los obstáculos son las probadas por SAEVUS.Tutorial.Obstaculos.*.
    """
    editor_niveles, actores = abrir_mapa_limpio(RUTA_MAPA_TUTORIAL)
    materiales = {nombre: crear_instancia_material(f"MI_Escenario_{nombre}", DIR_ESTILO, material_base, valores)
                  for nombre, valores in PIEZAS_ESCENARIO.items()}
    mallas = importar_decorado(materiales["Acento"])

    # Sala 0, la celda: moverse y mirar.
    crear_sala(actores, "S0", -600, 1400, 1400, 800, materiales, hueco_entrada=0)
    actores.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100)).set_actor_label("Inicio")
    crear_texto(actores, "Cartel_SAEVUS", "SAEVUS", unreal.Vector(1340, 0, 640), tamano=150.0)
    crear_info_zona(actores, "Mazmorra del Umbral", "Tutorial")
    antorchas_puerta(actores, "S0_Antorcha", 1400, 1400, materiales)
    crear_decorado(actores, "S0_Escombros", mallas["SM_Escombros"], unreal.Vector(-300, 500, 0), guinada=40)

    # Sala 1: escalón de 1 m (salto) y bloque de 2,2 m (doble salto), de pared a pared; se baja por una escalera.
    crear_sala(actores, "S1", 1400, 3600, 1200, 800, materiales)
    crear_bloque(actores, "S1_Escalon100", unreal.Vector(2000, 0, 50), (400, 1200, 100), materiales["Acento"])
    crear_bloque(actores, "S1_Bloque220", unreal.Vector(2600, 0, 110), (400, 1200, 220), materiales["Acento"])
    crear_escalera(actores, "S1_Bajada", 2800, 3400, 220, 0, 1200, materiales)
    antorchas_puerta(actores, "S1_Antorcha", 3600, 1200, materiales, lado=-1)

    # Sala 2: foso de 8 m (dash).
    crear_sala(actores, "S2", 3600, 5800, 1200, 800, materiales, suelo=False)
    crear_suelo(actores, "S2_Suelo", 3600, 4000, materiales, ancho=1200)
    crear_foso(actores, "S2_Foso", 4000, 4800, unreal.Vector(3800, 0, 100), materiales)
    crear_suelo(actores, "S2_Llegada", 4800, 5800, materiales, ancho=1200)
    antorchas_puerta(actores, "S2_Antorcha", 5800, 1200, materiales)

    # Sala 3, la chimenea: dos paredes a 3 m para rebotar entre ellas y una cornisa de 4,5 m que se escala.
    crear_sala(actores, "S3", 5800, 7600, 800, 1000, materiales)
    for lado, y in (("I", -200), ("D", 200)):
        crear_bloque(actores, f"S3_Chimenea{lado}", unreal.Vector(6250, y, 350), (500, 100, 700), materiales["Acento"])
    crear_bloque(actores, "S3_Cornisa450", unreal.Vector(7000, 0, 225), (600, 800, 450), materiales["Acento"])
    antorchas_puerta(actores, "S3_Antorcha", 7600, 800, materiales, lado=-1)

    # Sala 4, la galería: foso de 9 m junto al muro derecho (en y = 450, como la pared probada).
    crear_sala(actores, "S4", 7600, 9800, 800, 900, materiales, suelo=False)
    crear_suelo(actores, "S4_Suelo", 7600, 8000, materiales)
    crear_foso(actores, "S4_Foso", 8000, 8900, unreal.Vector(7800, 0, 100), materiales)
    crear_suelo(actores, "S4_Llegada", 8900, 9800, materiales)
    antorchas_puerta(actores, "S4_Antorcha", 9800, 800, materiales)

    # Sala 5, la sala de armas: combo, tajo hacia arriba, técnicas y guardia, entre cuatro columnas.
    crear_sala(actores, "S5", 9800, 12600, 1800, 900, materiales)
    crear_punto_control(actores, "Control_Combate", 9900, ancho=1800)
    for indice, y in enumerate((-250, 0, 250)):
        crear_muneco(actores, f"S5_Muneco{indice}", unreal.Vector(10500, y, 96), unreal.Rotator(0, 0, 180))
    crear_muneco(actores, "S5_MunecoAlto", unreal.Vector(11100, 0, 96 + 190), unreal.Rotator(0, 0, 180))
    for indice, y in enumerate((-180, 180)):
        crear_muneco(actores, f"S5_MunecoCargado{indice}", unreal.Vector(11800, y, 96), unreal.Rotator(0, 0, 180))
    for indice, (x, y) in enumerate(((10200, -650), (10200, 650), (12200, -650), (12200, 650))):
        crear_columna(actores, f"S5_Columna{indice}", x, y, 900, materiales)
    antorchas_puerta(actores, "S5_Antorcha", 12600, 1800, materiales, lado=-1)

    # Sala 6, la capilla: alma, curación y banco.
    crear_sala(actores, "S6", 12600, 14400, 1200, 800, materiales)
    crear_punto_control(actores, "Control_Alma", 12700, ancho=1200)
    for indice, y in enumerate((-200, 200)):
        crear_muneco(actores, f"S6_Muneco{indice}", unreal.Vector(13500, y, 96), unreal.Rotator(0, 0, 180))
    crear_banco(actores, "S6_Banco", 13000, -480, 0, materiales)
    antorchas_puerta(actores, "S6_Antorcha", 14400, 1200, materiales)

    # Sala 7, el gran salón (la sala central del Dungeon de GunZ): balcones a 4 m a ambos lados con
    # escaleras, columnas en dos filas para correr y rebotar, y un estrado en el centro.
    crear_sala(actores, "S7", 14400, 19000, 3000, 1200, materiales)
    crear_punto_control(actores, "Control_Salon", 14500, ancho=3000)
    for lado, signo in (("I", -1), ("D", 1)):
        y_balcon = signo * 1300
        crear_escalera(actores, f"S7_Escalera{lado}", 14600, 15400, 0, 400, 400, materiales, y=y_balcon)
        crear_bloque(actores, f"S7_Balcon{lado}", unreal.Vector(17000, y_balcon, 360), (3200, 400, 80), materiales["Piedra"])
        for indice, x in enumerate(range(15800, 18700, 800)):
            crear_columna(actores, f"S7_Apoyo{lado}{indice}", x, signo * 1150, 320, materiales, grosor=80.0)
        crear_antorcha(actores, f"S7_Antorcha{lado}", unreal.Vector(17000, signo * 1460, 650), materiales)
    for indice, x in enumerate((15600, 16400, 17200, 18000)):
        for lado, y in (("I", -600), ("D", 600)):
            crear_columna(actores, f"S7_Columna{lado}{indice}", x, y, 1100, materiales)
    crear_bloque(actores, "S7_Estrado", unreal.Vector(16800, 0, 20), (800, 800, 40), materiales["Acento"])
    for indice, (x, y) in enumerate(((14700, -1350), (18700, 1350), (18700, -1350))):
        crear_decorado(actores, f"S7_ColumnaRota{indice}", mallas["SM_ColumnaRota"], unreal.Vector(x, y, 0), guinada=indice * 70, escala=1.3)
    antorchas_puerta(actores, "S7_Antorcha", 19000, 3000, materiales)

    # Sala 8: fin del tutorial, banco y paso a la Mazmorra (y llegada desde ella).
    crear_sala(actores, "S8", 19000, 20400, 1200, 800, materiales)
    crear_texto(actores, "Cartel_Fin", "Fin del tutorial", unreal.Vector(20300, 0, 450), tamano=100.0)
    crear_banco(actores, "S8_Banco", 19400, -480, 0, materiales)
    crear_paso(actores, "S8_PasoMazmorra", unreal.Vector(19800, 540, 150), RUTA_MAPA_MAZMORRA, "DesdeTutorial", "Mazmorra  ->")
    crear_llegada(actores, "Llegada_DesdeMazmorra", unreal.Vector(19800, 0, 100), 0.0, "DesdeMazmorra")

    # Sala 9, reto opcional: obstáculos ya probados (SAEVUS.Tutorial.Obstaculos.*) encadenados sin descanso.
    crear_sala(actores, "S9", 20400, 24400, 800, 1000, materiales, suelo=False, hueco_salida=0)
    crear_punto_control(actores, "Control_Reto", 20500)
    crear_suelo(actores, "S9_Inicio", 20400, 20600, materiales)
    crear_foso(actores, "S9_Foso1", 20600, 21400, unreal.Vector(20500, 0, 100), materiales)
    crear_peligro(actores, "S9_Pinchos1", unreal.Vector(21000, 0, -120), (400, ANCHO_PASILLO / 2, 30), unreal.Vector(20500, 0, 100),
                  materiales["Puerta"])
    crear_suelo(actores, "S9_Suelo1", 21400, 21800, materiales)
    crear_foso(actores, "S9_Foso2", 21800, 22700, unreal.Vector(21650, 0, 100), materiales)
    crear_suelo(actores, "S9_Suelo2", 22700, 23300, materiales)
    crear_bloque(actores, "S9_Cornisa450", unreal.Vector(23700, 0, 225), (800, ANCHO_PASILLO, 450), materiales["Acento"])
    crear_suelo(actores, "S9_Fondo", 23300, 24400, materiales)
    crear_texto(actores, "Cartel_RetoSuperado", "Reto superado", unreal.Vector(24300, 0, 700), tamano=90.0)
    crear_fragmento(actores, "S9_Fragmento", unreal.Vector(23700, 0, 450 + 100), "Tutorial_Reto", materiales["Puerta"])

    anchos = {100: 1400, 1450: 1200, 3650: 1200, 5850: 800, 7650: 800, 9850: 1800, 10950: 1800, 12650: 1200, 14450: 3000, 20450: 800}
    for x, titulo, texto in AVISOS_TUTORIAL:
        crear_aviso(actores, x, titulo, texto, ancho=anchos[x])

    crear_iluminacion_oscura(actores)
    mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mundo.get_world_settings().set_editor_property("default_game_mode", bp_modo_juego.generated_class())
    editor_niveles.save_current_level()
    registrar(f"mapa tutorial: {RUTA_MAPA_TUTORIAL}")


# --- Mazmorra: mapa de práctica (referencia de estructura: el Dungeon de GunZ) -------------
# Salón central con anillo de balcones, columnas y estrado; alrededor, cuatro salas: entrada (oeste),
# galería de fosos (este, con un muro falso y una sala secreta), pozo (norte) y terraza (sur).
RUTA_MAPA_MAZMORRA = f"{RAIZ}/Mapas/L_Mazmorra"
RUTA_MAPA_ZONA1_RETIRADA = f"{RAIZ}/Mapas/L_Zona1"
ALTO_SALON = 1400.0
ALTURA_BALCON = 800.0
GROSOR_BALCON = 80.0


def crear_muro(actores, etiqueta, eje, fijo, desde, hasta, alto, materiales, huecos=()):
    """Muro recto de 1 m de grosor del fondo de los fosos a 'alto'. eje "x": a lo largo de X en y = fijo;
    eje "y": a lo largo de Y en x = fijo. huecos: (centro, ancho, alto del hueco) con dintel encima."""
    def tramo(nombre, a, b, z0, z1):
        centro = (a + b) / 2.0
        if eje == "x":
            crear_bloque(actores, nombre, unreal.Vector(centro, fijo, (z0 + z1) / 2.0), (b - a, 100, z1 - z0), materiales["Acento"])
        else:
            crear_bloque(actores, nombre, unreal.Vector(fijo, centro, (z0 + z1) / 2.0), (100, b - a, z1 - z0), materiales["Acento"])
    inicio = desde
    for indice, (centro, ancho, alto_hueco) in enumerate(sorted(huecos)):
        a, b = centro - ancho / 2.0, centro + ancho / 2.0
        if a > inicio:
            tramo(f"{etiqueta}_{indice}", inicio, a, FONDO_MUROS, alto)
        tramo(f"{etiqueta}_Dintel{indice}", a, b, alto_hueco, alto)
        inicio = b
    if hasta > inicio:
        tramo(f"{etiqueta}_Fin", inicio, hasta, FONDO_MUROS, alto)


def crear_recinto(actores, etiqueta, x0, x1, y0, y1, alto, materiales, muros, suelo=True):
    """Sala rectangular (interior x0..x1, y0..y1). muros: {lado: huecos} con lado en xmin, xmax, ymin, ymax;
    un lado que no está no se construye (lo pone la sala vecina)."""
    if suelo:
        crear_bloque(actores, f"{etiqueta}_Suelo", unreal.Vector((x0 + x1) / 2.0, (y0 + y1) / 2.0, -GROSOR_SUELO / 2),
                     (x1 - x0, y1 - y0, GROSOR_SUELO), materiales["Piedra"])
    if "ymin" in muros:
        crear_muro(actores, f"{etiqueta}_MuroYmin", "x", y0 - 50, x0 - 100, x1 + 100, alto, materiales, muros["ymin"])
    if "ymax" in muros:
        crear_muro(actores, f"{etiqueta}_MuroYmax", "x", y1 + 50, x0 - 100, x1 + 100, alto, materiales, muros["ymax"])
    if "xmin" in muros:
        crear_muro(actores, f"{etiqueta}_MuroXmin", "y", x0 - 50, y0, y1, alto, materiales, muros["xmin"])
    if "xmax" in muros:
        crear_muro(actores, f"{etiqueta}_MuroXmax", "y", x1 + 50, y0, y1, alto, materiales, muros["xmax"])


def crear_balcon(actores, etiqueta, x0, x1, y0, y1, materiales, eje_apoyos, borde_apoyos):
    """Losa a ALTURA_BALCON con apoyos cada 6 m en su borde interior (eje_apoyos: "x" o "y")."""
    crear_bloque(actores, etiqueta, unreal.Vector((x0 + x1) / 2.0, (y0 + y1) / 2.0, ALTURA_BALCON - GROSOR_BALCON / 2),
                 (x1 - x0, y1 - y0, GROSOR_BALCON), materiales["Piedra"])
    alto_apoyo = ALTURA_BALCON - GROSOR_BALCON
    desde, hasta = (x0, x1) if eje_apoyos == "x" else (y0, y1)
    numero = max(2, int((hasta - desde) // 600) + 1)
    for indice in range(numero):
        a = desde + 60 + (hasta - desde - 120) * indice / (numero - 1)
        x, y = (a, borde_apoyos) if eje_apoyos == "x" else (borde_apoyos, a)
        crear_columna(actores, f"{etiqueta}_Apoyo{indice}", x, y, alto_apoyo, materiales, grosor=80.0)


def construir_mapa_mazmorra(bp_modo_juego, material_base):
    editor_niveles, actores = abrir_mapa_limpio(RUTA_MAPA_MAZMORRA)
    materiales = {nombre: crear_instancia_material(f"MI_Escenario_{nombre}", DIR_ESTILO, material_base, valores)
                  for nombre, valores in PIEZAS_ESCENARIO.items()}
    mallas = importar_decorado(materiales["Acento"])
    puerta = lambda centro=0.0: [(centro, 600.0, 700.0)]

    # Salón central: 40 x 40 m, puertas en los cuatro lados.
    crear_recinto(actores, "Salon", -2000, 2000, -2000, 2000, ALTO_SALON, materiales,
                  {"xmin": puerta(), "xmax": puerta(), "ymin": puerta(), "ymax": puerta()})
    crear_info_zona(actores, "La Mazmorra", "Práctica")
    # Anillo de balcones a 8 m, abierto donde suben las escaleras (junto a los muros norte y sur).
    crear_escalera(actores, "Salon_EscaleraN", -1700, -300, 0, ALTURA_BALCON, 400, materiales, y=1800)
    crear_escalera(actores, "Salon_EscaleraS", 300, 1700, ALTURA_BALCON, 0, 400, materiales, y=-1800)
    crear_balcon(actores, "Salon_BalconN", -300, 2000, 1600, 2000, materiales, "x", 1640)
    crear_balcon(actores, "Salon_BalconS", -2000, 300, -2000, -1600, materiales, "x", -1640)
    crear_balcon(actores, "Salon_BalconE", 1600, 2000, -1600, 1600, materiales, "y", 1640)
    crear_balcon(actores, "Salon_BalconO", -2000, -1600, -1600, 1600, materiales, "y", -1640)
    # Ocho columnas en corro (para correr por ellas y rebotar) y un estrado bajo en el centro.
    for indice, (x, y) in enumerate(((1200, 0), (-1200, 0), (0, 1200), (0, -1200), (850, 850), (-850, 850), (850, -850), (-850, -850))):
        crear_columna(actores, f"Salon_Columna{indice}", x, y, ALTO_SALON - 100, materiales, grosor=200.0)
    crear_bloque(actores, "Salon_Estrado", unreal.Vector(0, 0, 60), (1000, 1000, 120), materiales["Acento"])
    for indice, (x, y) in enumerate(((-1940, 1000), (1940, -1000), (1000, 1940), (-1000, -1940))):
        crear_antorcha(actores, f"Salon_Antorcha{indice}", unreal.Vector(x, y, 1050), materiales)

    # Entrada (oeste): inicio, banco, paso al tutorial.
    crear_recinto(actores, "Entrada", -4800, -2000, -900, 900, 900, materiales, {"xmin": [], "ymin": [], "ymax": []})
    actores.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-4000, 0, 100)).set_actor_label("Inicio")
    crear_llegada(actores, "Llegada_DesdeTutorial", unreal.Vector(-3600, 0, 100), 0.0, "DesdeTutorial")
    crear_paso(actores, "Entrada_PasoTutorial", unreal.Vector(-4500, 740, 150), RUTA_MAPA_TUTORIAL, "DesdeMazmorra", "<-  Tutorial")
    crear_banco(actores, "Entrada_Banco", -3000, -700, 0, materiales)
    crear_punto_control(actores, "Control_Entrada", -3300, ancho=1800)
    crear_aviso(actores, -3800, "La Mazmorra", "Práctica libre. Al este, la galería: carrera por pared y dash.\n"
                "Al norte, el pozo: rebota entre paredes o sube en el ascensor.\n"
                "Al sur, la terraza: una palanca abre una compuerta. Hay secretos.", ancho=1800)
    crear_antorcha(actores, "Entrada_Antorcha", unreal.Vector(-2300, 780, 320), materiales)
    crear_decorado(actores, "Entrada_Escombros", mallas["SM_Escombros"], unreal.Vector(-4500, -600, 0), guinada=20)

    # Galería (este): foso de 9 m junto al muro derecho (carrera por pared), suelo, foso de 8 m (dash);
    # al fondo, un muro agrietado (pared falsa) da a una sala secreta.
    crear_recinto(actores, "Galeria", 2000, 6000, -800, 800, 1000, materiales,
                  {"ymin": [], "ymax": [], "xmax": [(0.0, 400.0, 400.0)]}, suelo=False)
    crear_suelo(actores, "Galeria_Suelo0", 2000, 2600, materiales, ancho=1600)
    crear_foso(actores, "Galeria_Foso9", 2600, 3500, unreal.Vector(2300, 0, 100), materiales)
    crear_suelo(actores, "Galeria_Suelo1", 3500, 4400, materiales, ancho=1600)
    crear_foso(actores, "Galeria_Foso8", 4400, 5200, unreal.Vector(4200, 0, 100), materiales)
    # Hasta 6100: también bajo la pared falsa, para pasar a la sala secreta al romperla.
    crear_suelo(actores, "Galeria_Suelo2", 5200, 6100, materiales, ancho=1600)
    crear_aviso(actores, 2050, "Galería", "Foso de 9 m: corre por la pared de la derecha.\n"
                "Foso de 8 m: salto, doble salto y dash.\nAl fondo, el muro suena hueco.", ancho=1600)
    crear_pared_rompible(actores, "Galeria_ParedFalsa", unreal.Vector(6050, 0, 200), (100, 400, 400), materiales["Acento"])
    crear_recinto(actores, "Secreto", 6100, 6900, -300, 300, 500, materiales, {"ymin": [], "ymax": [], "xmax": []})
    crear_fragmento(actores, "Secreto_Fragmento", unreal.Vector(6600, 0, 100), "Mazmorra_Secreto", materiales["Puerta"])
    crear_antorcha(actores, "Galeria_Antorcha", unreal.Vector(5800, -760, 320), materiales)

    # Pozo (norte): chimenea de paredes a 3 m hasta una cornisa de 9 m; o el ascensor junto a ella.
    crear_recinto(actores, "Pozo", -800, 800, 2000, 5000, 1600, materiales, {"xmin": [], "xmax": [], "ymax": []})
    for lado, x in (("I", -200), ("D", 200)):
        crear_bloque(actores, f"Pozo_Chimenea{lado}", unreal.Vector(x, 3300, 550), (100, 600, 1100), materiales["Acento"])
    crear_bloque(actores, "Pozo_Cornisa", unreal.Vector(0, 4300, 450), (1600, 1400, 900), materiales["Acento"])
    crear_plataforma_movil(actores, "Pozo_Ascensor", unreal.Vector(550, 3500, -48), (200, 200, 100), unreal.Vector(0, 0, 900), 200.0, 1.2,
                           materiales["Acento"])
    crear_texto(actores, "Cartel_Pozo", "El pozo", unreal.Vector(0, 3000, 1300), tamano=90.0)
    crear_fragmento(actores, "Pozo_Fragmento", unreal.Vector(0, 4700, 1000), "Mazmorra_Pozo", materiales["Puerta"])
    crear_antorcha(actores, "Pozo_Antorcha", unreal.Vector(0, 4960, 1200), materiales)

    # Terraza (sur): escalera hasta una terraza a 6 m; en ella, una palanca abre la compuerta del fondo.
    crear_recinto(actores, "Terraza", -1000, 1000, -4600, -2000, 1200, materiales, {"xmin": [], "xmax": [], "ymin": []})
    crear_escalera(actores, "Terraza_Escalera", -2400, -3600, 0, 600, 800, materiales, y=-600, eje="y")
    crear_bloque(actores, "Terraza_Plataforma", unreal.Vector(0, -4100, 300), (2000, 1000, 600), materiales["Acento"])
    compuerta = crear_puerta(actores, "Terraza_Compuerta", 300, 1000, 500, [], materiales["Puerta"], "", "", altura=600, y=-4100)
    crear_palanca(actores, "Terraza_Palanca", unreal.Vector(0, -4100, 600), [compuerta], materiales,
                  "Compuerta abierta", "Detrás espera un fragmento de máscara.")
    crear_fragmento(actores, "Terraza_Fragmento", unreal.Vector(700, -4100, 700), "Mazmorra_Terraza", materiales["Puerta"])
    crear_texto(actores, "Cartel_Terraza", "La terraza", unreal.Vector(0, -3000, 1100), tamano=90.0)
    crear_antorcha(actores, "Terraza_Antorcha", unreal.Vector(940, -2300, 320), materiales)

    crear_iluminacion_oscura(actores)
    mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mundo.get_world_settings().set_editor_property("default_game_mode", bp_modo_juego.generated_class())
    editor_niveles.save_current_level()
    registrar(f"mapa mazmorra: {RUTA_MAPA_MAZMORRA}")


def crear_palanca(actores, etiqueta, posicion, puertas, materiales, titulo, texto):
    """Palanca que se acciona de un golpe y abre sus puertas (compuertas sin enemigos)."""
    palanca = actores.spawn_actor_from_class(unreal.SAEVUSPalanca, unreal.Vector(posicion.x, posicion.y, posicion.z + 80), unreal.Rotator(0, 0, 0))
    palanca.set_actor_label(etiqueta)
    palanca.set_editor_property("puertas", puertas)
    palanca.set_editor_property("titulo_al_accionar", unreal.Text(titulo))
    palanca.set_editor_property("texto_al_accionar", unreal.Text(texto))
    for componente in palanca.get_components_by_class(unreal.StaticMeshComponent):
        componente.set_material(0, materiales["Puerta"] if componente.get_name() == "Brazo" else materiales["Acento"])
    palanca.set_folder_path("Mundo")
    return palanca


def crear_banco(actores, etiqueta, x, y, altura, materiales, guinada=0.0):
    """Banco (Hollow Knight) de pie en el suelo a 'altura': sentarse cura, rellena la munición y guarda."""
    banco = actores.spawn_actor_from_class(unreal.SAEVUSBanco, unreal.Vector(x, y, altura + 100), unreal.Rotator(0, 0, guinada))
    banco.set_actor_label(etiqueta)
    for componente in banco.get_components_by_class(unreal.StaticMeshComponent):
        componente.set_material(0, materiales["Acento"])
    banco.set_folder_path("Mundo")
    return banco


def crear_paso(actores, etiqueta, posicion, ruta_mapa, llegada, cartel):
    """Paso a otro mapa (ASAEVUSVolumenCambioMapa) con su cartel; llegada = etiqueta del inicio de destino."""
    paso = crear_volumen(actores, unreal.SAEVUSVolumenCambioMapa, etiqueta, posicion, (80, 60, 200), "Mundo")
    paso.set_editor_property("mapa", unreal.load_asset(ruta_mapa))
    if llegada:
        paso.set_editor_property("llegada", llegada)
    crear_texto(actores, f"Cartel_{etiqueta}", cartel, unreal.Vector(posicion.x, posicion.y, posicion.z + 180), tamano=50.0)
    return paso


def crear_llegada(actores, etiqueta, posicion, guinada, nombre_llegada):
    """Inicio de llegada desde otro mapa (PlayerStart con etiqueta; el inicio normal no lleva)."""
    inicio = actores.spawn_actor_from_class(unreal.PlayerStart, posicion, unreal.Rotator(0, 0, guinada))
    inicio.set_actor_label(etiqueta)
    inicio.set_editor_property("player_start_tag", nombre_llegada)
    return inicio


def crear_plataforma_movil(actores, etiqueta, centro, tamano_cm, recorrido, velocidad, pausa, material):
    """Plataforma que va y viene (ASAEVUSPlataformaMovil) entre centro y centro + recorrido."""
    plataforma = actores.spawn_actor_from_class(unreal.SAEVUSPlataformaMovil, centro, unreal.Rotator(0, 0, 0))
    plataforma.set_actor_label(etiqueta)
    plataforma.set_actor_scale3d(unreal.Vector(tamano_cm[0] / 100.0, tamano_cm[1] / 100.0, tamano_cm[2] / 100.0))
    plataforma.set_editor_property("recorrido", recorrido)
    plataforma.set_editor_property("velocidad", velocidad)
    plataforma.set_editor_property("pausa_extremo", pausa)
    plataforma.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
    plataforma.set_folder_path("Mundo")
    return plataforma


def crear_info_zona(actores, nombre, subtitulo):
    """Nombre de la zona que la interfaz muestra grande al entrar en el mapa."""
    info = actores.spawn_actor_from_class(unreal.SAEVUSInfoZona, unreal.Vector(0, 0, 0))
    info.set_actor_label("InfoZona")
    info.set_editor_property("nombre", unreal.Text(nombre))
    info.set_editor_property("subtitulo", unreal.Text(subtitulo))
    return info


def crear_pared_rompible(actores, etiqueta, centro, tamano_cm, material):
    """Pared falsa: un bloque que cae a golpes (ASAEVUSParedRompible)."""
    pared = actores.spawn_actor_from_class(unreal.SAEVUSParedRompible, centro, unreal.Rotator(0, 0, 0))
    pared.set_actor_label(etiqueta)
    pared.set_actor_scale3d(unreal.Vector(tamano_cm[0] / 100.0, tamano_cm[1] / 100.0, tamano_cm[2] / 100.0))
    pared.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
    pared.set_folder_path("Mundo")
    return pared


def crear_fragmento(actores, etiqueta, posicion, identificador, material):
    """Fragmento de máscara; identificador único en todo el juego (se guarda al recogerlo)."""
    fragmento = actores.spawn_actor_from_class(unreal.SAEVUSFragmentoMascara, posicion, unreal.Rotator(0, 0, 0))
    fragmento.set_actor_label(etiqueta)
    fragmento.set_editor_property("identificador", identificador)
    fragmento.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
    fragmento.set_folder_path("Mundo")
    return fragmento


def main():
    acciones, imc = crear_assets_entrada()
    config_entrada, movimiento, camara, repertorio, armas = crear_data_assets(acciones)
    bp_modo_juego = crear_blueprints(acciones, imc, config_entrada, movimiento, camara, repertorio, armas)
    datos_enemigos = crear_datos_enemigos(
        cargar(f"{DIR_ESTILO}/M_SAEVUS_PersonajeDibujado"), cargar(f"{DIR_MATERIALES}/M_ParticulaAlma"))
    construir_mapa_pruebas(bp_modo_juego, datos_enemigos)
    # La Zona 1 se retiró (2026-10-04): la sustituye la Mazmorra, que va antes que el tutorial (el paso
    # del final del tutorial apunta a ella).
    if eal.does_asset_exist(RUTA_MAPA_ZONA1_RETIRADA) and eal.delete_asset(RUTA_MAPA_ZONA1_RETIRADA):
        registrar(f"borrado {RUTA_MAPA_ZONA1_RETIRADA}")
    construir_mapa_mazmorra(bp_modo_juego, cargar(f"{DIR_ESTILO}/M_SAEVUS_PersonajeDibujado"))
    construir_mapa_tutorial(bp_modo_juego, datos_enemigos, cargar(f"{DIR_ESTILO}/M_SAEVUS_PersonajeDibujado"))
    if not eal.save_directory(RAIZ, only_if_is_dirty=False, recursive=True):
        raise RuntimeError(f"Falló el guardado de {RAIZ}")
    registrar("hecho")


try:
    main()
except Exception as error:  # noqa: BLE001 - mostrar cualquier fallo en el log y salir igualmente
    unreal.log_error(f"[SAEVUS assets] FALLO: {error}")
    raise
finally:
    unreal.SystemLibrary.quit_editor()
