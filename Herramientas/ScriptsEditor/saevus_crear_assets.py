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

# Colores del caballero boceto por ranura de material (nombres de material de Blender).
# Tono Hollow Knight: cuerpo casi negro, máscara clara, capa azul oscuro, metal pálido.
COLORES_CABALLERO = {
    "Cuerpo": (0.02, 0.02, 0.025, 1.0),
    "Mascara": (0.85, 0.85, 0.82, 1.0),
    "Capa": (0.04, 0.05, 0.08, 1.0),
    "Metal": (0.55, 0.58, 0.62, 1.0),
    "Ojos": (0.0, 0.0, 0.0, 1.0),
}
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
    "IA_SiguienteArma": ["MouseScrollDown", "Gamepad_RightShoulder"],
    "IA_RanuraArma1": ["One"],
    "IA_RanuraArma2": ["Two"],
    "IA_RanuraArma3": ["Three"],
    "IA_RanuraArma4": ["Four"],
    # F1-F5, F9 y F11 son atajos de depuración del motor (BaseInput.ini).
    "IA_AlternarHUD": ["F6"],
}

# Arsenal, en orden de ranura (teclas 1-4). Valores de primera pasada; se ajustan en los Data Assets.
# Los campos de arma de fuego son propiedades de FSAEVUSAjusteArmaFuego.
ARMAS = {
    "DA_Arma_Espada": {"nombre_visible": "Espada", "tipo": "cuerpo_a_cuerpo", "tiempo_equipar": 0.2},
    "DA_Arma_Pistola": {
        "nombre_visible": "Pistola", "tipo": "arma_fuego", "tiempo_equipar": 0.25,
        "arma_fuego": {"danio": 20.0, "intervalo_disparo": 0.25, "automatica": False, "tamano_cargador": 12,
                       "tiempo_recarga": 1.2, "alcance": 5000.0, "dispersion_grados": 0.5, "perdigones_por_disparo": 1},
    },
    "DA_Arma_Rifle": {
        "nombre_visible": "Rifle", "tipo": "arma_fuego", "tiempo_equipar": 0.4,
        "arma_fuego": {"danio": 9.0, "intervalo_disparo": 0.1, "automatica": True, "tamano_cargador": 30,
                       "tiempo_recarga": 1.8, "alcance": 6000.0, "dispersion_grados": 1.5, "perdigones_por_disparo": 1},
    },
    "DA_Arma_Escopeta": {
        "nombre_visible": "Escopeta", "tipo": "arma_fuego", "tiempo_equipar": 0.45,
        "arma_fuego": {"danio": 7.0, "intervalo_disparo": 0.8, "automatica": False, "tamano_cargador": 6,
                       "tiempo_recarga": 2.0, "alcance": 2000.0, "dispersion_grados": 6.0, "perdigones_por_disparo": 8},
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

    materiales = malla.get_editor_property("materials")
    for ranura in materiales:
        nombre_ranura = str(ranura.get_editor_property("material_slot_name"))
        for clave, color in COLORES_CABALLERO.items():
            if nombre_ranura.lower().startswith(clave.lower()):
                ranura.set_editor_property("material_interface", crear_material_color(f"M_Caballero_{clave}", color, directorio=DIR_CABALLERO))
    malla.set_editor_property("materials", materiales)
    registrar(f"caballero importado: {malla.get_path_name()} ({len(materiales)} materiales)")
    return malla


def crear_assets_entrada():
    acciones = {}
    for nombre in ASIGNACIONES_SAEVUS:
        accion = obtener_o_crear(nombre, f"{DIR_ENTRADA}/Acciones", unreal.InputAction, unreal.InputAction_Factory())
        accion.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
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
    config_entrada.set_editor_property("accion_siguiente_arma", acciones["IA_SiguienteArma"])
    config_entrada.set_editor_property(
        "acciones_ranura_arma", [acciones[f"IA_RanuraArma{ranura}"] for ranura in range(1, len(ARMAS) + 1)]
    )

    # Los valores se quedan en los del código hasta ajustarlos en el editor.
    movimiento = data_asset("DA_AjustesMovimiento_Base", unreal.SAEVUSAjustesMovimiento)
    camara = data_asset("DA_AjustesCamara_Base", unreal.SAEVUSAjustesCamara)
    repertorio = data_asset("DA_RepertorioMelee_Base", unreal.SAEVUSDatosRepertorioMelee)
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
    personaje.get_editor_property("character_movement").set_editor_property("ajustes_movimiento", movimiento)
    personaje.get_editor_property("camara").set_editor_property("ajustes_camara", camara)
    personaje.get_editor_property("combate").set_editor_property("datos_repertorio", repertorio)
    personaje.get_editor_property("armas").set_editor_property("arsenal", armas)
    # Polvo claro y frío para la estela del dash: resalta sobre el escenario oscuro.
    material_estela = crear_material_color("M_ParticulaPolvo", (0.75, 0.78, 0.85, 1.0), sin_luz=True)
    personaje.get_editor_property("estela_dash").set_editor_property("override_materials", [material_estela])

    bp_controlador = obtener_o_crear_blueprint("BP_SAEVUSControladorJugador", unreal.SAEVUSControladorJugador)
    controlador = cdo_blueprint(bp_controlador)
    controlador.set_editor_property("contextos_entrada", [cargar(IMC_PLANTILLA), cargar(IMC_PLANTILLA_RATON), imc])
    controlador.set_editor_property("accion_alternar_hud", acciones["IA_AlternarHUD"])

    bp_modo_juego = obtener_o_crear_blueprint("BP_SAEVUSModoJuego", unreal.SAEVUSModoJuego)
    modo_juego = cdo_blueprint(bp_modo_juego)
    modo_juego.set_editor_property("default_pawn_class", bp_personaje.generated_class())
    modo_juego.set_editor_property("player_controller_class", bp_controlador.generated_class())

    for blueprint in (bp_personaje, bp_controlador, bp_modo_juego):
        unreal.BlueprintEditorLibrary.compile_blueprint(blueprint)
    return bp_modo_juego


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


def construir_mapa_pruebas(bp_modo_juego):
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

    crear_iluminacion(actores)
    actores.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100)).set_actor_label("Inicio")

    mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mundo.get_world_settings().set_editor_property("default_game_mode", bp_modo_juego.generated_class())
    editor_niveles.save_current_level()


def main():
    acciones, imc = crear_assets_entrada()
    config_entrada, movimiento, camara, repertorio, armas = crear_data_assets(acciones)
    bp_modo_juego = crear_blueprints(acciones, imc, config_entrada, movimiento, camara, repertorio, armas)
    construir_mapa_pruebas(bp_modo_juego)
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
