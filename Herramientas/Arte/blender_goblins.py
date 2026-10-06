"""
Genera los goblins de SAEVUS en Blender: malla + esqueleto (los mismos 25 huesos que el protagonista,
con otras proporciones), para animarlos por código como al protagonista.

Diseño: el de la referencia canónica REF-05 (Documentos/REFERENCIAS_ARTE.md; ARTE_SAEVUS.md, Goblins).
Especie: delgados y nervudos, cabeza grande de hueso con hocico en punta, ojos negros grandes, orejas
largas con el interior marrón, capucha y bufanda de trapo verde pizarra, cinturón con aro, faldón de
tiras rotas, brazos largos con vendas y brazales, garras en manos y pies, armas curvas y toscas.
Primero el goblin base (REF-05 arriba a la izquierda: hoz larga); las variantes reutilizan la especie.

Ranuras de material por nombre: "Arma" es lo que brilla al avisar de un ataque (se lee qué va a golpear).
Convenciones (pipeline-assets.md): metros, mira a -Y en Blender (+Y en UE), origen en los pies.

Ejecución sin interfaz (Generar-Goblins.ps1 lo hace):
  blender.exe --background --factory-startup --python blender_goblins.py -- <directorio de salida>
"""

import math
import os
import random
import sys

import bpy
from mathutils import Matrix, Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from blender_piezas import *  # noqa: E402,F403

SEMILLA = 11

# Colores (lineales) medidos en REF-05 (ARTE_SAEVUS.md, Paleta). Se repiten en saevus_crear_assets.py.
colores_materiales({
    "Piel": (0.11, 0.087, 0.076, 1.0),     # hueso grisáceo #5F534E
    "Oscuro": (0.010, 0.011, 0.012, 1.0),  # ropa interior y pantalón #1A1B1D
    "Trapo": (0.027, 0.031, 0.033, 1.0),   # capucha, bufanda, faldón #2E3133
    "Cuero": (0.048, 0.029, 0.025, 1.0),   # cinturón, vendas, interior de las orejas
    "Metal": (0.075, 0.097, 0.12, 1.0),    # aros y hebillas
    "Ojos": (0.004, 0.004, 0.005, 1.0),    # ojos negros brillantes
    "Arma": (0.16, 0.14, 0.13, 1.0),       # hoja tosca; brilla al avisar
})

D = -1.0  # derecha del goblin (-X)
I = 1.0


def huesos_goblin(e):
    """Esqueleto del goblin a escala e (1 = goblin base de 1,35 m hasta lo alto de las orejas)."""
    def p(x, y, z):
        return (x * e, y * e, z * e)
    huesos = [
        ("raiz", None, p(0, 0, 0), p(0, 0, 0.1)),
        ("pelvis", "raiz", p(0, 0, 0.62), p(0, 0, 0.70)),
        ("columna", "pelvis", p(0, 0, 0.70), p(0, 0, 0.84)),
        ("pecho", "columna", p(0, 0, 0.84), p(0, 0, 1.00)),
        ("cuello", "pecho", p(0, -0.02, 1.00), p(0, -0.04, 1.08)),
        ("cabeza", "cuello", p(0, -0.04, 1.08), p(0, -0.04, 1.36)),
        ("capa_1", "pecho", p(0, 0.10, 0.98), p(0, 0.13, 0.72)),
        ("capa_2", "capa_1", p(0, 0.13, 0.72), p(0, 0.16, 0.46)),
        ("capa_3", "capa_2", p(0, 0.16, 0.46), p(0, 0.18, 0.25)),
    ]
    for lado, s in (("d", D), ("i", I)):
        huesos += [
            (f"hombro_{lado}", "pecho", p(s * 0.05, 0, 0.99), p(s * 0.15, 0, 0.99)),
            (f"brazo_{lado}", f"hombro_{lado}", p(s * 0.15, 0, 0.99), p(s * 0.21, 0.01, 0.75)),
            (f"antebrazo_{lado}", f"brazo_{lado}", p(s * 0.21, 0.01, 0.75), p(s * 0.25, -0.03, 0.52)),
            (f"mano_{lado}", f"antebrazo_{lado}", p(s * 0.25, -0.03, 0.52), p(s * 0.26, -0.05, 0.44)),
            (f"muslo_{lado}", "pelvis", p(s * 0.08, 0, 0.62), p(s * 0.12, -0.02, 0.34)),
            (f"pierna_{lado}", f"muslo_{lado}", p(s * 0.12, -0.02, 0.34), p(s * 0.12, 0.03, 0.09)),
            (f"pie_{lado}", f"pierna_{lado}", p(s * 0.12, 0.03, 0.09), p(s * 0.12, -0.12, 0.02)),
        ]
    agarre = p(D * 0.262, -0.065, 0.47)
    huesos += [
        ("arma", "mano_d", agarre, (agarre[0], agarre[1], agarre[2] - 0.5 * e)),
        ("arma_fuego", "mano_d", p(D * 0.26, -0.05, 0.50), p(D * 0.26, -0.25, 0.50)),
    ]
    return huesos


