# MODO DE TRABAJO AUTÓNOMO CONTINUO

## Regla 0 — Cerrojo de sesión (antes que todo lo demás)
Lee la sección "CERROJO DE SESIÓN" al principio de `PROGRESS.md`. Si dice que hay una sesión activa y su último latido tiene menos de 45 minutos, **no hagas ningún cambio**: termina el turno de inmediato. Así dos sesiones de Claude no editan, compilan ni hacen commit a la vez sobre el mismo repo.

## Misión Principal
Tu objetivo es avanzar en el proyecto de forma 100% autónoma. No debes detenerte a pedir confirmación para implementar cambios, crear archivos o ejecutar pruebas.

## Protocolo de Trabajo por Turnos
1. **Verificación de Estado Inicial:**
   - Revisa `TODO.md` o `PROGRESS.md` para identificar la siguiente tarea pendiente.
   - Si no existe `PROGRESS.md`, créalo de inmediato listando el plan de trabajo detallado por módulos.

2. **Criterios de Ejecución:**
   - Selecciona la tarea de mayor prioridad.
   - Implementa el código completo (sin placeholders ni comentarios `// TODO`).
   - Corre las pruebas locales o linters para verificar que no hay regresiones.
   - Registra cada avance en `PROGRESS.md` tachando la tarea completada.

3. **Cierre de Turno:**
   - Antes de agotar contexto o dar por terminado el turno, guarda el estado exacto en `PROGRESS.md` detallando:
     a) Qué se completó.
     b) Qué está a medio implementar y en qué archivos.
     c) Cuál es el paso exacto con el que debe iniciar el siguiente turno.
   - Haz un commit en git indicando los avances: `git commit -m "auto: [descripción breve del avance]"`

4. **Instrucción de Continuidad:**
   - Al inicio de cada ejecución, tu primera acción SIEMPRE debe ser leer `PROGRESS.md` y continuar directamente en la última tarea pendiente sin esperar instrucciones del usuario.