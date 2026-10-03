"""
Comprueba que los assets de SAEVUS existen y que los valores por defecto de los Blueprints que
fija saevus_crear_assets.py sobrevivieron al guardado (proceso de editor nuevo, así los valores
vienen del disco). Escribe una línea por comprobación y "[SAEVUS verificar] SUPERADA" o
"[SAEVUS verificar] FALLIDA".

Ejecución sin interfaz:
  UnrealEditor-Cmd.exe SAEVUS.uproject -ExecutePythonScript="<este archivo>" -unattended -nullrhi
"""

import unreal

eal = unreal.EditorAssetLibrary
fallos = []

ASSETS_OBLIGATORIOS = (
    "/Game/SAEVUS/Entrada/IMC_SAEVUS",
    "/Game/SAEVUS/Entrada/Acciones/IA_Esprintar",
    "/Game/SAEVUS/Entrada/Acciones/IA_Caminar",
    "/Game/SAEVUS/Entrada/Acciones/IA_Dash",
    "/Game/SAEVUS/Entrada/Acciones/IA_Atacar",
    "/Game/SAEVUS/Entrada/Acciones/IA_Recargar",
    "/Game/SAEVUS/Entrada/Acciones/IA_SiguienteArma",
    "/Game/SAEVUS/Entrada/Acciones/IA_AlternarHUD",
    "/Game/SAEVUS/Datos/DA_AjustesMovimiento_Base",
    "/Game/SAEVUS/Datos/DA_AjustesCamara_Base",
    "/Game/SAEVUS/Datos/DA_RepertorioMelee_Base",
    "/Game/SAEVUS/Datos/DA_Arma_Espada",
    "/Game/SAEVUS/Datos/DA_Arma_Pistola",
    "/Game/SAEVUS/Datos/DA_Arma_Rifle",
    "/Game/SAEVUS/Datos/DA_Arma_Escopeta",
    "/Game/SAEVUS/Mapas/L_Pruebas",
    "/Game/SAEVUS/Arte/Materiales/M_ParticulaPolvo",
)

PROPIEDADES_CONFIG_ENTRADA = (
    "accion_mover", "accion_mirar", "accion_mirar_raton", "accion_saltar", "accion_esprint", "accion_caminar",
    "accion_dash", "accion_atacar", "accion_recargar", "accion_siguiente_arma",
)


def comprobar(descripcion, condicion):
    if condicion:
        unreal.log(f"[SAEVUS verificar] ok    {descripcion}")
    else:
        unreal.log_error(f"[SAEVUS verificar] FALLO {descripcion}")
        fallos.append(descripcion)


def cdo(ruta):
    blueprint = eal.load_asset(ruta)
    comprobar(f"existe {ruta}", blueprint is not None)
    return unreal.get_default_object(blueprint.generated_class()) if blueprint else None


def main():
    for ruta in ASSETS_OBLIGATORIOS:
        comprobar(f"existe {ruta}", eal.does_asset_exist(ruta))

    config_entrada = eal.load_asset("/Game/SAEVUS/Datos/DA_ConfigEntrada")
    comprobar("existe DA_ConfigEntrada", config_entrada is not None)
    if config_entrada:
        for propiedad in PROPIEDADES_CONFIG_ENTRADA:
            comprobar(f"DA_ConfigEntrada.{propiedad} asignada", config_entrada.get_editor_property(propiedad) is not None)
        comprobar("4 acciones de ranura de arma", len(config_entrada.get_editor_property("acciones_ranura_arma")) == 4)

    personaje = cdo("/Game/SAEVUS/Blueprints/BP_SAEVUSPersonaje")
    if personaje:
        malla = personaje.get_editor_property("mesh")
        comprobar("malla del personaje asignada", malla.get_editor_property("skeletal_mesh_asset") is not None)
        comprobar("clase de animación asignada", malla.get_editor_property("anim_class") is not None)
        comprobar("config de entrada asignada", personaje.get_editor_property("config_entrada") is not None)
        movimiento = personaje.get_editor_property("character_movement")
        comprobar("el movimiento es de SAEVUS", isinstance(movimiento, unreal.SAEVUSComponenteMovimiento))
        comprobar("ajustes de movimiento asignados", movimiento.get_editor_property("ajustes_movimiento") is not None)
        comprobar("ajustes de cámara asignados", personaje.get_editor_property("camara").get_editor_property("ajustes_camara") is not None)
        combate = personaje.get_editor_property("combate")
        comprobar("repertorio de combate asignado", combate is not None and combate.get_editor_property("datos_repertorio") is not None)
        armas = personaje.get_editor_property("armas")
        arsenal = armas.get_editor_property("arsenal") if armas else []
        comprobar("el arsenal tiene 4 armas", len(arsenal) == 4 and all(arsenal))
        estela = personaje.get_editor_property("estela_dash")
        comprobar("material de la estela del dash asignado", estela is not None and len(estela.get_editor_property("override_materials")) > 0)

    controlador = cdo("/Game/SAEVUS/Blueprints/BP_SAEVUSControladorJugador")
    if controlador:
        comprobar("el controlador tiene 3 contextos de entrada", len(controlador.get_editor_property("contextos_entrada")) == 3)
        comprobar("acción de alternar HUD asignada", controlador.get_editor_property("accion_alternar_hud") is not None)

    modo_juego = cdo("/Game/SAEVUS/Blueprints/BP_SAEVUSModoJuego")
    if modo_juego:
        clase_pawn = modo_juego.get_editor_property("default_pawn_class")
        comprobar("el pawn del modo de juego es BP_SAEVUSPersonaje", clase_pawn is not None and "BP_SAEVUSPersonaje" in clase_pawn.get_name())

    if fallos:
        unreal.log_error(f"[SAEVUS verificar] FALLIDA ({len(fallos)} comprobaciones)")
    else:
        unreal.log("[SAEVUS verificar] SUPERADA")


try:
    main()
except Exception as error:  # noqa: BLE001 - mostrar cualquier fallo en el log y salir igualmente
    unreal.log_error(f"[SAEVUS verificar] FALLIDA: {error}")
finally:
    unreal.SystemLibrary.quit_editor()
