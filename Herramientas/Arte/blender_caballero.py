"""
Genera el protagonista de SAEVUS en Blender: malla de piezas + esqueleto propio, y lo exporta como
FBX (y guarda el .blend para editarlo a mano).

Diseño: el de las referencias canónicas REF-01, REF-02 y REF-03 (Documentos/REFERENCIAS_ARTE.md,
Documentos/ARTE_SAEVUS.md, Protagonista). Humanoide esbelto (cabeza ≈ 1/8 de la altura), casco de
placas oscuras en forma de hoja con dos aletas barridas hacia atrás, máscara de hueso en V con ojos
cian, capa verde pizarra con capucha y tiras dentadas cerrada con un broche de anillo, correas en X,
cinturón, faldón, brazales de hueso, pantalón holgado, espinillas vendadas, pies de tres garras y una
espada larga de un filo con guarda de anillo. Geometría solo donde cambia la silueta (REF-02,
"formas simples y claras, grandes volúmenes, detalles controlados"); el resto lo pone el material.

Piezas rígidas (peso 1 a un hueso) salvo la capa y el faldón, que reparten el peso por altura entre
varios huesos para doblarse con suavidad (REF-02: capa "deformable, movimiento secundario").
Mismos 25 huesos que el boceto anterior (ESAEVUSHueso en C++); el nombre interno sigue siendo
"Caballero" (ARTE_SAEVUS.md, Decisiones).

Convenciones (Documentos/pipeline-assets.md): metros, el personaje mira a -Y en Blender (llega
mirando a +Y en UE, como el maniquí), origen en los pies. Derecha del personaje = -X en Blender.

Ejecución sin interfaz (Generar-Caballero.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_caballero.py -- <directorio de salida>
"""

import math
import os
import random
import sys

import bpy
from mathutils import Matrix, Vector

NOMBRE_MALLA = "SK_Caballero"
SEMILLA = 7  # las tiras dentadas usan azar con semilla fija: el modelo sale siempre igual

# Colores (lineales) medidos en las referencias (ARTE_SAEVUS.md, Paleta). Se repiten en los materiales
# de UE (saevus_crear_assets.py, PIEZAS_CABALLERO); aquí solo sirven para verlo en Blender.
COLORES = {
    "Cuerpo": (0.014, 0.013, 0.015, 1.0),   # cuerpo oscuro #1F1E21
    "Casco": (0.027, 0.024, 0.028, 1.0),    # placas del casco #2E2B2F
    "Hueso": (0.24, 0.18, 0.16, 1.0),       # máscara, brazales, escamas #877771
    "Capa": (0.033, 0.040, 0.042, 1.0),     # verde pizarra #33393A
    "Cuero": (0.048, 0.029, 0.025, 1.0),    # cinturón, correas, interior de las aletas #3E302C
    "Tela": (0.024, 0.021, 0.023, 1.0),     # pantalón #2B282A
    "Metal": (0.075, 0.097, 0.12, 1.0),     # acero de la hoja, anillos #4E5862
    "Filo": (0.21, 0.43, 0.63, 1.0),        # filo azul de la espada #7FB0D0
    "Ojos": (0.16, 0.65, 0.79, 1.0),        # ojos cian #6FD3E6
}

# Proporciones (ARTE_SAEVUS.md, medidas en REF-01 frente y REF-02): altura total H = 1,92 m.
ALTURA_BARBILLA = 1.69
ALTURA_HOMBRO = 1.51
ALTURA_CINTURON = 1.27
ALTURA_MUNECA = 1.02
ALTURA_CADERA = 0.96
ALTURA_RODILLA = 0.52
ALTURA_TOBILLO = 0.15
LARGO_HOJA = 0.88      # ≈ 0,46 H (REF-02, REF-04 n.º 1); C++: FSAEVUSAjusteAnimacion::LongitudHoja
LARGO_PUNO = 0.16