class Goblin:
    """Construye las piezas de un goblin a escala e; las variantes añaden o cambian piezas."""

    def __init__(self, e=1.0, semilla=SEMILLA):
        self.e = e
        self.azar = random.Random(semilla)

    def v(self, x, y, z):
        return Vector((x * self.e, y * self.e, z * self.e))

    def cabeza(self, orejas=0.30, hocico=0.17):
        e, v = self.e, self.v
        piezas = [
            elipsoide("cabeza", "Piel", v(0, -0.05, 1.19), (0.095 * e, 0.115 * e, 0.105 * e)),
            # Frente y hocico en punta hacia abajo y delante (máscara de hueso).
            lamina("cabeza", "Piel", v(0, -0.115, 1.29), (0, -0.38, -1), (1, 0, 0), hocico * e + 0.08 * e, 0.15 * e, 0.07 * e,
                   lambda t: max(0.05, 1.0 - 0.95 * t ** 1.2), curva=12, eje_curva=(1, 0, 0), segmentos=5),
        ]
        for s in (D, I):
            # Ojos negros grandes, rasgados hacia arriba y afuera.
            ojo = elipsoide("cabeza", "Ojos", (0, 0, 0), (0.032 * e, 0.012 * e, 0.019 * e), 10, 6)
            ojo.matrix_world = Matrix.Translation(v(s * 0.047, -0.162, 1.185)) @ Matrix.Rotation(math.radians(-22 * s), 4, "Y") \
                @ Matrix.Rotation(math.radians(20 * s), 4, "Z")
            piezas.append(ojo)
            # Ceja de hueso sobre el ojo.
            piezas.append(lamina("cabeza", "Piel", v(s * 0.01, -0.15, 1.225), (s * 0.9, 0.35, 0.35), (0, 0, 1), 0.09 * e, 0.028 * e,
                                 0.016 * e, perfil_hoja, segmentos=3))
            # Orejas largas en punta, hacia atrás, afuera y arriba; interior de cuero.
            direccion = Vector((s * 0.75, 0.55, 0.45)).normalized()
            lateral = direccion.cross(Vector((0, -1, 0))).normalized()
            base = v(s * 0.075, -0.02, 1.23)
            piezas.append(lamina("cabeza", "Piel", base, direccion, lateral, orejas * e, 0.10 * e, 0.014 * e, perfil_hoja,
                                 curva=20, eje_curva=(0, -s, 0), segmentos=6))
            piezas.append(lamina("cabeza", "Cuero", base + v(0, -0.008, 0), direccion, lateral, orejas * 0.82 * e, 0.065 * e, 0.006 * e,
                                 perfil_hoja, curva=20, eje_curva=(0, -s, 0), segmentos=6))
        return piezas

    def capucha(self):
        e, v = self.e, self.v
        piezas = [
            # Capucha caída detrás de la cabeza y bufanda alrededor del cuello.
            lamina("cabeza", "Trapo", v(0, 0.03, 1.31), (0, 0.42, -1), (1, 0, 0), 0.34 * e, 0.27 * e, 0.09 * e, perfil_hoja,
                   curva=15, eje_curva=(1, 0, 0), segmentos=5),
            tubo("cuello", "Trapo", [v(0, -0.01, 0.97), v(0, -0.03, 1.07)], [0.11 * e, 0.085 * e], 10),
        ]
        for grados in range(-150, 151, 30):
            a = math.radians(grados)
            radial = Vector((math.sin(a), math.cos(a), 0))
            base = v(0, -0.01, 1.03) + radial * (0.095 * e)
            piezas.append(lamina("pecho", "Trapo", base, radial * 0.45 + Vector((0, 0, -1)), radial.cross(Vector((0, 0, 1))),
                                 (0.24 + self.azar.uniform(-0.04, 0.05)) * e, 0.10 * e, 0.012 * e, perfil_tira,
                                 desvio_punta=self.azar.uniform(-0.02, 0.02) * e, segmentos=4))
        return piezas

    def torso(self, ancho=1.0):
        e, v = self.e, self.v
        a = ancho
        piezas = [
            tubo("pecho", "Trapo", [v(0, 0, 0.82), v(0, 0.01, 0.92), v(0, 0.02, 1.01)], [0.075 * e * a, 0.095 * e * a, 0.085 * e * a], 10, 0.72),
            tubo("columna", "Oscuro", [v(0, 0, 0.64), v(0, 0, 0.84)], [0.065 * e * a, 0.07 * e * a], 10, 0.75),
            elipsoide("pelvis", "Oscuro", v(0, 0, 0.64), (0.09 * e * a, 0.07 * e * a, 0.07 * e)),
            # Joroba: la espalda encorvada bajo la capucha.
            elipsoide("pecho", "Trapo", v(0, 0.05, 0.95), (0.10 * e * a, 0.07 * e, 0.07 * e)),
            # Cinturón con aro.
            tubo("pelvis", "Cuero", [v(0, 0, 0.695), v(0, 0, 0.735)], [0.078 * e * a, 0.078 * e * a], 12, 0.85),
            anillo_metalico("pelvis", "Metal", v(0, -0.07 * a, 0.715), (0, -1, 0), 0.03 * e, 0.007 * e),
        ]
        # Faldón de tiras rotas alrededor de la cadera, que se reparte entre pelvis y muslo.
        for grados in range(-165, 166, 30):
            r = math.radians(grados)
            radial = Vector((math.sin(r), -math.cos(r), 0))  # 0 = delante
            lado = "d" if radial.x < 0 else "i"
            tramos = [(0.68 * e, "pelvis"), (0.42 * e, f"muslo_{lado}")]
            base = v(radial.x * 0.08 * a, radial.y * 0.065 * a, 0.70)
            piezas.append(lamina(lambda q, t=tramos: interpolar_por_altura(q.z, t), "Trapo", base, radial * 0.25 + Vector((0, 0, -1)),
                                 radial.cross(Vector((0, 0, 1))), (0.42 + self.azar.uniform(-0.08, 0.06)) * e, 0.085 * e, 0.010 * e,
                                 perfil_tira, desvio_punta=self.azar.uniform(-0.03, 0.03) * e, segmentos=5))
        return piezas

    def brazos(self, grosor=1.0):
        e, v = self.e, self.v
        g = grosor
        piezas = []
        for lado, s in (("d", D), ("i", I)):
            piezas += [
                elipsoide(f"hombro_{lado}", "Piel", v(s * 0.14, 0, 0.985), (0.045 * e * g, 0.045 * e * g, 0.04 * e * g)),
                tubo(f"brazo_{lado}", "Piel", [v(s * 0.15, 0, 0.99), v(s * 0.21, 0.01, 0.75)], [0.033 * e * g, 0.026 * e * g], 8),
                tubo(f"antebrazo_{lado}", "Piel", [v(s * 0.21, 0.01, 0.75), v(s * 0.25, -0.03, 0.52)], [0.03 * e * g, 0.022 * e * g], 8),
                # Vendas de cuero en el antebrazo y placa en punta en el codo.
                tubo(f"antebrazo_{lado}", "Cuero", [v(s * 0.232, -0.012, 0.64), v(s * 0.247, -0.027, 0.55)], [0.034 * e * g, 0.028 * e * g], 8),
                lamina(f"antebrazo_{lado}", "Piel", v(s * 0.215, 0.03, 0.75), (s * 0.3, 1, 0.5), (0, 0, 1), 0.075 * e, 0.035 * e, 0.012 * e,
                       perfil_pua, segmentos=3),
                # Hombrera de cuero.
                lamina(f"hombro_{lado}", "Cuero", v(s * 0.10, -0.01, 1.03), (s * 1, 0.1, -0.55), (0, 1, 0), 0.12 * e, 0.10 * e, 0.016 * e,
                       perfil_hoja, curva=25, eje_curva=(0, s, 0), segmentos=4),
                elipsoide(f"mano_{lado}", "Piel", v(s * 0.258, -0.045, 0.49), (0.026 * e, 0.032 * e, 0.038 * e), 10, 7),
            ]
            for k in range(4):
                piezas.append(lamina(f"mano_{lado}", "Piel", v(s * 0.265, -0.07 + 0.016 * k, 0.465), (s * 0.2, -0.3, -1), (0, 1, 0),
                                     0.085 * e, 0.012 * e, 0.012 * e, perfil_pua, curva=-35, eje_curva=(s, 0, 0), segmentos=3))
        return piezas

    def piernas(self, grosor=1.0):
        e, v = self.e, self.v
        g = grosor
        piezas = []
        for lado, s in (("d", D), ("i", I)):
            piezas += [
                tubo(f"muslo_{lado}", "Oscuro", [v(s * 0.08, 0, 0.64), v(s * 0.10, -0.012, 0.48), v(s * 0.12, -0.02, 0.35)],
                     [0.058 * e * g, 0.06 * e * g, 0.042 * e * g], 8),
                # Rodillera de hueso, espinilla fina con vendas y pie de garras.
                lamina(f"pierna_{lado}", "Piel", v(s * 0.12, -0.06, 0.29), (0, -0.3, 1), (1, 0, 0), 0.12 * e, 0.07 * e, 0.025 * e,
                       perfil_hoja, segmentos=4),
                tubo(f"pierna_{lado}", "Piel", [v(s * 0.12, -0.02, 0.35), v(s * 0.12, 0.0, 0.22), v(s * 0.12, 0.03, 0.09)],
                     [0.036 * e * g, 0.032 * e * g, 0.025 * e * g], 8),
                tubo(f"pierna_{lado}", "Cuero", [v(s * 0.12, 0.004, 0.2), v(s * 0.12, 0.022, 0.12)], [0.035 * e * g, 0.03 * e * g], 8),
                elipsoide(f"pie_{lado}", "Piel", v(s * 0.12, -0.01, 0.05), (0.04 * e, 0.06 * e, 0.04 * e), 10, 7),
                lamina(f"pie_{lado}", "Piel", v(s * 0.12, 0.03, 0.06), (0, 1, -0.6), (1, 0, 0), 0.07 * e, 0.025 * e, 0.02 * e, perfil_pua,
                       segmentos=3),
            ]
            for desvio in (-0.03, 0.0, 0.03):
                piezas.append(lamina(f"pie_{lado}", "Piel", v(s * 0.12 + desvio, -0.045, 0.045), (desvio * 3, -1, -0.3), (1, 0, 0),
                                     0.13 * e, 0.026 * e, 0.024 * e, perfil_pua, curva=-30, eje_curva=(1, 0, 0), segmentos=4))
        return piezas

    def hoz(self, largo=0.55, curva=-95, ancho=0.065):
        """Hoz larga y tosca (REF-05 arriba a la izquierda): mango por la mano, hoja que se curva hacia delante."""
        e, v = self.e, self.v
        agarre = v(D * 0.262, -0.065, 0.47)
        return [
            tubo("arma", "Cuero", [agarre + v(0, 0, 0.12), agarre + v(0, 0, -0.03)], [0.015 * e, 0.016 * e], 6),
            lamina("arma", "Arma", agarre + v(0, 0, -0.02), (0, 0.15, -1), (0, 1, 0), largo * e, ancho * e, 0.012 * e,
                   lambda t: max(0.05, (0.5 + t) if t < 0.5 else (1.0 - t) * 2.0), curva=curva, eje_curva=(1, 0, 0), segmentos=8),
        ]

    def base(self):
        return self.cabeza() + self.capucha() + self.torso() + self.brazos() + self.piernas()


