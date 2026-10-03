"""
Actualiza el latido del cerrojo de sesión en PROGRESS.md con la hora actual.
Uso: python Herramientas/actualizar_latido.py ["Estado"]
El estado por defecto es "SESIÓN INTERACTIVA ACTIVA".
"""

import datetime
import pathlib
import re
import sys

RUTA = pathlib.Path(__file__).resolve().parent.parent / "PROGRESS.md"

estado = sys.argv[1] if len(sys.argv) > 1 else "SESIÓN INTERACTIVA ACTIVA"
ahora = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
texto = RUTA.read_text(encoding="utf-8")
texto = re.sub(r"^\*\*Estado: .*\*\*$", f"**Estado: {estado}**", texto, count=1, flags=re.MULTILINE)
texto = re.sub(r"^\*\*Último latido: .*$", f"**Último latido: {ahora}** (la sesión activa lo actualiza en cada commit)", texto, count=1, flags=re.MULTILINE)
RUTA.write_text(texto, encoding="utf-8")
print(f"latido {ahora} — {estado}")
