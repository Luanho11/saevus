"""
Paleta de un personaje sacada de su referencia (Documentos/REFERENCIAS_ARTE.md): para cada material, colores reales
de los píxeles del dibujo en las cajas de muestra, ordenados de oscuro a claro. Así el tono y la variación de las
facetas del modelo salen de la referencia y no de un color plano elegido a mano.

Python del sistema (numpy, PIL). Escribe:
- <textura.png>: paleta de ANCHO x ANCHO píxeles; cada material ocupa FILAS_POR_MATERIAL filas seguidas, en el orden
  del JSON, con sus colores de oscuro a claro (sRGB, tal cual el dibujo);
- <textura.json>: {"ancho": .., "alto": .., "materiales": {material: [fila_inicial, número de celdas]}} para
  blender_piezas.pintar_paleta.

Uso: python paleta_desde_referencia.py <configuración.json> <textura.png>
"""

import json
import sys

import numpy as np
from PIL import Image, ImageFilter

ANCHO = 16
FILAS_POR_MATERIAL = 4
# Diferencia mínima (0-255) con el fondo de la hoja para que un píxel cuente como del personaje.
UMBRAL_FONDO = 10
SEMILLA = 5


def main():
    configuracion, salida = sys.argv[1], sys.argv[2]
    datos = json.load(open(configuracion, encoding="utf-8"))
    # Mediana 3x3: quita el ruido de compresión sin perder las facetas.
    imagen = Image.open(datos["imagen"]).convert("RGB").filter(ImageFilter.MedianFilter(3))
    pixeles = np.asarray(imagen).astype(np.float32)
    fondo = np.median(np.concatenate([pixeles[:8].reshape(-1, 3), pixeles[-8:].reshape(-1, 3)]), axis=0)
    azar = np.random.default_rng(SEMILLA)
    celdas = ANCHO * FILAS_POR_MATERIAL
    materiales = datos["muestras"]
    paleta = np.zeros((FILAS_POR_MATERIAL * len(materiales), ANCHO, 3), np.uint8)
    indice = {}
    for orden, (material, cajas) in enumerate(materiales.items()):
        muestras = np.concatenate([pixeles[y0:y1, x0:x1].reshape(-1, 3) for x0, y0, x1, y1 in cajas])
        muestras = muestras[np.abs(muestras - fondo).max(axis=1) > UMBRAL_FONDO]
        if len(muestras) < celdas:
            raise SystemExit(f"{material}: solo {len(muestras)} píxeles de personaje en sus cajas")
        elegidos = muestras[azar.choice(len(muestras), celdas, replace=False)]
        elegidos = elegidos[np.argsort(elegidos @ np.array([0.2126, 0.7152, 0.0722]))]
        fila = orden * FILAS_POR_MATERIAL
        paleta[fila:fila + FILAS_POR_MATERIAL] = elegidos.reshape(FILAS_POR_MATERIAL, ANCHO, 3).round().astype(np.uint8)
        indice[material] = [fila, celdas]
        media = elegidos.mean(axis=0).round().astype(int)
        print(f"[paleta] {material}: {len(muestras)} píxeles, media #{media[0]:02X}{media[1]:02X}{media[2]:02X}")
    Image.fromarray(paleta).save(salida)
    json.dump({"ancho": ANCHO, "alto": paleta.shape[0], "materiales": indice},
              open(salida.rsplit(".", 1)[0] + ".json", "w", encoding="utf-8"), indent=1)
    print(f"[paleta] escrita {salida} ({ANCHO}x{paleta.shape[0]})")


main()
