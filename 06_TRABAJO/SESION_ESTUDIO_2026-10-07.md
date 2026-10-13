---
estado: pendiente_de_respuestas
actualizado: 2026-10-07
resumen: Declaraciones de Armando, preguntas del montaje y alcance real del estudio iniciado.
---

# Sesión de estudio · 07-10-2026

## Declaraciones del usuario

- Quiere estudiar la historia y aprender cómo funciona el kart y por qué.
- Tiene el kart disponible para responder preguntas del montaje.
- Reconfirma 16 celdas de batería, tras la declaración del 01-10 de 16 funcionales.
- Quiere añadir mejora de pantalla y mejora de PCB.
- Describe energía residual al desconectar el contactor; la atribuye a los variadores y dice que afecta al programa. No aporta todavía mediciones ni secuencia detallada.
- Pide dejarlo guardado y preguntar cuando falte información.

Procedencia: mensaje de esta conversación del 07-10-2026. Son declaraciones, no una inspección física.

## Preguntas enviadas, respuestas pendientes

1. Al apagar: ¿qué queda encendido o falla, cuánto dura y cómo se recupera? ¿Ocurre con USB desconectado?
2. Pantalla: ¿prioridad de lectura al sol/tamaño, menús/avisos o datos de batería/autonomía?
3. Batería: ¿las 16 están en serie y activas, y el BMS muestra 16 tensiones? ¿Qué modelo aparece en BMS y variadores, sin desmontar?

| Dato para la ficha del montaje | Estado al cerrar esta nota |
|---|---|
| Número de celdas | 16, confirmado por el usuario; conexión y configuración BMS por contrastar. |
| BMS actual | El 01-10 lo identificó como el de los archivos finales; etiqueta y parámetros pendientes. |
| Variador izquierdo/derecho | Modelos y configuraciones pendientes de lectura física. |
| Alimentación de STM32/ESP32 | Punto de toma, secuencia y tensiones al apagar pendientes. |
| Firmware grabado | No identificado; no confundir con fuentes entregados. |
| Síntoma al apagar | Declarado; componente, tiempo y recuperación pendientes. |
| Pantalla deseada | Mejora solicitada; requisitos pendientes. |

## Resultados guardados

- [Guía de estudio](GUIA_DE_ESTUDIO_DEL_KART.md): historia, explicaciones por sistemas, preguntas y recorrido de aprendizaje.
- [PROB-0003](../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0003_energia_residual_al_abrir_contactor.md): apagado y energía residual, sin causa confirmada.
- [Ideas](IDEAS.md): pantalla y PCB ampliadas como propuestas.
- Estado, tareas y cobertura documental actualizados.

## Cobertura real de esta sesión

Consulta de índice, fichas, notas previas, inventario de código y plan. Recuperación mediante `buscar_documentos.py` del texto de DOC-0042 PDF **8, 29, 31, 33, 43–44, 48–49, 69, 74–75, 78, 90–91**; DOC-0008 PDF **11, 14**. DOC-0031: búsqueda que localizó precarga en PDF 11; no nueva lectura completa de esa página.

Historia: DOC-0023 PDF **5** (índice, no comprobación del banco); DOC-0024 PDF **5**; DOC-0025 PDF **109**; DOC-0026 PDF **47**; DOC-0002 PDF **60**; DOC-0027 PDF **67**; DOC-0003 PDF **44**. Texto revisado; ninguna revisión visual nueva de figuras o tablas. Se reaprovecharon las notas anteriores sin releer informes enteros.

Consulta externa del manual oficial Kelly KEB V3.4 y nota Texas Instruments SCDA015 para explicar condensadores/precarga y realimentación de circuitos apagados. Enlaces y límites en PROB-0003. No equivalen a identificar el hardware real.

No se modificaron originales, firmware, parámetros del BMS ni cableado. Guardado local; sin commit/push ni nueva copia de seguridad configurada.