# Huesos: (nombre, padre, cabeza, cola). Pose de reposo en "A": brazos un poco abiertos y hacia abajo.
D = -1.0  # lado derecho del personaje (-X)
I = 1.0   # lado izquierdo (+X)
GUARDA = (D * 0.285, -0.06, 0.90)  # guarda de la espada, bajo el puño cerrado y algo delante de la muñeca
HUESOS = [
    ("raiz", None, (0, 0, 0), (0, 0, 0.12)),
    ("pelvis", "raiz", (0, 0, ALTURA_CADERA), (0, 0, 1.06)),
    ("columna", "pelvis", (0, 0, 1.06), (0, 0, 1.28)),
    ("pecho", "columna", (0, 0, 1.28), (0, 0, 1.50)),
    ("cuello", "pecho", (0, 0, 1.50), (0, 0, 1.62)),
    ("cabeza", "cuello", (0, 0, 1.62), (0, 0, 1.92)),
    ("capa_1", "pecho", (0, 0.12, 1.47), (0, 0.17, 1.12)),
    ("capa_2", "capa_1", (0, 0.17, 1.12), (0, 0.21, 0.80)),
    # Tercer tramo de la capa: no lo anima nadie, sigue a capa_2.
    ("capa_3", "capa_2", (0, 0.21, 0.80), (0, 0.24, 0.50)),
]
for lado, s in (("d", D), ("i", I)):
    HUESOS += [
        (f"hombro_{lado}", "pecho", (s * 0.06, 0, ALTURA_HOMBRO), (s * 0.19, 0, ALTURA_HOMBRO)),
        (f"brazo_{lado}", f"hombro_{lado}", (s * 0.19, 0, ALTURA_HOMBRO), (s * 0.245, 0.01, 1.25)),
        (f"antebrazo_{lado}", f"brazo_{lado}", (s * 0.245, 0.01, 1.25), (s * 0.275, -0.02, ALTURA_MUNECA)),
        (f"mano_{lado}", f"antebrazo_{lado}", (s * 0.275, -0.02, ALTURA_MUNECA), (s * 0.285, -0.035, 0.93)),
        (f"muslo_{lado}", "pelvis", (s * 0.095, 0, ALTURA_CADERA), (s * 0.11, 0, ALTURA_RODILLA)),
        (f"pierna_{lado}", f"muslo_{lado}", (s * 0.11, 0, ALTURA_RODILLA), (s * 0.11, 0.03, ALTURA_TOBILLO)),
        (f"pie_{lado}", f"pierna_{lado}", (s * 0.11, 0.03, ALTURA_TOBILLO), (s * 0.11, -0.15, 0.02)),
    ]
# Armas en la mano derecha: la espada cuelga hacia abajo en reposo (su hueso empieza en la guarda,
# como mide la estela en C++); la pistola apunta al frente (retirada del juego, se conserva el hueso).
HUESOS += [
    ("arma", "mano_d", GUARDA, (GUARDA[0], GUARDA[1], GUARDA[2] - LARGO_HOJA)),
    ("arma_fuego", "mano_d", (D * 0.285, -0.05, 0.98), (D * 0.285, -0.32, 0.98)),
]

# Ayudantes de modelado compartidos (misma carpeta).
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blender_piezas import *  # noqa: E402,F403

colores_materiales(COLORES)

# --- Pesos repartidos ---------------------------------------------------------------------

TRAMOS_CAPA = [(1.44, "pecho"), (1.26, "capa_1"), (0.96, "capa_2"), (0.62, "capa_3")]


def pesos_capa(posicion):
    return interpolar_por_altura(posicion.z, TRAMOS_CAPA)


def pesos_faldon(lado):
    """El faldón cuelga de la pelvis y, hacia abajo, sigue al muslo de su lado (no lo atraviesa al correr)."""
    tramos = [(1.12, "pelvis"), (0.70, f"muslo_{lado}")]
    return lambda posicion: interpolar_por_altura(posicion.z, tramos)

# --- El personaje ------------------------------------------------------------------------

