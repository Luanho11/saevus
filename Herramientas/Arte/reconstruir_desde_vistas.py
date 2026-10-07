"""
Reconstruye el volumen de un personaje a partir de las vistas de las referencias (Documentos/REFERENCIAS_ARTE.md),
para que la forma y el color del modelo salgan del dibujo y no de primitivas a mano.

Python del sistema (numpy, scipy, PIL). Cada vista es un recorte de una hoja con una silueta sobre fondo uniforme
(lo que se aparta del fondo, más claro u oscuro, es figura), registrado en el espacio del personaje (metros; mira a
-Y, derecha = -X, arriba = +Z, pies en Z = 0):
- de cuerpo: escala por el alto total (alto_m), o la de otra vista ("paso_como"), pies en Z = 0, centrada en las piernas;
- de detalle (primeros planos): escala por el alto de la pieza, con su borde inferior en z_base y centrada sobre la
  vista de cuerpo correspondiente a esa altura.
Cada pieza (cuerpo, cabeza...) se talla como casco visual con sus vistas (un vóxel es suyo si cae en la silueta de
cada una) y, opcionalmente, con una elipse por tramo de la silueta de frente (brazos y piernas redondos, no losas).
El color de cada vóxel de superficie se toma de las vistas que lo ven, pesadas por lo de frente que lo miran.
Escribe <salida>.npz con las piezas: puntos (m), colores (sRGB 0-1) y tamaño del vóxel de cada una, para
blender_malla_desde_puntos.py.

Uso: python reconstruir_desde_vistas.py <configuración.json> <salida.npz>
"""

import json
import sys

import numpy as np
from PIL import Image
from scipy import ndimage as ndi

EJES = {"x": 0, "y": 1}
MIRADAS = {"-y": np.array([0, -1, 0]), "+y": np.array([0, 1, 0]), "-x": np.array([-1, 0, 0]), "+x": np.array([1, 0, 0])}


class Vista:
    """Silueta y color de un recorte, con la conversión entre píxeles y el espacio del personaje."""

    def __init__(self, nombre, definicion, imagenes, umbral):
        self.nombre = nombre
        imagen = imagenes.setdefault(definicion["imagen"], np.asarray(Image.open(definicion["imagen"]).convert("RGB")).astype(np.float32))
        x0, y0, x1, y1 = definicion["caja"]
        recorte = imagen[y0:y1, x0:x1]
        borde = np.concatenate([recorte[:, :5], recorte[:, -5:]], axis=1)
        fondo = np.median(borde, axis=1, keepdims=True)
        m = np.abs(recorte - fondo).max(axis=2) > umbral
        m = ndi.binary_closing(m, iterations=2)
        m = ndi.binary_fill_holes(m)
        etiquetas, numero = ndi.label(m)
        m = etiquetas == (np.argmax(ndi.sum(m, etiquetas, range(1, numero + 1))) + 1)
        corte = definicion.get("corte")  # fila (en el recorte) por debajo de la cual no es de esta pieza
        if corte is not None:
            m[corte:] = False
        self.m, self.color = m, recorte
        ys, xs = np.nonzero(m)
        self.arriba, self.abajo = ys.min(), (corte - 1 if corte is not None else ys.max())
        self.eje = EJES[definicion["eje"]]
        self.signo = definicion.get("signo", 1)
        self.mira = MIRADAS[definicion["mira"]]
        self.espejo = definicion.get("espejo", False)
        self.prioridad = definicion.get("prioridad", 1.0)
        self.definicion = definicion
        # Registro (se completa en registrar): mundo = ((columna - centro) * signo * paso, ..., (abajo - fila) * paso + z_base).
        self.paso = self.centro = self.z_base = None

    def registrar_cuerpo(self, alto_m):
        self.paso = alto_m / (self.abajo - self.arriba)
        self.z_base = 0.0
        ys, xs = np.nonzero(self.m)
        piernas = ys > self.arriba + 0.55 * (self.abajo - self.arriba)
        self.centro = xs[piernas].mean()

    def registrar_detalle(self, alto_m, z_base, sobre):
        """Escala por el alto de la pieza; centrada sobre la vista de cuerpo 'sobre' en su franja de alturas."""
        self.paso = alto_m / (self.abajo - self.arriba)
        self.z_base = z_base
        ys, xs = np.nonzero(self.m)
        centro_propio = xs.mean()
        filas = [f for f in range(sobre.m.shape[0]) if z_base <= sobre.z_de_fila(f) <= z_base + alto_m]
        cols = np.concatenate([np.nonzero(sobre.m[f])[0] for f in filas if sobre.m[f].any()])
        mundo_sobre = (cols.mean() - sobre.centro) * sobre.signo * sobre.paso
        # La columna centro_propio cae en mundo_sobre: centro = columna que cae en 0.
        self.centro = centro_propio - mundo_sobre / (self.signo * self.paso)

    def z_de_fila(self, fila):
        return (self.abajo - fila) * self.paso + self.z_base

    def pixel(self, puntos):
        """Fila y columna de cada punto del mundo (pueden caer fuera)."""
        coordenada = puntos[:, self.eje]
        if self.espejo:
            coordenada = np.abs(coordenada) * np.sign(self.signo)
        columna = coordenada / (self.signo * self.paso) + self.centro
        fila = self.abajo - (puntos[:, 2] - self.z_base) / self.paso
        return np.round(fila).astype(int), np.round(columna).astype(int)

    def dentro(self, puntos):
        fila, columna = self.pixel(puntos)
        valido = (fila >= 0) & (fila < self.m.shape[0]) & (columna >= 0) & (columna < self.m.shape[1])
        resultado = np.zeros(len(puntos), dtype=bool)
        resultado[valido] = self.m[fila[valido], columna[valido]]
        return resultado

    def muestra(self, puntos):
        fila, columna = self.pixel(puntos)
        fila = np.clip(fila, 0, self.m.shape[0] - 1)
        columna = np.clip(columna, 0, self.m.shape[1] - 1)
        return self.color[fila, columna] / 255.0, self.dentro(puntos)


