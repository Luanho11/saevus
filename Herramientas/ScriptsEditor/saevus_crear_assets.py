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
    "IA_Recargar": ["R", "Gamepad_FaceButton_Top"],
    # Mantener para la guardia con el arma cuerpo a cuerpo (GunZ).
    "IA_Guardia": ["RightMouseButton", "Gamepad_LeftTrigger"],
    # Mantener para gastar alma y curar (Hollow Knight).
    "IA_Curar": ["Q", "Gamepad_LeftShoulder"],
    "IA_SiguienteArma": ["MouseScrollDown", "Gamepad_RightShoulder"],
    "IA_RanuraArma1": ["One"],
    "IA_RanuraArma2": ["Two"],
    "IA_RanuraArma3": ["Three"],
    "IA_RanuraArma4": ["Four"],
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
    edicion.connect_material_property(parametro_escalar(material, "Especular", 0.15, -900, 250), "", unreal.MaterialProperty.MP_SPECULAR)

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


# Contorno por profundidad: laplaciano de la inversa de la profundidad (una superficie plana da 0 en
# pantalla, así el suelo no se raya) en 4 vecinos. Siluetas y pliegues fuertes se oscurecen; se
# desvanece con la distancia para no ensuciar el fondo.
CODIGO_CONTORNO = """
float d = max(Profundidad.r, 1.0);
float il = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(-Grosor, 0)).r, 1.0);
float ir = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(Grosor, 0)).r, 1.0);
float iu = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(0, -Grosor)).r, 1.0);
float id = 1.0 / max(SceneTextureFetch(PPI_SceneDepth, float2(0, Grosor)).r, 1.0);
float laplaciano = (abs(il + ir - 2.0 / d) + abs(iu + id - 2.0 / d)) * d;
float borde = saturate((laplaciano - Umbral) * Nitidez) * saturate(1.0 - d / DistanciaMaxima);
return lerp(Escena.rgb, ColorBorde.rgb, borde * Intensidad);
"""


def entrada_personalizada(nombre):
    entrada = unreal.CustomInput()
    entrada.set_editor_property("input_name", nombre)
    return entrada