def goblin_base():
    """Goblin base, canónico de la especie (REF-05 arriba a la izquierda): ágil, capucha y hoz larga."""
    g = Goblin()
    return g.base() + g.hoz(), huesos_goblin(1.0)


# --- Rey Goblin (REF-06) ------------------------------------------------------------------

ALTO_REY = 3.3  # hasta lo alto de la corona: ≈ 1,9 veces el protagonista (comparación de escala de REF-08)
colores_materiales({"Placas": (0.068, 0.044, 0.038, 1.0)})  # placas pardo oscuras del Rey #4A3B37 (REF-06)


def huesos_rey():
    """Esqueleto del Rey: proporciones de REF-06 (cabeza ≈ 0,13 del alto, hombros anchos, brazos largos)."""
    k = ALTO_REY / 1.92

    def p(x, y, z):
        return (x * k * 1.15, y * k, z * k)
    huesos = [
        ("raiz", None, p(0, 0, 0), p(0, 0, 0.12)),
        ("pelvis", "raiz", p(0, 0, 0.92), p(0, 0, 1.02)),
        ("columna", "pelvis", p(0, 0, 1.02), p(0, 0, 1.24)),
        ("pecho", "columna", p(0, 0, 1.24), p(0, 0, 1.48)),
        ("cuello", "pecho", p(0, -0.01, 1.48), p(0, -0.03, 1.60)),
        ("cabeza", "cuello", p(0, -0.03, 1.60), p(0, -0.03, 1.92)),
        ("capa_1", "pecho", p(0, 0.13, 1.46), p(0, 0.18, 1.05)),
        ("capa_2", "capa_1", p(0, 0.18, 1.05), p(0, 0.23, 0.60)),
        ("capa_3", "capa_2", p(0, 0.23, 0.60), p(0, 0.27, 0.15)),
    ]
    for lado, s in (("d", D), ("i", I)):
        huesos += [
            (f"hombro_{lado}", "pecho", p(s * 0.06, 0, 1.47), p(s * 0.20, 0, 1.47)),
            (f"brazo_{lado}", f"hombro_{lado}", p(s * 0.20, 0, 1.47), p(s * 0.25, 0.01, 1.17)),
            (f"antebrazo_{lado}", f"brazo_{lado}", p(s * 0.25, 0.01, 1.17), p(s * 0.28, -0.02, 0.90)),
            (f"mano_{lado}", f"antebrazo_{lado}", p(s * 0.28, -0.02, 0.90), p(s * 0.29, -0.04, 0.80)),
            (f"muslo_{lado}", "pelvis", p(s * 0.10, 0, 0.92), p(s * 0.12, 0, 0.48)),
            (f"pierna_{lado}", f"muslo_{lado}", p(s * 0.12, 0, 0.48), p(s * 0.12, 0.03, 0.12)),
            (f"pie_{lado}", f"pierna_{lado}", p(s * 0.12, 0.03, 0.12), p(s * 0.12, -0.15, 0.02)),
        ]
    agarre = p(D * 0.29, -0.06, 0.85)
    huesos += [
        ("arma", "mano_d", agarre, (agarre[0], agarre[1], agarre[2] - 0.6)),
        ("arma_fuego", "mano_d", p(D * 0.29, -0.05, 0.88), p(D * 0.29, -0.25, 0.88)),
    ]
    return huesos, k