def tallar(pieza, vistas, paso):
    """Casco visual de la pieza en una rejilla de vóxeles de 'paso' metros. Devuelve (ocupado, origen)."""
    usadas = [vistas[n] for n in pieza["vistas_casco"]]
    minimo, maximo = np.array(pieza["caja_m"][0], dtype=float), np.array(pieza["caja_m"][1], dtype=float)
    tamano = np.ceil((maximo - minimo) / paso).astype(int) + 1
    gx, gy, gz = (minimo[i] + np.arange(tamano[i]) * paso for i in range(3))
    ocupado = np.zeros((tamano[2], tamano[1], tamano[0]), dtype=bool)
    X, Y = np.meshgrid(gx, gy)
    elipses = pieza.get("elipses")
    for k, z in enumerate(gz):
        puntos = np.stack([X.ravel(), Y.ravel(), np.full(X.size, z)], axis=1)
        dentro = np.ones(len(puntos), dtype=bool)
        for vista in usadas:
            dentro &= vista.dentro(puntos)
        if elipses and dentro.any():
            dentro &= recortar_elipses(puntos, dentro, vistas[elipses["frente"]], elipses)
        ocupado[k] = dentro.reshape(X.shape)
    ocupado = ndi.gaussian_filter(ocupado.astype(np.float32), pieza.get("suavizado_voxeles", 1.6)) > 0.45
    # Fuera los trozos sueltos (bordes de otras piezas que el casco deja flotando): solo lo bastante grande.
    etiquetas, numero = ndi.label(ocupado)
    if numero > 1:
        tamanos = ndi.sum(ocupado, etiquetas, range(1, numero + 1))
        minimo_trozo = pieza.get("fraccion_minima_trozo", 0.003) * tamanos.max()
        ocupado = np.isin(etiquetas, 1 + np.nonzero(tamanos >= minimo_trozo)[0])
    return ocupado, minimo