def cabeza():
    """Casco de placas en hoja con aletas, máscara de hueso en V y ojos cian (REF-01, cabeza)."""
    piezas = [elipsoide("cabeza", "Casco", (0, 0.01, 1.775), (0.072, 0.088, 0.098))]
    adelante = Vector((0, -1, 0))
    # Máscara: rombo central de la barbilla (pico) a la frente, placas de mejilla y de ceja.
    piezas.append(lamina("cabeza", "Hueso", (0, -0.085, ALTURA_BARBILLA), (0, 0.32, 1), (1, 0, 0), 0.20, 0.075, 0.022,
                         perfil_rombo, curva=-22, eje_curva=(1, 0, 0)))
    for s in (D, I):
        piezas.append(lamina("cabeza", "Hueso", (s * 0.012, -0.08, 1.705), (s * 0.55, 0.45, 0.70), (0, 0, 1), 0.12, 0.045, 0.016,
                             perfil_hoja, curva=-20, eje_curva=(0, 0, s)))
        piezas.append(lamina("cabeza", "Hueso", (s * 0.012, -0.088, 1.80), (s * 0.85, 0.35, 0.40), (0, 0, 1), 0.105, 0.032, 0.014,
                             perfil_hoja, curva=-25, eje_curva=(0, 0, s)))
        # Ojos rasgados, con el rabillo hacia arriba y afuera.
        ojo = elipsoide("cabeza", "Ojos", (0, 0, 0), (0.019, 0.006, 0.0085), 10, 6)
        ojo.matrix_world = Matrix.Translation((s * 0.036, -0.083, 1.772)) @ Matrix.Rotation(math.radians(-18 * s), 4, "Y")
        piezas.append(ojo)
    # Cresta de placas oscuras que suben y se abren hacia atrás.
    placas = [
        ((0, -0.03, 1.855), (0, 0.15, 1), 0.12, 0.055, -30),
        ((-0.028, -0.015, 1.85), (-0.35, 0.35, 1), 0.13, 0.05, -25),
        ((0.028, -0.015, 1.85), (0.35, 0.35, 1), 0.13, 0.05, -25),
        ((-0.05, 0.015, 1.83), (-0.65, 0.55, 0.8), 0.12, 0.045, -20),
        ((0.05, 0.015, 1.83), (0.65, 0.55, 0.8), 0.12, 0.045, -20),
        ((-0.022, 0.06, 1.835), (-0.2, 1, 0.75), 0.15, 0.05, 15),
        ((0.022, 0.06, 1.835), (0.2, 1, 0.75), 0.15, 0.05, 15),
        ((0, 0.085, 1.79), (0, 1, 0.25), 0.13, 0.05, 20),
    ]
    for base, direccion, largo, ancho, curva in placas:
        piezas.append(lamina("cabeza", "Casco", base, direccion, adelante.cross(Vector(direccion)).normalized()
                             if abs(Vector(direccion).normalized().y) < 0.9 else (1, 0, 0), largo, ancho, 0.018, perfil_hoja,
                             curva=curva, segmentos=5))
    # Aletas: grandes, barridas hacia atrás y afuera, con la punta hacia arriba; interior de cuero (marrón).
    for s in (D, I):
        direccion = Vector((s * 0.80, 0.55, 0.50)).normalized()
        lateral = direccion.cross(adelante).normalized()
        base = Vector((s * 0.062, 0.02, 1.79))
        piezas.append(lamina("cabeza", "Casco", base, direccion, lateral, 0.30, 0.088, 0.014, perfil_hoja,
                             curva=28, eje_curva=adelante * s, segmentos=7))
        piezas.append(lamina("cabeza", "Cuero", base + Vector((0, -0.008, 0)), direccion, lateral, 0.25, 0.06, 0.006,
                             perfil_hoja, curva=28, eje_curva=adelante * s, segmentos=7))
    return piezas