def crear_material_contorno():
    edicion = unreal.MaterialEditingLibrary
    material = material_vacio("M_PP_Contorno", DIR_ESTILO)
    material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    material.set_editor_property("blendable_location", valor_enum(unreal.BlendableLocation, "BL_SCENE_COLOR_AFTER_DOF", "BL_AFTER_DOF"))

    personalizado = nodo(material, unreal.MaterialExpressionCustom, -300, 0)
    personalizado.set_editor_property("code", CODIGO_CONTORNO)
    personalizado.set_editor_property("output_type", unreal.CustomMaterialOutputType.CMOT_FLOAT3)
    personalizado.set_editor_property("description", "Contorno")
    entradas = ["Escena", "Profundidad", "Grosor", "Umbral", "Nitidez", "Intensidad", "DistanciaMaxima", "ColorBorde"]
    personalizado.set_editor_property("inputs", [entrada_personalizada(nombre) for nombre in entradas])

    escena = nodo(material, unreal.MaterialExpressionSceneTexture, -800, -200)
    escena.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
    profundidad = nodo(material, unreal.MaterialExpressionSceneTexture, -800, -50)
    profundidad.set_editor_property("scene_texture_id", unreal.SceneTextureId.PPI_SCENE_DEPTH)
    edicion.connect_material_expressions(escena, "Color", personalizado, "Escena")
    edicion.connect_material_expressions(profundidad, "Color", personalizado, "Profundidad")
    valores = (("Grosor", 1.0), ("Umbral", 0.08), ("Nitidez", 6.0), ("Intensidad", 0.85), ("DistanciaMaxima", 6000.0))
    for indice, (nombre, valor) in enumerate(valores):
        edicion.connect_material_expressions(parametro_escalar(material, nombre, valor, -800, 100 + indice * 100), "", personalizado, nombre)
    edicion.connect_material_expressions(parametro_vector(material, "ColorBorde", (0.01, 0.01, 0.015, 1.0), -800, 650), "", personalizado, "ColorBorde")
    edicion.connect_material_property(personalizado, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    edicion.recompile_material(material)
    return material


def borrar_materiales_obsoletos():
    """Los M_Caballero_* de color plano los sustituyen las instancias MI_Caballero_*."""
    for clave in PIEZAS_CABALLERO:
        ruta = f"{DIR_CABALLERO}/M_Caballero_{clave}"
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
    config_entrada.set_editor_property("accion_recargar", acciones["IA_Recargar"])
    config_entrada.set_editor_property("accion_curar", acciones["IA_Curar"])
    config_entrada.set_editor_property("accion_guardia", acciones["IA_Guardia"])
    config_entrada.set_editor_property("accion_siguiente_arma", acciones["IA_SiguienteArma"])
    config_entrada.set_editor_property(
        "acciones_ranura_arma", [acciones[f"IA_RanuraArma{ranura}"] for ranura in range(1, len(ARMAS) + 1)]
    )

    # Los valores se quedan en los del código hasta ajustarlos en el editor.
    movimiento = data_asset("DA_AjustesMovimiento_Base", unreal.SAEVUSAjustesMovimiento)
    camara = data_asset("DA_AjustesCamara_Base", unreal.SAEVUSAjustesCamara)
    ajuste_camara = camara.get_editor_property("camara")
    ajuste_camara.set_editor_property("material_contorno", crear_material_contorno())
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
    personaje.get_editor_property("armas").set_editor_property("arsenal", armas)
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


def crear_enemigo(actores, etiqueta, datos, posicion):
    """De pie en el suelo (posicion.z = altura del suelo); los voladores, a su altura de vuelo sobre él."""
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

# Avisos por sección: (x inicio del volumen, título, texto). El cartel 3D repite el título.
AVISOS_TUTORIAL = [
    (100, "Moverse", "WASD: moverse   ·   Ratón: mirar   ·   Mayús: esprintar   ·   Alt: caminar"),
    (1500, "Saltar", "Espacio: saltar.\nPulsa otra vez en el aire: doble salto."),
    (3550, "Dash", "Toca dos veces una dirección (W W, A A...) para hacer dash.\nTambién en el aire: salto, doble salto y dash para cruzar."),
    (5650, "Paredes", "Salta contra una pared para rebotar en ella.\nCorre de frente hacia una pared para escalarla."),
    (7700, "Carrera por pared", "Salta hacia la pared en diagonal y sigue avanzando: correrás por ella.\nSalta para soltarte."),
    (9550, "Combate", "Clic izquierdo: atacar. Encadena tres tajos. Mira hacia arriba al atacar: tajo hacia arriba.\nClic derecho: guardia, bloquea lo que llega de frente. Tajo y guardia se encadenan."),
    (10650, "Técnicas", "Mantén el clic izquierdo y suelta: ataque cargado (giro).\nAtaca durante el dash: tajo en carrera. En el aire: hasta tres tajos seguidos."),
    (11550, "Rebote", "En el aire, mira hacia abajo y ataca: tajo hacia abajo. Si golpeas algo rebotas\ny recuperas el dash y el doble salto. Rebota en los pinchos: tocarlos duele."),
    (13550, "Alma", "Cada golpe llena el alma.\nMantén Q en el suelo para gastarla y curarte. Un golpe te interrumpe."),
    (14600, "Armas de fuego", "1-4 o la rueda: cambiar de arma. Dispara a los muñecos de arriba.\nLas balas de reserva se gastan: cada golpe de espada las devuelve. R: recargar."),
    (15100, "Arena", "Los enemigos avisan antes de atacar: brillan y se agachan. Esquiva con dash y castiga\ncuando se recuperan. Los proyectiles se paran con la guardia o se devuelven con un tajo.\nA los voladores: salta y tajo hacia arriba, o rebota encima."),
    (21250, "Reto (opcional)", "Pinchos, carrera por pared y cornisa sin descanso.\nTodo lo aprendido, encadenado."),
    (17700, "El Guardián", "Un mini jefe. A media vida se enfada: es más rápido y salta.\nUsa todo lo aprendido."),
]


def crear_texto(actores, etiqueta, texto, posicion, tamano=60.0, color=unreal.Color(200, 215, 255, 255)):
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


def crear_aviso(actores, x, titulo, texto, altura=0.0):
    volumen = crear_volumen(actores, unreal.SAEVUSVolumenTutorial, f"Aviso_{titulo}", unreal.Vector(x + 150, 0, altura + 150),
                            (150, ANCHO_PASILLO / 2, 300), "Tutorial")
    volumen.set_editor_property("titulo", unreal.Text(titulo))
    volumen.set_editor_property("texto", unreal.Text(texto))
    crear_texto(actores, f"Cartel_{titulo}", titulo, unreal.Vector(x + 300, -ANCHO_PASILLO / 2 + 40, altura + 260))


def crear_punto_control(actores, etiqueta, x, y=0.0, z=0.0):
    volumen = crear_volumen(actores, unreal.SAEVUSPuntoControl, etiqueta, unreal.Vector(x, y, z + 100), (100, ANCHO_PASILLO / 2, 200), "Tutorial")
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


def decorar_tutorial(actores, mallas):
    # Arcos en los pasos de sección: pilares a los lados del pasillo, que bajan al vacío.
    for x in (1500, 3500, 5600, 7300, 9500, 11500, 13500, 21500):
        crear_decorado(actores, f"Arco_{x}", mallas["SM_ArcoSeccion"], unreal.Vector(x, 0, 0), guinada=90.0)
    # Columnas rotas a ambos lados, fuera del suelo (flotan sobre la niebla del vacío).
    for indice, x in enumerate(range(700, 15000, 1500)):
        for lado, y in (("I", -620), ("D", 620)):
            crear_decorado(actores, f"Columna_{lado}{x}", mallas["SM_ColumnaRota"], unreal.Vector(x + (indice % 2) * 300, y, -250),
                           guinada=(indice * 47) % 360, escala=1.2)
    # Escombros junto a los bordes en las zonas de descanso.
    for x, y in ((600, -300), (1100, 320), (9800, 300), (10900, -320), (13900, 310), (14600, -300)):
        crear_decorado(actores, f"Escombros_{x}", mallas["SM_Escombros"], unreal.Vector(x, y, 0), guinada=(x * 13) % 360)


def crear_suelo(actores, etiqueta, x_inicio, x_fin, materiales, altura=0.0, ancho=ANCHO_PASILLO):
    largo = x_fin - x_inicio
    return crear_bloque(actores, etiqueta, unreal.Vector((x_inicio + x_fin) / 2.0, 0, altura - GROSOR_SUELO / 2), (largo, ancho, GROSOR_SUELO),
                        materiales["Piedra"])


def crear_puerta(actores, etiqueta, x, ancho, alto, enemigos, material, titulo, texto, altura=0.0, y=0.0):
    """Puerta de arena (se abre al morir sus enemigos) o, sin enemigos, compuerta de una palanca."""
    puerta = actores.spawn_actor_from_class(unreal.SAEVUSPuertaArena, unreal.Vector(x, y, altura + alto / 2.0))
    puerta.set_actor_label(etiqueta)
    puerta.set_actor_scale3d(unreal.Vector(0.6, ancho / 100.0, alto / 100.0))
    puerta.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
    puerta.set_editor_property("enemigos", enemigos)
    puerta.set_editor_property("titulo_al_abrir", unreal.Text(titulo))
    puerta.set_editor_property("texto_al_abrir", unreal.Text(texto))
    puerta.set_folder_path("Tutorial")
    return puerta


def crear_arena(actores, etiqueta, x_inicio, x_fin, ancho, alto_muros, materiales, hueco_salida, altura=0.0):
    """Suelo y muros laterales; el muro de salida deja un hueco central de hueco_salida para la puerta."""
    crear_suelo(actores, f"{etiqueta}_Suelo", x_inicio, x_fin, materiales, altura=altura, ancho=ancho)
    largo = x_fin - x_inicio
    centro_x = (x_inicio + x_fin) / 2.0
    for lado, y in (("I", -ancho / 2), ("D", ancho / 2)):
        crear_bloque(actores, f"{etiqueta}_Muro{lado}", unreal.Vector(centro_x, y, altura + alto_muros / 2), (largo, 100, alto_muros), materiales["Acento"])
    tramo = (ancho - hueco_salida) / 2.0
    for lado, signo in (("I", -1), ("D", 1)):
        y = signo * (hueco_salida / 2.0 + tramo / 2.0)
        crear_bloque(actores, f"{etiqueta}_Salida{lado}", unreal.Vector(x_fin, y, altura + alto_muros / 2), (100, tramo, alto_muros), materiales["Acento"])


def crear_iluminacion_oscura(actores):
    """Noche fría sin atmósfera: luz de luna azulada, cielo apagado, niebla densa. Exposición fija."""
    luna = actores.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-55, yaw=-40))
    luna.set_actor_label("Luna")
    luz = luna.get_component_by_class(unreal.DirectionalLightComponent)
    luz.set_mobility(unreal.ComponentMobility.MOVABLE)
    luz.set_editor_property("intensity", 2.5)
    luz.set_editor_property("light_color", unreal.Color(170, 190, 255, 255))
    # La luna manda en el sombreado forward/translúcido; el relleno no compite con ella.
    luz.set_editor_property("forward_shading_priority", 1)

    # Relleno desde el lado contrario, sin sombras: sustituye a la luz de cielo (sin atmósfera el cielo es
    # negro y no aportaría luz ambiente) y deja leer las formas en sombra.
    cielo = actores.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000), unreal.Rotator(roll=0, pitch=-30, yaw=140))
    cielo.set_actor_label("Relleno")
    relleno = cielo.get_component_by_class(unreal.DirectionalLightComponent)
    relleno.set_mobility(unreal.ComponentMobility.MOVABLE)
    relleno.set_editor_property("intensity", 0.8)
    relleno.set_editor_property("light_color", unreal.Color(90, 110, 170, 255))
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
    editor_niveles, actores = abrir_mapa_limpio(RUTA_MAPA_TUTORIAL)
    materiales = {nombre: crear_instancia_material(f"MI_Escenario_{nombre}", DIR_ESTILO, material_base, valores)
                  for nombre, valores in PIEZAS_ESCENARIO.items()}

    # S0 Inicio.
    crear_suelo(actores, "S0_Suelo", -500, 1500, materiales)
    actores.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100)).set_actor_label("Inicio")
    crear_texto(actores, "Cartel_SAEVUS", "SAEVUS", unreal.Vector(900, 0, 450), tamano=160.0)

    # S1 Saltar: escalón de 1 m (salto) y bloque de 2,2 m (doble salto).
    crear_suelo(actores, "S1_Suelo", 1500, 3500, materiales)
    crear_bloque(actores, "S1_Escalon100", unreal.Vector(2000, 0, 50), (400, ANCHO_PASILLO, 100), materiales["Acento"])
    crear_bloque(actores, "S1_Bloque220", unreal.Vector(2600, 0, 110), (400, ANCHO_PASILLO, 220), materiales["Acento"])

    # S2 Dash: foso de 8 m.
    crear_foso(actores, "S2_Foso", 3500, 4300, unreal.Vector(3300, 0, 100), materiales)
    crear_suelo(actores, "S2_Suelo", 4300, 5600, materiales)

    # S3 Paredes: chimenea de 7 m para saltos de pared y cornisa de 4,5 m que se puede escalar.
    crear_suelo(actores, "S3_Suelo", 5600, 7400, materiales)
    for lado, y in (("I", -200), ("D", 200)):
        crear_bloque(actores, f"S3_Chimenea{lado}", unreal.Vector(6450, y, 350), (500, 100, 700), materiales["Acento"])
    crear_bloque(actores, "S3_Cornisa450", unreal.Vector(7000, 0, 225), (600, ANCHO_PASILLO, 450), materiales["Acento"])

    # S3b Carrera por pared: foso de 9 m junto a una pared larga.
    crear_suelo(actores, "S3b_Suelo", 7300, 8000, materiales)
    crear_foso(actores, "S3b_Foso", 8000, 8900, unreal.Vector(7800, 0, 100), materiales)
    crear_bloque(actores, "S3b_ParedCarrera", unreal.Vector(8450, ANCHO_PASILLO / 2 + 50, 300), (1500, 100, 600), materiales["Acento"])
    crear_suelo(actores, "S3b_Llegada", 8900, 9500, materiales)

    # S4 Combate: tres muñecos y uno flotando para el tajo hacia arriba.
    crear_suelo(actores, "S4_Suelo", 9500, 11500, materiales)
    crear_punto_control(actores, "Control_Combate", 9600)
    for indice, y in enumerate((-250, 0, 250)):
        crear_muneco(actores, f"S4_Muneco{indice}", unreal.Vector(10400, y, 96), unreal.Rotator(0, 0, 180))
    crear_muneco(actores, "S4_MunecoAlto", unreal.Vector(11000, 0, 96 + 190), unreal.Rotator(0, 0, 180))
    # Pareja para el ataque cargado (a los lados) y uno lejano para el tajo en carrera.
    for indice, y in enumerate((-180, 180)):
        crear_muneco(actores, f"S4_MunecoCargado{indice}", unreal.Vector(10900, y, 96), unreal.Rotator(0, 0, 180))

    # S5 Rebote: foso de 12 m (no se cruza sin rebotar) con dos muñecos flotando a media altura.
    crear_suelo(actores, "S5_Suelo", 11500, 11800, materiales)
    crear_foso(actores, "S5_Foso", 11800, 13000, unreal.Vector(11650, 0, 100), materiales)
    # Pinchos a lo largo del foso (se rebota en ellos) y un muñeco flotando para practicar primero.
    crear_peligro(actores, "S5_Pinchos", unreal.Vector(12400, 0, -120), (600, ANCHO_PASILLO / 2, 30), unreal.Vector(11650, 0, 100),
                  materiales["Puerta"])
    crear_muneco(actores, "S5_MunecoRebote1", unreal.Vector(12200, 0, -60))
    crear_suelo(actores, "S5_Llegada", 13000, 13500, materiales)

    # S6 Alma: muñecos para llenar el alma antes de la arena.
    crear_suelo(actores, "S6_Suelo", 13500, 15000, materiales)
    crear_punto_control(actores, "Control_Alma", 13600)
    for indice, y in enumerate((-200, 200)):
        crear_muneco(actores, f"S6_Muneco{indice}", unreal.Vector(14300, y, 96), unreal.Rotator(0, 0, 180))
    # Armas de fuego: dos muñecos altos a los lados, mejor a tiros; los de abajo rellenan la reserva con la espada.
    for indice, y in enumerate((-(ANCHO_PASILLO / 2 - 80), ANCHO_PASILLO / 2 - 80)):
        crear_muneco(actores, f"S6_MunecoDiana{indice}", unreal.Vector(14850, y, 96 + 330), unreal.Rotator(0, 0, 180))

    # S7 Arena de mini enemigos; la puerta se abre al vaciarla.
    crear_arena(actores, "S7_Arena", 15000, 17500, 2000, 400, materiales, 600)
    crear_punto_control(actores, "Control_Arena", 15100)
    enemigos_arena = [
        crear_enemigo(actores, "S7_Rastrero1", datos_enemigos["DA_Enemigo_Rastrero"], unreal.Vector(16100, -400, 0)),
        crear_enemigo(actores, "S7_Rastrero2", datos_enemigos["DA_Enemigo_Rastrero"], unreal.Vector(16400, 400, 0)),
        crear_enemigo(actores, "S7_Saltador", datos_enemigos["DA_Enemigo_Saltador"], unreal.Vector(16900, 0, 0)),
        crear_enemigo(actores, "S7_Escupidor", datos_enemigos["DA_Enemigo_Escupidor"], unreal.Vector(17200, 600, 0)),
        crear_enemigo(actores, "S7_Revoloteador", datos_enemigos["DA_Enemigo_Revoloteador"], unreal.Vector(16700, -500, 0)),
    ]
    crear_puerta(actores, "S7_Puerta", 17500, 600, 400, enemigos_arena, materiales["Puerta"],
                 "Puerta abierta", "Sigue adelante.")

    # S8 Arena del Guardián.
    crear_arena(actores, "S8_Arena", 17500, 20500, 2400, 600, materiales, 600)
    crear_punto_control(actores, "Control_Jefe", 17700)
    jefe = crear_enemigo(actores, "S8_Guardian", datos_enemigos["DA_Jefe_Guardian"], unreal.Vector(19400, 0, 0))
    crear_puerta(actores, "S8_Puerta", 20500, 600, 600, [jefe], materiales["Puerta"],
                 "Tutorial completado", "Has derrotado al Guardián.\nSigue practicando: todo se combina con todo.")

    # S9 Fin.
    crear_suelo(actores, "S9_Suelo", 20500, 21500, materiales)
    crear_texto(actores, "Cartel_Fin", "Fin del tutorial", unreal.Vector(21200, 0, 300), tamano=100.0)
    portal = crear_volumen(actores, unreal.SAEVUSVolumenCambioMapa, "S9_PasoZona1", unreal.Vector(20900, ANCHO_PASILLO / 2 - 60, 150),
                           (80, 60, 200), "Tutorial")
    portal.set_editor_property("mapa", unreal.load_asset(RUTA_MAPA_ZONA1))
    crear_texto(actores, "Cartel_Zona1", "Zona 1  ->", unreal.Vector(20900, ANCHO_PASILLO / 2 - 60, 330), tamano=50.0)

    # S10 Reto opcional: obstáculos ya probados (SAEVUS.Tutorial.Obstaculos.*) encadenados sin descanso.
    crear_punto_control(actores, "Control_Reto", 21300)
    crear_foso(actores, "S10_Foso1", 21500, 22300, unreal.Vector(21350, 0, 100), materiales)
    crear_peligro(actores, "S10_Pinchos1", unreal.Vector(21900, 0, -120), (400, ANCHO_PASILLO / 2, 30), unreal.Vector(21350, 0, 100),
                  materiales["Puerta"])
    crear_suelo(actores, "S10_Suelo1", 22300, 22700, materiales)
    crear_foso(actores, "S10_Foso2", 22700, 23600, unreal.Vector(22550, 0, 100), materiales)
    crear_bloque(actores, "S10_ParedCarrera", unreal.Vector(23150, ANCHO_PASILLO / 2 + 50, 300), (1500, 100, 600), materiales["Acento"])
    crear_suelo(actores, "S10_Suelo2", 23600, 24200, materiales)
    crear_bloque(actores, "S10_Cornisa450", unreal.Vector(24500, 0, 225), (600, ANCHO_PASILLO, 450), materiales["Acento"])
    crear_texto(actores, "Cartel_RetoSuperado", "Reto superado", unreal.Vector(24600, 0, 650), tamano=90.0)

    for x, titulo, texto in AVISOS_TUTORIAL:
        crear_aviso(actores, x, titulo, texto)

    decorar_tutorial(actores, importar_decorado(materiales["Acento"]))

    crear_iluminacion_oscura(actores)
    mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mundo.get_world_settings().set_editor_property("default_game_mode", bp_modo_juego.generated_class())
    editor_niveles.save_current_level()
    registrar(f"mapa tutorial: {RUTA_MAPA_TUTORIAL}")