def recortar_elipses(puntos, dentro, frente, elipses):
    """Por cada tramo de la silueta de frente en este corte, una elipse de fondo proporcional a su ancho."""
    resultado = np.zeros(len(puntos), dtype=bool)
    fila, _ = frente.pixel(puntos[:1])
    fila = int(np.clip(fila[0], 0, frente.m.shape[0] - 1))
    tramos_fila = np.diff(np.concatenate([[0], frente.m[fila].astype(np.int8), [0]]))
    inicios, finales = np.nonzero(tramos_fila == 1)[0], np.nonzero(tramos_fila == -1)[0]
    ys_dentro = puntos[dentro, 1]
    centro_y = (ys_dentro.min() + ys_dentro.max()) / 2.0 if len(ys_dentro) else 0.0
    fondo_max = (ys_dentro.max() - ys_dentro.min()) / 2.0 if len(ys_dentro) else 0.0
    for a, b in zip(inicios, finales):
        cx = ((a + b) / 2.0 - frente.centro) * frente.signo * frente.paso
        w = max((b - a) / 2.0 * frente.paso, frente.paso)
        ancho_tronco = elipses.get("ancho_tronco_m", 0.10)
        d = min(w * elipses.get("factor_fondo", 0.7) if w > ancho_tronco else w, fondo_max + frente.paso)
        cy = centro_y if w > ancho_tronco else np.clip(centro_y, ys_dentro.min() + d, ys_dentro.max() - d) if len(ys_dentro) else 0.0
        resultado |= ((puntos[:, 0] - cx) / w) ** 2 + ((puntos[:, 1] - cy) / max(d, frente.paso)) ** 2 <= 1.0
    return resultado


def colorear(puntos, normales, vistas, nombres):
    total = np.zeros((len(puntos), 3), dtype=np.float32)
    peso_total = np.full((len(puntos), 1), 1e-5, dtype=np.float32)
    for nombre in nombres:
        vista = vistas[nombre]
        color, visto = vista.muestra(puntos)
        mira = vista.mira.astype(np.float32)
        if vista.definicion.get("ambos_lados"):
            # Vista de un costado usada para los dos (perfil): cuenta lo de frente que mira cada costado.
            frontalidad = np.abs(normales @ mira)
        else:
            frontalidad = np.clip(normales @ mira, 0, 1)  # la normal apunta hacia la cámara de la vista
        peso = (frontalidad ** 2 * vista.prioridad * visto)[:, None]
        total += color * peso
        peso_total += peso
    return total / peso_total


def main():
    configuracion = json.load(open(sys.argv[1], encoding="utf-8"))
    umbral = configuracion.get("umbral", 10)
    imagenes = {}
    vistas = {n: Vista(n, {"imagen": configuracion["imagen"], **d}, imagenes, umbral) for n, d in configuracion["vistas"].items()}
    for vista in vistas.values():
        if "detalle" not in vista.definicion:
            vista.registrar_cuerpo(configuracion["alto_m"])
    for vista in vistas.values():
        if "paso_como" in vista.definicion:
            # Misma escala de dibujo que otra vista de la fila (su propio alto no es el del personaje: corona más baja...).
            vista.paso = vistas[vista.definicion["paso_como"]].paso
    for vista in vistas.values():
        if "detalle" in vista.definicion:
            detalle = vista.definicion["detalle"]
            vista.registrar_detalle(detalle["alto_m"], detalle["z_base"], vistas[detalle["sobre"]])

    salida = {}
    for pieza in configuracion["piezas"]:
        paso = pieza["voxel_m"]
        ocupado, origen = tallar(pieza, vistas, paso)
        superficie = ocupado & ~ndi.binary_erosion(ocupado)
        kz, ky, kx = np.nonzero(superficie)
        puntos = np.stack([origen[0] + kx * paso, origen[1] + ky * paso, origen[2] + kz * paso], axis=1).astype(np.float32)
        if "z_max" in pieza:
            conservar = puntos[:, 2] <= pieza["z_max"]
            puntos, kz, ky, kx = puntos[conservar], kz[conservar], ky[conservar], kx[conservar]
        suave = ndi.gaussian_filter(ocupado.astype(np.float32), 1.5)
        gz, gy, gx = np.gradient(suave)
        normales = -np.stack([gx[kz, ky, kx], gy[kz, ky, kx], gz[kz, ky, kx]], axis=1)
        normales /= np.linalg.norm(normales, axis=1, keepdims=True) + 1e-6
        colores = colorear(puntos, normales, vistas, pieza["vistas_color"])
        nombre = pieza["nombre"]
        salida[f"{nombre}_puntos"], salida[f"{nombre}_colores"], salida[f"{nombre}_paso"] = puntos, colores.astype(np.float32), np.float32(paso)
        print(f"[reconstruir] {nombre}: vóxel {paso * 1000:.1f} mm, {ocupado.sum()} vóxeles, {len(puntos)} de superficie")
    salida["piezas"] = np.array([p["nombre"] for p in configuracion["piezas"]])
    np.savez_compressed(sys.argv[2], **salida)
    print(f"[reconstruir] escrito {sys.argv[2]}")


main()