def torso():
    piezas = [
        tubo("cuello", "Cuerpo", [(0, 0.005, 1.47), (0, 0.01, 1.66)], [0.042, 0.036]),
        # Cintura estrecha y pecho más ancho, aplanados de delante a atrás.
        tubo("columna", "Cuerpo", [(0, 0, 1.04), (0, 0, 1.17), (0, 0, 1.30)], [0.105, 0.098, 0.115], 12, 0.75),
        tubo("pecho", "Cuerpo", [(0, 0, 1.27), (0, 0, 1.40), (0, 0.005, 1.52)], [0.12, 0.145, 0.13], 12, 0.68),
        # Escamas de hueso en la abertura en V del pecho (REF-01).
        lamina("pecho", "Hueso", (0, -0.088, 1.27), (0, -0.12, 1), (1, 0, 0), 0.24, 0.14, 0.012,
               lambda t: max(0.08, t ** 0.7), segmentos=4),
        elipsoide("pelvis", "Tela", (0, 0.0, 1.0), (0.13, 0.095, 0.10)),
        # Cinturón ancho, segunda correa baja y hebilla.
        tubo("columna", "Cuero", [(0, 0, 1.235), (0, 0, 1.30)], [0.122, 0.122], 14, 0.78),
        tubo("pelvis", "Cuero", [(0, 0.0, 1.17), (0, 0.0, 1.205)], [0.13, 0.13], 14, 0.80),
        anillo_metalico("columna", "Metal", (0.04, -0.098, 1.265), (0, -1, 0), 0.018, 0.005),
    ]
    # Correas cruzadas en X sobre el pecho, pegadas a su superficie (elipse del tubo del pecho).
    for s in (D, I):
        puntos = []
        for k in range(7):
            f = k / 6
            x, z = s * 0.115 * (1 - 2 * f), 1.28 + 0.22 * f
            radio_x = 0.12 + 0.025 * (z - 1.27) / 0.13 if z < 1.40 else 0.145 - 0.015 * (z - 1.40) / 0.12
            puntos.append((x, -0.68 * radio_x * math.sqrt(max(0.0, 1 - (x / radio_x) ** 2)) - 0.006, z))
        piezas.append(tubo("pecho", "Cuero", puntos, [0.004] * 7, 6, 4.0, Vector((0, 1, 0))))
    # Faldón: dos paneles delante y dos detrás, del cinturón a la rodilla, con los bajos rotos.
    azar = random.Random(SEMILLA)
    for s, lado in ((D, "d"), (I, "i")):
        for y, inclinacion in ((-0.11, -0.10), (0.10, 0.14)):
            piezas.append(lamina(pesos_faldon(lado), "Capa", (s * 0.065, y, 1.24), (s * 0.08, inclinacion, -1), (1, 0, 0),
                                 0.68 + azar.uniform(-0.06, 0.04), 0.10, 0.012, perfil_tira,
                                 desvio_punta=azar.uniform(-0.04, 0.04), segmentos=6))
    return piezas


def brazos():
    piezas = []
    for lado, s in (("d", D), ("i", I)):
        piezas.append(elipsoide(f"hombro_{lado}", "Cuerpo", (s * 0.17, 0, ALTURA_HOMBRO - 0.01), (0.045, 0.042, 0.042)))
        piezas.append(tubo(f"brazo_{lado}", "Cuerpo", [(s * 0.19, 0, ALTURA_HOMBRO), (s * 0.245, 0.01, 1.25)], [0.04, 0.032]))
        piezas.append(tubo(f"antebrazo_{lado}", "Cuerpo", [(s * 0.245, 0.01, 1.25), (s * 0.275, -0.02, ALTURA_MUNECA)], [0.032, 0.026]))
        # Brazal de hueso segmentado y placa en punta en el codo.
        piezas.append(tubo(f"antebrazo_{lado}", "Hueso", [(s * 0.25, 0.005, 1.215), (s * 0.262, -0.007, 1.13), (s * 0.274, -0.019, 1.04)],
                           [0.04, 0.043, 0.032], 8))
        piezas.append(lamina(f"antebrazo_{lado}", "Hueso", (s * 0.255, 0.03, 1.22), (s * 0.25, 1, 0.55), (0, 0, 1), 0.07, 0.035, 0.012,
                             perfil_pua, segmentos=3))
        # Mano oscura con cuatro garras.
        piezas.append(elipsoide(f"mano_{lado}", "Cuerpo", (s * 0.282, -0.03, 0.975), (0.026, 0.034, 0.042), 10, 7))
        for k in range(4):
            y = -0.055 + 0.016 * k
            piezas.append(lamina(f"mano_{lado}", "Cuerpo", (s * 0.29, y, 0.955), (s * 0.15, -0.35, -1), (0, 1, 0), 0.075, 0.012, 0.012,
                                 perfil_pua, curva=-30, eje_curva=(s, 0, 0), segmentos=3))
    return piezas