# --- Zona 1: Ruinas del Paso -------------------------------------------------------
RUTA_MAPA_ZONA1 = f"{RAIZ}/Mapas/L_Zona1"
# Galería del atajo: carril a la derecha del camino principal, por debajo de la planta alta.
Y_GALERIA = 650.0
ANCHO_GALERIA = 500.0
ALTURA_GALERIA = -300.0


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


def crear_caida_global(actores, etiqueta, x_inicio, x_fin, retorno):
    """Red bajo la zona: quien cae al vacío entre x_inicio y x_fin vuelve a retorno."""
    caida = crear_volumen(actores, unreal.SAEVUSVolumenCaida, etiqueta, unreal.Vector((x_inicio + x_fin) / 2.0, 0, -1500),
                          ((x_fin - x_inicio) / 2.0, 4000, 200), "Mundo")
    caida.set_editor_property("punto_retorno", retorno - caida.get_actor_location())
    return caida


def construir_mapa_zona1(bp_modo_juego, datos_enemigos, material_base):
    """Zona 1 boceto: ruinas con desniveles, tres encuentros, un banco y un atajo con palanca."""
    editor_niveles, actores = abrir_mapa_limpio(RUTA_MAPA_ZONA1)
    materiales = {nombre: crear_instancia_material(f"MI_Escenario_{nombre}", DIR_ESTILO, material_base, valores)
                  for nombre, valores in PIEZAS_ESCENARIO.items()}
    enemigo = lambda etiqueta, tipo, x, y, z: crear_enemigo(actores, etiqueta, datos_enemigos[tipo], unreal.Vector(x, y, z))

    # Z0 Entrada.
    crear_suelo(actores, "Z0_Suelo", -500, 1200, materiales)
    actores.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100)).set_actor_label("Inicio")
    crear_texto(actores, "Cartel_Zona1", "Ruinas del Paso", unreal.Vector(1100, 0, 450), tamano=120.0)
    crear_aviso(actores, 100, "Ruinas del Paso", "Zona 1 (boceto). Las ruinas bajan hacia el este.\nMás adelante, un banco guarda tu avance.")

    # Z1 Descenso en escalones de 1 m y primer encuentro abajo.
    crear_suelo(actores, "Z1_Escalon1", 1200, 1600, materiales, altura=-100)
    crear_suelo(actores, "Z1_Escalon2", 1600, 2000, materiales, altura=-200)
    crear_suelo(actores, "Z1_Suelo", 2000, 4600, materiales, altura=-300)
    enemigo("Z1_Rastrero1", "DA_Enemigo_Rastrero", 3300, -250, -300)
    enemigo("Z1_Rastrero2", "DA_Enemigo_Rastrero", 3800, 250, -300)
    enemigo("Z1_Revoloteador", "DA_Enemigo_Revoloteador", 4100, 0, -300)

    # Z2 Grieta: foso con dos plataformas.
    crear_foso(actores, "Z2_Grieta", 4600, 5500, unreal.Vector(4450, 0, -200), materiales, altura=-300)
    for indice, x in enumerate((4850, 5250)):
        crear_bloque(actores, f"Z2_Plataforma{indice}", unreal.Vector(x, 0, -350), (200, 300, 100), materiales["Acento"])

    # Z3 Banco: suelo ancho que llega hasta el carril de la galería (cerrado por la compuerta del atajo).
    ancho_banco = ANCHO_PASILLO / 2 + Y_GALERIA + ANCHO_GALERIA / 2
    crear_bloque(actores, "Z3_Suelo", unreal.Vector(6000, (Y_GALERIA + ANCHO_GALERIA / 2 - ANCHO_PASILLO / 2) / 2, -300 - GROSOR_SUELO / 2),
                 (1000, ancho_banco, GROSOR_SUELO), materiales["Piedra"])
    crear_punto_control(actores, "Control_Banco", 5700, z=-300)
    crear_bloque(actores, "Z3_Banco", unreal.Vector(5950, -280, -280), (220, 70, 40), materiales["Acento"])
    crear_texto(actores, "Cartel_Banco", "Banco", unreal.Vector(5950, -280, -120), tamano=50.0)
    crear_aviso(actores, 5550, "Banco", "Los bancos guardan tu avance: si caes o mueres, vuelves aquí.\n"
                "A la derecha, una compuerta cerrada: se abre desde el otro lado.", altura=-300)

    # Z4 Torre: muro de 4,5 m que se escala y planta alta con el segundo encuentro.
    crear_bloque(actores, "Z4_Muro", unreal.Vector(6800, 0, -75), (600, ANCHO_PASILLO, 450), materiales["Acento"])
    crear_suelo(actores, "Z4_PlantaAlta", 7100, 9600, materiales, altura=150)
    enemigo("Z4_Saltador", "DA_Enemigo_Saltador", 8200, 0, 150)
    enemigo("Z4_Revoloteador", "DA_Enemigo_Revoloteador", 8700, -200, 150)
    enemigo("Z4_Escupidor", "DA_Enemigo_Escupidor", 9300, 250, 150)

    # Z5 Arena de las ruinas: la puerta se abre al vaciarla.
    crear_punto_control(actores, "Control_Arena", 9700, z=150)
    crear_arena(actores, "Z5_Arena", 9600, 12000, 2000, 400, materiales, 600, altura=150)
    enemigos_arena = [
        enemigo("Z5_Rastrero1", "DA_Enemigo_Rastrero", 10400, -500, 150),
        enemigo("Z5_Rastrero2", "DA_Enemigo_Rastrero", 10700, 500, 150),
        enemigo("Z5_Saltador", "DA_Enemigo_Saltador", 11000, 0, 150),
        enemigo("Z5_Escupidor", "DA_Enemigo_Escupidor", 11500, 600, 150),
        enemigo("Z5_Revoloteador1", "DA_Enemigo_Revoloteador", 10800, -300, 150),
        enemigo("Z5_Revoloteador2", "DA_Enemigo_Revoloteador", 11300, 300, 150),
    ]
    crear_puerta(actores, "Z5_Puerta", 12000, 600, 400, enemigos_arena, materiales["Puerta"],
                 "Ruinas despejadas", "Sigue: a la derecha de la salida se baja a la galería del atajo.", altura=150)

    # Z6 Antesala; por la derecha se cae a la galería.
    crear_suelo(actores, "Z6_Suelo", 12000, 13000, materiales, altura=150)
    crear_aviso(actores, 12100, "Antesala", "A la derecha, una bajada lleva a la galería del atajo.\nAdelante espera la Madre Alada.", altura=150)

    # Z7 Arena de la Madre Alada (jefe volador); su puerta lleva al final.
    crear_punto_control(actores, "Control_Jefe", 13100, z=150)
    crear_arena(actores, "Z7_Arena", 13000, 16000, 2400, 700, materiales, 600, altura=150)
    jefe = enemigo("Z7_MadreAlada", "DA_Jefe_MadreAlada", 15000, 0, 150)
    crear_puerta(actores, "Z7_Puerta", 16000, 600, 700, [jefe], materiales["Puerta"],
                 "Zona 1 completada", "Has derrotado a la Madre Alada.", altura=150)

    # Z8 Fin.
    crear_suelo(actores, "Z8_Suelo", 16000, 17000, materiales, altura=150)
    crear_texto(actores, "Cartel_Fin", "Fin de la Zona 1 (boceto)", unreal.Vector(16850, 0, 450), tamano=80.0)

    # Galería del atajo: de la salida al banco, por debajo; la compuerta junto al banco solo la abre su palanca.
    largo_galeria = 13000 - 6500
    crear_bloque(actores, "Galeria_Suelo", unreal.Vector(6500 + largo_galeria / 2, Y_GALERIA, ALTURA_GALERIA - GROSOR_SUELO / 2),
                 (largo_galeria, ANCHO_GALERIA, GROSOR_SUELO), materiales["Piedra"])
    alto_galeria = 150 - ALTURA_GALERIA
    crear_bloque(actores, "Galeria_MuroExterior", unreal.Vector(6500 + largo_galeria / 2, Y_GALERIA + ANCHO_GALERIA / 2 + 50, ALTURA_GALERIA + alto_galeria / 2),
                 (largo_galeria, 100, alto_galeria), materiales["Acento"])
    # Muro interior bajo la planta alta y la arena (antes, el muro de la torre ya cierra el carril).
    crear_bloque(actores, "Galeria_MuroInterior", unreal.Vector((7100 + 12000) / 2, ANCHO_PASILLO / 2 - 50, ALTURA_GALERIA + (50 - ALTURA_GALERIA) / 2),
                 (12000 - 7100, 100, 50 - ALTURA_GALERIA), materiales["Acento"])
    crear_bloque(actores, "Galeria_Fondo", unreal.Vector(13050, Y_GALERIA, ALTURA_GALERIA + alto_galeria / 2), (100, ANCHO_GALERIA, alto_galeria), materiales["Acento"])
    compuerta = crear_puerta(actores, "Galeria_Compuerta", 6550, ANCHO_GALERIA, 350, [], materiales["Puerta"], "", "",
                             altura=ALTURA_GALERIA, y=Y_GALERIA)
    crear_palanca(actores, "Galeria_Palanca", unreal.Vector(6750, Y_GALERIA, ALTURA_GALERIA), [compuerta], materiales,
                  "Atajo abierto", "La compuerta lleva al banco.")

    # Redes bajo el vacío: antes del banco se vuelve a la entrada; después, al banco.
    crear_caida_global(actores, "Caida_Entrada", -1000, 5500, unreal.Vector(300, 0, 100))
    crear_caida_global(actores, "Caida_Banco", 5500, 18000, unreal.Vector(5700, 0, -200))

    # Decorado: columnas rotas a los lados y arcos en los cambios de tramo.
    mallas = importar_decorado(materiales["Acento"])
    for x, z in ((1200, -100), (4600, -300), (7100, 150), (12000, 150)):
        crear_decorado(actores, f"Arco_{x}", mallas["SM_ArcoSeccion"], unreal.Vector(x, 0, z), guinada=90.0)
    for indice, x in enumerate(range(500, 13000, 1400)):
        crear_decorado(actores, f"Columna_I{x}", mallas["SM_ColumnaRota"], unreal.Vector(x + (indice % 2) * 250, -650, -450),
                       guinada=(indice * 53) % 360, escala=1.4)
    for x, y, z in ((2600, -300, -300), (3900, 320, -300), (6200, -320, -300), (7600, 300, 150), (12600, -300, 150)):
        crear_decorado(actores, f"Escombros_{x}", mallas["SM_Escombros"], unreal.Vector(x, y, z), guinada=(x * 7) % 360)

    crear_iluminacion_oscura(actores)
    mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mundo.get_world_settings().set_editor_property("default_game_mode", bp_modo_juego.generated_class())
    editor_niveles.save_current_level()
    registrar(f"mapa zona 1: {RUTA_MAPA_ZONA1}")


def main():
    acciones, imc = crear_assets_entrada()
    config_entrada, movimiento, camara, repertorio, armas = crear_data_assets(acciones)
    bp_modo_juego = crear_blueprints(acciones, imc, config_entrada, movimiento, camara, repertorio, armas)
    datos_enemigos = crear_datos_enemigos(
        cargar(f"{DIR_ESTILO}/M_SAEVUS_PersonajeDibujado"), cargar(f"{DIR_MATERIALES}/M_ParticulaAlma"))
    construir_mapa_pruebas(bp_modo_juego, datos_enemigos)
    # La Zona 1 antes que el tutorial: el paso del final del tutorial apunta a ella.
    construir_mapa_zona1(bp_modo_juego, datos_enemigos, cargar(f"{DIR_ESTILO}/M_SAEVUS_PersonajeDibujado"))
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