def rey_goblin():
    """
    Rey Goblin (REF-06, fase 1; la fase 2 recolorea "Trapo" en rojo y enciende "Ojos" desde los datos).
    Corona de púas de hierro, máscara en pico, hombreras de placas con púas, broche de anillo, costillas
    de hueso, cinturón con aro, faldón roto, capa hasta el suelo, piernas con placas, guja de media luna.
    """
    huesos, k = huesos_rey()
    azar = random.Random(SEMILLA + 5)

    def v(x, y, z):
        return Vector((x * k * 1.15, y * k, z * k))
    piezas = [
        # Cabeza: cráneo de placas, máscara de hueso en pico.
        elipsoide("cabeza", "Placas", v(0, 0, 1.73), (0.085 * k, 0.10 * k, 0.11 * k)),
        lamina("cabeza", "Piel", v(0, -0.085, 1.84), (0, -0.30, -1), (1, 0, 0), 0.22 * k, 0.10 * k, 0.05 * k,
               lambda t: max(0.05, 1.0 - 0.95 * t ** 1.1), curva=10, eje_curva=(1, 0, 0), segmentos=5),
    ]
    for s in (D, I):
        ojo = elipsoide("cabeza", "Ojos", (0, 0, 0), (0.02 * k, 0.008 * k, 0.01 * k), 10, 6)
        ojo.matrix_world = Matrix.Translation(v(s * 0.034, -0.095, 1.765)) @ Matrix.Rotation(math.radians(-20 * s), 4, "Y")
        piezas.append(ojo)
        # Orlas de púas a los lados de la cabeza.
        for altura, largo, abertura in ((1.78, 0.22, 0.15), (1.72, 0.16, -0.05)):
            direccion = Vector((s * 1.0, 0.25, abertura)).normalized()
            piezas.append(lamina("cabeza", "Placas", v(s * 0.06, 0.0, altura), direccion, Vector((0, 0, 1)), largo * k, 0.05 * k,
                                 0.014 * k, perfil_pua, curva=15, eje_curva=(0, s, 0), segmentos=4))
    # Corona: aro de hierro con banda de tela y púas altas.
    piezas.append(tubo("cabeza", "Metal", [v(0, 0, 1.83), v(0, 0, 1.87)], [0.075 * k, 0.075 * k], 12, 1.1))
    piezas.append(tubo("cabeza", "Trapo", [v(0, 0, 1.835), v(0, 0, 1.855)], [0.078 * k, 0.078 * k], 12, 1.1))
    for indice in range(9):
        a = 2 * math.pi * indice / 9
        base = v(math.sin(a) * 0.065, math.cos(a) * 0.075, 1.86)
        largo = (0.22 if indice in (0, 4, 5) else 0.15) + azar.uniform(-0.02, 0.03)
        piezas.append(lamina("cabeza", "Metal", base, (math.sin(a) * 0.15, math.cos(a) * 0.15, 1), (math.cos(a), -math.sin(a), 0),
                             largo * k, 0.03 * k, 0.03 * k, perfil_pua, segmentos=3))
    # Torso: costillas de hueso a la vista, cintura de placas, cinturón con aro.
    piezas += [
        tubo("cuello", "Placas", [v(0, 0, 1.46), v(0, -0.02, 1.62)], [0.06 * k, 0.05 * k]),
        tubo("pecho", "Placas", [v(0, 0, 1.22), v(0, 0, 1.36), v(0, 0.01, 1.50)], [0.15 * k, 0.18 * k, 0.17 * k], 12, 0.72),
        lamina("pecho", "Piel", v(0, -0.11, 1.25), (0, -0.15, 1), (1, 0, 0), 0.20 * k, 0.15 * k, 0.012 * k,
               lambda t: max(0.1, t ** 0.6), segmentos=4),
        tubo("columna", "Placas", [v(0, 0, 1.0), v(0, 0, 1.24)], [0.14 * k, 0.145 * k], 12, 0.75),
        elipsoide("pelvis", "Placas", v(0, 0, 0.97), (0.14 * k, 0.10 * k, 0.10 * k)),
        tubo("pelvis", "Cuero", [v(0, 0, 1.08), v(0, 0, 1.14)], [0.135 * k, 0.135 * k], 14, 0.8),
        anillo_metalico("pelvis", "Metal", v(0, -0.11, 1.11), (0, -1, 0), 0.04 * k, 0.01 * k),
    ]
    for z in (1.28, 1.33, 1.38):
        piezas.append(tubo("pecho", "Piel", [v(-0.10, -0.11, z), v(0, -0.132, z + 0.02), v(0.10, -0.11, z)], [0.009 * k] * 3, 6))
    # Esclavina de placas en punta sobre hombros y espalda (tela al frente), con el broche de anillo.
    for grados in range(-160, 161, 20):
        a = math.radians(grados)
        radial = Vector((math.sin(a), math.cos(a), 0))
        base = v(radial.x * 0.11, radial.y * 0.08 + 0.01, 1.53)
        piezas.append(lamina("pecho", "Placas" if abs(grados) < 120 else "Trapo", base, radial * 0.9 + Vector((0, 0, -1)),
                             radial.cross(Vector((0, 0, 1))), (0.40 + azar.uniform(-0.05, 0.05)) * k, 0.13 * k, 0.02 * k,
                             perfil_hoja, curva=20, eje_curva=radial.cross(Vector((0, 0, 1))), segmentos=4))
    piezas.append(anillo_metalico("pecho", "Metal", v(0, -0.115, 1.45), (0, -1, 0.2), 0.035 * k, 0.009 * k))
    # Faldón roto por delante y a los lados; capa larga hasta el suelo por detrás.
    for grados in range(-120, 121, 24):
        r = math.radians(grados)
        radial = Vector((math.sin(r), -math.cos(r), 0))
        lado = "d" if radial.x < 0 else "i"
        tramos = [(1.05 * k, "pelvis"), (0.6 * k, f"muslo_{lado}")]
        piezas.append(lamina(lambda q, t=tramos: interpolar_por_altura(q.z, t), "Trapo", v(radial.x * 0.12, radial.y * 0.09, 1.10),
                             radial * 0.2 + Vector((0, 0, -1)), radial.cross(Vector((0, 0, 1))), (0.62 + azar.uniform(-0.08, 0.08)) * k,
                             0.10 * k, 0.012 * k, perfil_tira, desvio_punta=azar.uniform(-0.04, 0.04) * k, segmentos=6))
    tramos_capa = [(1.40 * k, "pecho"), (1.15 * k, "capa_1"), (0.75 * k, "capa_2"), (0.35 * k, "capa_3")]
    for grados in range(-105, 106, 15):
        a = math.radians(grados)
        radial = Vector((math.sin(a), math.cos(a), 0))
        piezas.append(lamina(lambda q: interpolar_por_altura(q.z, tramos_capa), "Trapo", v(radial.x * 0.17, radial.y * 0.12 + 0.03, 1.47),
                             radial * 0.18 + Vector((0, 0.06, -1)), radial.cross(Vector((0, 0, 1))),
                             (1.38 + azar.uniform(-0.12, 0.06)) * k, 0.12 * k, 0.014 * k, perfil_tira,
                             desvio_punta=azar.uniform(-0.05, 0.05) * k, segmentos=9))
    # Brazos y piernas de placas con púas; manos y pies de garra.
    for lado, s in (("d", D), ("i", I)):
        piezas += [
            elipsoide(f"hombro_{lado}", "Placas", v(s * 0.19, 0, 1.47), (0.075 * k, 0.07 * k, 0.065 * k)),
            tubo(f"brazo_{lado}", "Placas", [v(s * 0.20, 0, 1.47), v(s * 0.25, 0.01, 1.17)], [0.075 * k, 0.062 * k], 8),
            tubo(f"antebrazo_{lado}", "Placas", [v(s * 0.25, 0.01, 1.17), v(s * 0.265, -0.005, 1.03), v(s * 0.28, -0.02, 0.90)],
                 [0.062 * k, 0.07 * k, 0.045 * k], 8),
            lamina(f"antebrazo_{lado}", "Placas", v(s * 0.255, 0.03, 1.15), (s * 0.4, 1, 0.3), (0, 0, 1), 0.12 * k, 0.04 * k, 0.015 * k,
                   perfil_pua, segmentos=3),
            elipsoide(f"mano_{lado}", "Placas", v(s * 0.29, -0.03, 0.86), (0.032 * k, 0.04 * k, 0.05 * k), 10, 7),
            tubo(f"muslo_{lado}", "Placas", [v(s * 0.10, 0, 0.95), v(s * 0.115, 0, 0.70), v(s * 0.12, 0, 0.49)],
                 [0.10 * k, 0.105 * k, 0.075 * k], 10),
            lamina(f"pierna_{lado}", "Placas", v(s * 0.12, -0.05, 0.40), (0, -0.3, 1), (1, 0, 0), 0.16 * k, 0.08 * k, 0.03 * k,
                   perfil_hoja, segmentos=4),
            tubo(f"pierna_{lado}", "Placas", [v(s * 0.12, 0, 0.49), v(s * 0.12, 0.015, 0.30), v(s * 0.12, 0.03, 0.12)],
                 [0.075 * k, 0.07 * k, 0.05 * k], 8),
            elipsoide(f"pie_{lado}", "Placas", v(s * 0.12, -0.02, 0.06), (0.05 * k, 0.08 * k, 0.05 * k), 10, 7),
        ]
        # Hombreras de placas en punta que ensanchan la silueta (REF-06).
        for direccion, largo in (((s * 0.3, 0, 1), 0.20), ((s * 1, 0.1, 0.6), 0.26), ((s * 1, -0.2, 0.1), 0.24), ((s * 1, 0.3, -0.4), 0.22),
                                 ((s * 0.5, 0.6, 0.6), 0.2)):
            piezas.append(lamina(f"hombro_{lado}", "Placas", v(s * 0.20, 0.0, 1.50), direccion, Vector((0, 1, 0)), largo * k, 0.14 * k,
                                 0.03 * k, perfil_hoja, segmentos=4))
        # Placas sobre antebrazo y espinilla.
        piezas.append(lamina(f"antebrazo_{lado}", "Placas", v(s * 0.30, -0.02, 0.98), (0.0, -0.05, 1), (0, 1, 0), 0.24 * k, 0.09 * k, 0.03 * k,
                             perfil_hoja, segmentos=4))
        piezas.append(lamina(f"pierna_{lado}", "Placas", v(s * 0.12, -0.075, 0.14), (0, -0.1, 1), (1, 0, 0), 0.26 * k, 0.10 * k, 0.03 * k,
                             perfil_hoja, segmentos=4))
        for dedo in range(4):
            piezas.append(lamina(f"mano_{lado}", "Placas", v(s * 0.30, -0.06 + 0.017 * dedo, 0.83), (s * 0.15, -0.3, -1), (0, 1, 0),
                                 0.10 * k, 0.014 * k, 0.014 * k, perfil_pua, curva=-30, eje_curva=(s, 0, 0), segmentos=3))
        for desvio in (-0.035, 0.0, 0.035):
            piezas.append(lamina(f"pie_{lado}", "Placas", v(s * 0.12 + desvio, -0.06, 0.05), (desvio * 2.5, -1, -0.3), (1, 0, 0),
                                 0.16 * k, 0.03 * k, 0.028 * k, perfil_pua, curva=-25, eje_curva=(1, 0, 0), segmentos=4))
        for z in (1.05, 0.62, 0.30):
            hueso = f"muslo_{lado}" if z > 0.5 else f"pierna_{lado}"
            piezas.append(lamina(hueso, "Placas", v(s * 0.17, 0.0, z), (s * 1, 0.4, 0.5), (0, 0, 1), 0.10 * k, 0.035 * k, 0.015 * k,
                                 perfil_pua, segmentos=3))
    # Guja: asta vertical por la mano derecha; hoja de hacha en media luna arriba, aro y tela; pincho abajo.
    agarre = v(D * 0.29, -0.06, 0.85)
    arriba, abajo = agarre + Vector((0, 0, 1.55)), agarre + Vector((0, 0, -0.80))
    piezas += [
        tubo("arma", "Cuero", [abajo, arriba], [0.028, 0.032], 8),
        tubo("arma", "Trapo", [arriba - Vector((0, 0, 0.30)), arriba - Vector((0, 0, 0.18))], [0.04, 0.04], 8),
        anillo_metalico("arma", "Metal", arriba + Vector((0, -0.05, 0.05)), (1, 0, 0), 0.07, 0.016),
        lamina("arma", "Arma", arriba + Vector((0, -0.04, 0.10)), (0, -1, 0.35), (0, 0, 1), 0.80, 0.32, 0.03,
               lambda t: max(0.05, math.sin(math.pi * min(1.0, t * 0.85 + 0.15)) ** 0.8), curva=-130, eje_curva=(1, 0, 0), segmentos=10),
        lamina("arma", "Metal", arriba, (0, 0, 1), (0, 1, 0), 0.30, 0.06, 0.04, perfil_pua, segmentos=3),
        lamina("arma", "Metal", abajo, (0, 0, -1), (0, 1, 0), 0.22, 0.05, 0.04, perfil_pua, segmentos=3),
    ]
    return piezas, huesos


TIPOS = {
    "SK_Goblin": goblin_base,
    "SK_ReyGoblin": rey_goblin,
}


def main():
    argumentos = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    if not argumentos:
        raise SystemExit("uso: blender --background --python blender_goblins.py -- <directorio de salida>")
    directorio = argumentos[0]
    for nombre, crear in TIPOS.items():
        limpiar_escena()
        piezas, huesos = crear()
        armadura = crear_armadura(huesos)
        malla = unir(piezas, armadura, nombre)
        exportar(malla, armadura, directorio, len(huesos))


main()