def piernas():
    piezas = []
    for lado, s in (("d", D), ("i", I)):
        # Pantalón holgado hasta la rodilla.
        piezas.append(tubo(f"muslo_{lado}", "Tela", [(s * 0.095, 0, 1.0), (s * 0.10, -0.01, 0.80), (s * 0.108, -0.005, 0.62), (s * 0.11, 0, 0.53)],
                           [0.075, 0.088, 0.078, 0.055], 10))
        # Espinilla fina vendada (cuero) con rodillera oscura y vendas marcadas.
        piezas.append(tubo(f"pierna_{lado}", "Cuero", [(s * 0.11, 0, 0.54), (s * 0.11, 0.012, 0.34), (s * 0.11, 0.03, 0.15)],
                           [0.045, 0.042, 0.032], 8))
        piezas.append(lamina(f"pierna_{lado}", "Casco", (s * 0.11, -0.045, 0.43), (0, -0.25, 1), (1, 0, 0), 0.14, 0.07, 0.02,
                             perfil_hoja, segmentos=4))
        for z, r in ((0.40, 0.047), (0.30, 0.044), (0.21, 0.038)):
            piezas.append(tubo(f"pierna_{lado}", "Tela", [(s * 0.11, 0.01 + (0.54 - z) * 0.06, z + 0.012), (s * 0.11, 0.01 + (0.54 - z) * 0.06, z - 0.012)],
                               [r, r], 8))
        # Pie de tres garras hacia delante con talón marcado.
        piezas.append(elipsoide(f"pie_{lado}", "Cuerpo", (s * 0.11, -0.01, 0.07), (0.045, 0.07, 0.05), 10, 7))
        for desvio in (-0.035, 0.0, 0.035):
            piezas.append(lamina(f"pie_{lado}", "Cuerpo", (s * 0.11 + desvio, -0.05, 0.06), (desvio * 2.5, -1, -0.35), (1, 0, 0),
                                 0.15, 0.03, 0.028, perfil_pua, curva=-25, eje_curva=(1, 0, 0), segmentos=4))
        piezas.append(lamina(f"pie_{lado}", "Cuerpo", (s * 0.11, 0.03, 0.08), (0, 1, -0.7), (1, 0, 0), 0.07, 0.025, 0.02, perfil_pua,
                             segmentos=3))
    return piezas


def capa():
    """Capucha y cuello alto, esclavina sobre los hombros y capa de tiras dentadas; broche de anillo."""
    # Capucha caída a la espalda, en punta hacia abajo (REF-01 espalda).
    piezas = [lamina("pecho", "Capa", (0, 0.07, 1.68), (0, 0.45, -1), (1, 0, 0), 0.30, 0.22, 0.07, perfil_hoja,
                     curva=20, eje_curva=(1, 0, 0), segmentos=5)]
    azar = random.Random(SEMILLA + 1)
    # Cuello alto: placas de tela que suben alrededor del cuello, abiertas por delante.
    for grados in range(-140, 141, 35):
        a = math.radians(grados)
        radial = Vector((math.sin(a), math.cos(a), 0))
        base = Vector((0, 0.01, 1.46)) + radial * 0.07
        piezas.append(lamina("pecho", "Capa", base, radial * 0.30 + Vector((0, 0, 1)), radial.cross(Vector((0, 0, 1))), 0.21, 0.10, 0.012,
                             perfil_hoja, segmentos=4))
    # Esclavina: tiras cortas sobre hombros, espalda y los lados del pecho (la V del pecho queda a la vista).
    for grados in range(-140, 141, 20):
        a = math.radians(grados)  # 0 = espalda (+Y); ±180 = delante, que queda abierto
        radial = Vector((math.sin(a), math.cos(a), 0))
        base = Vector((radial.x * 0.12, radial.y * 0.085 + 0.015, 1.56))
        piezas.append(lamina("pecho", "Capa", base, radial * 0.75 + Vector((0, 0, -1)), radial.cross(Vector((0, 0, 1))),
                             0.30 + azar.uniform(-0.04, 0.04), 0.12, 0.012, perfil_tira, curva=25, eje_curva=radial.cross(Vector((0, 0, 1))),
                             desvio_punta=azar.uniform(-0.03, 0.03), segmentos=5))
    # Capa: tiras largas en arco por la espalda, hasta media pierna, que reparten peso entre los huesos de capa.
    for grados in range(-105, 106, 21):
        a = math.radians(grados)
        radial = Vector((math.sin(a), math.cos(a), 0))
        base = Vector((radial.x * 0.17, radial.y * 0.115 + 0.035, 1.48))
        lateral_extremo = abs(grados) > 80
        largo = (0.70 if lateral_extremo else 0.92) + azar.uniform(-0.10, 0.08)
        piezas.append(lamina(pesos_capa, "Capa", base, radial * 0.22 + Vector((0, 0.10, -1)), radial.cross(Vector((0, 0, 1))),
                             largo, 0.14, 0.014, perfil_tira, desvio_punta=azar.uniform(-0.05, 0.05), segmentos=8))
    piezas.append(anillo_metalico("pecho", "Metal", (0, -0.112, 1.47), (0, -1, 0.25), 0.028, 0.0065))
    return piezas


