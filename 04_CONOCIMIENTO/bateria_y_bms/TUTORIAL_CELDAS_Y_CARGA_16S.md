---
estado: orientacion_documental_pendiente_contraste_fisico
actualizado: 2026-10-13
resumen: Winston, tensión, SOC y carga 16S; identidad, capacidad y ajustes reales pendientes.
---

# Tutorial de celdas y carga 16S

Consulta documental del 13-10-2026; no es un ensayo ni una configuración aplicada.

## Identidad y evidencia

- **Usuario:** Armando declara 16 celdas y aproximadamente 3,2–3,3 V por celda **en reposo, sin cargar ni usar**. Sin captura, valores individuales precisos, instrumento, temperatura ni duración del reposo; no se asigna SOC medido.
- **DOC-0042, PDF 69 / impresa 55, §3.4 y figura 17:** Thunder Sky Winston LiFePO₄, 3,3 V nominales, 60 Ah, 16 en serie en esa descripción histórica. Texto recuperado mediante buscador y página completa revisada visualmente.
- **DOC-0030, PDF 1, cabecera y tabla visual:** TSWB-LYP60AHA, **70 Ah**, 35 A (0,5 CA), intervalo 2,8–3,8 V, 2,1 kg ±50 g. Difiere de 60 Ah/2,3 kg del informe. No adoptar 70 Ah sin etiqueta/revisión física; conservar 60 Ah como referencia documental. Los títulos de curvas presentes en la extracción no corresponden a curvas visibles en la página.
- [Página de producto Winston](https://winston-battery.eu/lithium-cells/30-winston-battery-cell-60ah-33v-lifeypo4-tswb-lyp60aha.html), Key Parameters y explicación de Y, consultada 13-10: familia LiFeYPO₄/LiFePO₄, 60 Ah, 3,3 V, intervalo 2,8–3,8 V y carga inicial 3,65 V. Identifica la familia, no todas las unidades montadas.
- Antecedente 15S en DOC-0042 PDF 70/85: no aplicable automáticamente a las 16 actuales declaradas; [consulta previa](../../06_TRABAJO/COMPRA_CARGADOR_Y_NEUMATICOS.md).

## Funcionamiento y porcentaje

El litio se desplaza entre electrodos durante carga/descarga; los electrones pasan por el circuito externo y aportan energía. El voltaje es diferencia de potencial, no cantidad de Ah restante.

En serie se suman voltajes, la corriente atraviesa todas las celdas y no se suman Ah. Con la referencia histórica de 60 Ah: **16 × 3,3 = 52,8 V nominales; 60 Ah; 3168 Wh nominales**. No equivale a capacidad útil actual medida.

SOC es carga restante respecto a capacidad utilizable: 100 % lleno según el criterio de final de carga; 0 % fin de capacidad utilizable, no cero voltios. DOD es lo descargado: de 100 a 20 % son 80 % DOD. SOH describe salud/capacidad respecto a nueva; una batería envejecida puede marcar 100 % y almacenar menos energía.

Las LFP tienen una curva muy plana. **3,2–3,3 V en reposo no determina 50 %, 80 % ni otro porcentaje exacto.** No es el mínimo documental de 2,8 V ni el final de carga. A 3,2 V podría quedar poca reserva; depende de historial, temperatura, reposo, precisión y estado. Durante consumo la tensión baja; cargando sube; al desconectar se relaja. No mezclar condiciones.

[TI SLUA565](https://www.ti.com/lit/an/slua565/slua565.pdf), p.1 figura 1 y p.2 figura 2, respalda la forma general OCV/DOD y dependencia con corriente. Describe otra celda, A123 ANR26650M1: **no se trasladan sus límites ni su curva como calibración Winston**. Para SOC fiable hace falta conteo de corriente/Ah, capacidad y sincronización correctos; no presumir calibrado el BMS del kart.

## Tensiones: límites frente a objetivos

Equivalencias de cálculo para 16 celdas iguales en serie. En uso real manda la primera que llega a un límite.

| Referencia | Celda | 16S |
|---|---:|---:|
| Límite inferior documental, no objetivo de uso | 2,80 V | 44,8 V |
| Margen orientativo para parar y recargar antes del límite | alrededor de 3,00 V | 48,0 V |
| Rango en reposo declarado | 3,20–3,30 V | 51,2–52,8 V si todas están dentro |
| Nominal del informe | 3,30 V | 52,8 V |
| Objetivo moderado propuesto durante carga habitual | 3,45–3,55 V | 55,2–56,8 V |
| Techo prudente propuesto para carga normal | 3,65 V | 58,4 V |
| Extremo superior documental, NO consigna recomendada | 3,80 V | 60,8 V |

**3,65 V/58,4 V es una propuesta conservadora, no garantía de ausencia de daño ni máximo universal de toda LFP.** La ficha local recoge 3,8 V de operación pero no un perfil completo; la página del producto recoge 3,65 V de carga inicial. Se propone no utilizar el extremo 3,8 V como objetivo cotidiano. Confirmar revisión física, BMS, cargador, precisión y balanceo antes de ajustes definitivos.

Los objetivos moderados son orientativos y dependen del perfil/balanceo. 3,45 o 3,55 V no corresponde a un SOC fijo; depende de corriente y tiempo. El margen 3,0 V no es 0 % calibrado ni una protección aplicada. Bajo aceleración puede haber caídas transitorias; registrar corriente y temperatura.

Ejemplo sintético: **1 celda a 3,80 V + 15 a 3,64 V = 58,4 V**. El total parece correcto, pero una excede el techo propuesto. El total nunca sustituye la supervisión de las 16.

## Tutorial práctico de carga

1. Identificar etiquetas de celdas (incluido reemplazo), cargador y BMS; confirmar configuración 16S activa y 16 lecturas. «48 V» o «60 V» comercial no define tensión final.
2. Cargador compatible LFP 16S con **CC/CV**, polaridad/conector y camino protegido por BMS verificados. Conservar bloqueo de marcha documentado en DOC-0042 PDF 76; no puentear protección.
3. En corriente constante (CC), limita amperios y aumenta tensión. En tensión constante (CV), mantiene el techo mientras disminuye corriente. Finalizar según perfil aprobado: alcanzar tensión no significa terminar instantáneamente. No mantener indefinidamente el techo ni emplear ecualización/desulfatación de plomo.
4. Vigilar máxima/mínima por celda, diferencia, corriente y temperatura. Si una llega al techo, no subir el total para forzar a las demás. Ante cortes repetidos, calor anormal o deformación, detener y diagnosticar.
5. Registrar total, 16 valores, corriente final, temperatura, hora y reposo. La bajada de tensión al desconectar no representa una pérdida brusca equivalente de capacidad.

No fijar amperios definitivos sin confirmar conjunto. DOC-0042 PDF 77 (consulta previa) declara 4 A usados/circuito preparado para 10 A, sin acreditar estado actual. Si se validan **10 A** y referencia **60 Ah**, son **0,167 C**; añadir 30 Ah tomaría al menos 3 h a corriente constante, más cola CV/balanceo. Cálculo ideal, no ensayo ni predicción del SOC actual.

Como estrategia orientativa con SOC fiable: recargar hacia 20 % y usar 80–90 % como techo cotidiano si basta autonomía; 100 % cuando haga falta y se complete un perfil correcto. **No son límites del fabricante ni voltajes equivalentes exactos.** Cargas de balanceo/sincronización dependen del BMS. No hace falta agotar antes de recargar; almacenar con carga intermedia y comprobaciones periódicas.

## Método, resultados y siguiente paso

Se consultaron índice, fichas y notas antes de originales; buscador DOC-0030 p.1/DOC-0042 p.69 y revisión visual completa de ambas. [Renders y vista previa](../../06_TRABAJO/evidencias/2026-10-13_bateria/). Gráfica de reposo **sintética y didáctica**, no medida ni digitalización Winston; selector ×16 supone celdas iguales. No permite convertir tensión a SOC. Conserva franja declarada, límite inferior documental y techo de carga propuesto separado de reposo.

Positivo: límites/consignas diferenciados y cálculos 16S; negativo/limitación: discrepancia 60/70 Ah, parámetros físicos y SOC no acreditados. Sin cambios de firmware/binario, ajuste BMS o ensayo físico. No se ejecuta commit/push en esta consulta. Falló el alias Windows python; resuelto con runtime suministrado. URL PDF actual Winston devolvió 404 y charging logic no accesible: no se usan como fuentes verificadas. Un primer parche no se aplicó por edición concurrente del estado; relectura y actualización focalizada conservan ese trabajo.

Próximo: etiqueta de cargador, etiqueta de celda y pantalla de 16 tensiones/protecciones BMS para concretar carga real. [PROB-0010](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0010_identificacion_y_carga_bateria_16s.md), [diario](../../06_TRABAJO/DIARIO_PARA_RELATORIO.md).