def espada():
    """Espada larga de un filo con guarda de anillo, puño largo envuelto y pomo en punta (REF-01, REF-02)."""
    guarda = Vector(GUARDA)
    abajo = Vector((0, 0, -1))
    adelante = Vector((0, -1, 0))  # el filo mira hacia delante en reposo
    # Hoja: sección asimétrica (filo afilado delante, lomo romo detrás), recta y con una ligera curva al final.
    bm = bmesh.new()
    segmentos = 10
    filas = []
    for i in range(segmentos + 1):
        t = i / segmentos
        curva = 0.04 * max(0.0, (t - 0.6) / 0.4) ** 2
        centro = guarda + abajo * (0.02 + LARGO_HOJA * t) + adelante * curva
        ancho = 0.034 * (1.0 if t < 0.75 else max(0.0, (1.0 - t) / 0.25) ** 0.8) + 0.0015
        grosor = 0.0045 * (1.0 - 0.6 * t) + 0.001
        filas.append([
            bm.verts.new(centro + adelante * (0.55 * ancho)),          # filo
            bm.verts.new(centro + Vector((grosor, 0, 0)) + adelante * (0.1 * ancho)),
            bm.verts.new(centro - adelante * (0.45 * ancho) + Vector((0.6 * grosor, 0, 0))),  # lomo
            bm.verts.new(centro - adelante * (0.45 * ancho) - Vector((0.6 * grosor, 0, 0))),
            bm.verts.new(centro - Vector((grosor, 0, 0)) + adelante * (0.1 * ancho)),
        ])
    indices = []
    for a, b in zip(filas, filas[1:]):
        for k in range(5):
            bm.faces.new((a[k], a[(k + 1) % 5], b[(k + 1) % 5], b[k]))
            indices.append(1 if k in (0, 4) else 0)  # las dos caras que forman el filo brillan en azul
    bm.faces.new(list(reversed(filas[0])))
    indices.append(0)
    piezas = [pieza(bm, "arma", ["Metal", "Filo"], indices)]
    # Guarda: anillo con el agujero hacia el costado y dos gavilanes cortos.
    piezas.append(anillo_metalico("arma", "Metal", guarda + abajo * 0.012, (1, 0, 0), 0.03, 0.0075))
    piezas.append(anillo_metalico("arma", "Filo", guarda + abajo * 0.012, (1, 0, 0), 0.016, 0.003))
    for sentido in (1, -1):
        piezas.append(lamina("arma", "Metal", guarda + adelante * (0.03 * sentido), adelante * sentido + Vector((0, 0, 0.5)), (1, 0, 0),
                             0.05, 0.02, 0.012, perfil_pua, curva=35 * sentido, eje_curva=(1, 0, 0), segmentos=3))
    # Puño envuelto que sube por dentro de la mano y pomo en punta.
    piezas.append(tubo("arma", "Cuero", [guarda - abajo * 0.005, guarda - abajo * LARGO_PUNO], [0.015, 0.013], 8))
    piezas.append(lamina("arma", "Metal", guarda - abajo * LARGO_PUNO, -abajo, (1, 0, 0), 0.05, 0.026, 0.026, perfil_rombo, segmentos=3))
    return piezas


def pistola():
    """Pistola de boceto (armas de fuego retiradas del juego; el hueso y la malla se conservan)."""
    return [
        tubo("arma_fuego", "Metal", [(D * 0.285, -0.08, 0.99), (D * 0.285, -0.30, 0.99)], [0.022, 0.02], 6),
        tubo("arma_fuego", "Cuero", [(D * 0.285, -0.07, 0.99), (D * 0.285, -0.05, 0.92)], [0.018, 0.016], 6),
    ]


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_caballero.py -- <directorio de salida>")
    limpiar_escena()
    armadura = crear_armadura(HUESOS)
    piezas = cabeza() + torso() + brazos() + piernas() + capa() + espada() + pistola()
    malla = unir(piezas, armadura, NOMBRE_MALLA)
    exportar(malla, armadura, argumentos[0], len(HUESOS))


main()
