---
estado: corrección local dev10-ui compilada y probada en PC; sin instalar
actualizado: 2026-10-21
---

# ENS-0027 · Media ponderada y sensibilidad del pedal

## Petición y configuración

Tras [ENS-0026](ENS-0026_diagnostico_bloqueo_marchas.md), Armando declara que dev9 mejoró los cambios, pero perdió sensibilidad y «solo aceleró a los 800 en el ADC». Prefiere la respuesta anterior y pide una media ponderada de muestras. También confirma que giraba hacia un tope cuando volvió a N. Finalmente comunica **pantalla desconectada**: esta continuación se realiza exclusivamente en PC, sin acceso SWD/Wi-Fi ni grabación.

Última configuración física identificada: F746 **dev9-ui**, ESP32 **v1.2-diagnostico**; v1.3-dev3 trasera continúa sin instalar. HEAD de base `bebe4528747bc6e05dc1ba244cd3ce9a090a7942`, cambios locales previos conservados. Evidencia inicial/dev9 y respaldo íntegro siguen en ENS-0026. [Fuentes, binarios, datos y logs de esta continuación](../evidencias/pedal_filtrado_2026-10-21/README.md).

## Diagnóstico y criterio de diseño

El desplazamiento del cero a 320 en dev9 cambia toda la pendiente: ADC 800 daba 1593/4095 (38,9 %) en dev8 y 1275/4095 (31,1 %) en dev9. Esto demuestra pérdida de mando para la misma ADC; **no demuestra que toda la falta de movimiento hasta 800 provenga de ese cambio**. No hay curva medida de par/RPM frente al pedal, ni prueba simultánea controlada del umbral de arranque de los motores.

Los registros anteriores distinguen variación y desplazamiento: reposo dedicado media 187,13; retorno media 218,56; máximo 302 en la fase R/freno con pedal declarado libre. Una media ponderada suaviza oscilaciones, pero deja pasar un desplazamiento sostenido. Volver directamente a cero 124/permisivo hasta 141 reproduciría el bloqueo. Permitir engranar con mando residual positivo tampoco es la solución.

**Dev10 conserva cero hasta ADC 320**, parámetro provisional basado en el máximo observado 302 + 18, y recupera exactamente la curva histórica desde **512 hasta fondo**. El punto 512 es una elección de diseño, no un punto calibrado del sensor. Entre 320 y 512 hay una recta continua hasta la respuesta histórica de 512: evita el salto de aproximadamente 11 % que produciría aplicar la curva antigua justo al salir de 320. El tramo inicial sigue siendo una zona muerta; no se afirma recuperar todo el recorrido físico perdido. Confirmar tacto/respuesta en banco antes de considerar cerrado PROB-0012.

## Implementación

`Core/Src/kart_frontend.c`, `pedal_command`:

1. Cada lectura ADC se valida **antes** del filtro; se conservan los límites históricos de plausibilidad 24–1961 cuentas. Una sola conversión fallida/fuera de rango produce mando cero e invalida la entrada; se borra el historial.
2. Media de cuatro muestras válidas: `f = floor((4*x[n] + 3*x[n-1] + 2*x[n-2] + x[n-3])/10)`. Tarea nominal cada 5 ms; cuatro muestras abarcan 15 ms desde la primera hasta la última en la prueba ideal. No es latencia física medida del kart.
3. Primera muestra válida inicializa las cuatro posiciones; no se arranca el promedio con ceros que puedan hacer parecer suelto un pedal pisado. Reiniciar o una separación entre lecturas ≥20 ms descarta el historial antiguo. Cálculo sin espera/bucle de adquisición adicional ni asignación dinámica.
4. Lectura actual ≤320 fuerza cero en ese ciclo y reinicia el historial: sin cola al soltar. Durante una reducción parcial por encima del cero, la media conserva su respuesta temporal de cuatro muestras; no se promete seguimiento instantáneo de toda bajada.
5. La marcha y el modo exigen a la vez entrada válida, **lectura actual ≤320 y mando cero**. Un promedio aún bajo al comenzar a pisar no puede autorizar la marcha. Freno, quietud, selector, START y CAN mantienen sus reglas.

Con `z=320`, `j=512`, `l=124`, `h=1861`, mando entero:

- `0` si la lectura actual o `f` están en zona cero;
- `(f-z)*(j-l)*4095 / ((j-z)*(h-l))` para `z < f < j`;
- `(f-l)*4095/(h-l)` para `j ≤ f < h`;
- `4095` al alcanzar `h`.

Los cocientes se truncan como en la normalización anterior. El código verifica el orden de las constantes al compilar. ADC físico permanece bruto en snapshot/CAN 0x101/CSV; `accel_raw`, históricamente llamado así, es el mando normalizado ahora filtrado. No cambia el layout de 116 bytes, protocolo CAN ni esquema CSV 2. Entradas muestra **«ADC físico / mando filtrado»**. Versión visible, CSV y archivos: `stm32_frontal-v1.3-dev10-ui`. El aviso del formateador legado se acorta a «PARAR Y SOLTAR PEDAL» para que no se trunque al crecer dev9→dev10.

| ADC estable | dev8, mando % | dev9, mando % | dev10, mando % |
|---|---:|---:|---:|
| 200 | 4,4 | 0 | 0 |
| 320 | 11,3 | 0 | 0 |
| 400 | 15,9 | 5,2 | 9,3 |
| 500 | 21,6 | 11,7 | 20,9 |
| 800 | 38,9 | 31,1 | 38,9 |
| 1861 | 100 | 100 | 100 |

Tabla de **mando calculado**, no porcentaje de par, RPM o recorrido físico. CSV exhaustivo y escalón sintético generados ejecutando la función real están en la evidencia.

## Verificación y resultados negativos

- **185/0 frontal**: regresiones, curva monótona en todo el intervalo válido, máximo incremento estable 5/4095 por cuenta ADC; equivalencia dev8 exhaustiva desde 512, pesos, asentamiento de cuatro muestras, ruido alterno sintético de 80 ADC pico a pico reducido a 16 en la media, retorno inmediato, fallo aislado, reinicio, hueco temporal/desbordamiento del reloj, STOP y rechazo de marcha con promedio bajo/lectura actual alta.
- **163/0 dashboard**, 124 renders sintéticos; página Entradas inspeccionada. No ensayo TouchGFX/LCD físico.
- Reproducción de **2.061 ADC originales** de reposo/retorno/R-freno/dev9: mando cero en todas y permiso D con **CAN, dirección, freno y selector simulados**. Tiempos originales conservados; las capturas son aproximadamente 20 Hz, no las muestras internas completas de 200 Hz. Este replay verifica el cero/permiso, **no mide atenuación del ruido ni latencia real del filtro**.
- ARM F746 **BUILD OK**, sin upload. Interna 349.852 B, QSPI 6.351.932 B idéntica a dev9. ELF SHA-256 `bbc9955d69a3c6a5be551930d38c99a4780944e55d5b8737a53a81bf68696257`; HEX `0ad5c22995668b28d477b28a378c61476f70b687dc293927ac28515bae6ff600`.
- Primeras regresiones: **185/3**, dos expectativas manuales redondeaban 2913 a 2914 erróneamente y un aviso legado se truncaba por el nombre más largo; corregidos. Siguiente **185/1**, el test exigía literalmente «MODO:» en el aviso abreviado; sustituido por comprobación del texto completo de cada aviso, más estricta sobre contenido. Final **185/0**. Logs negativos conservados. Avisos MSVC del arnés y ARM de dependencias preservados. Primer comando de pruebas no arrancó por ruta de salida equivocada; directorios vacíos creados por error retirados, sin pérdida de datos.

## Límites y siguiente ensayo

**Dev10 no está instalada.** No se ha comprobado mejor tacto ni arranque físico por debajo de ADC 800. No hay autoajuste de cero, cambio del fondo, modificación de ESP32, DIF/ESC/BLQ, PWM, límites de reversa ni filtro/rango de dirección. La media no identifica ni elimina una causa eléctrica sostenida.

La dirección llegó a ADC 813 en ENS-0026 frente al mínimo admisible 942; el usuario confirma giro hacia un tope. El filtro del pedal no debe ocultar ese fallo. La prueba PC con 813 sigue invalidando dirección, pasando a N y mandando cero. Centro, ambos topes y retorno deben medirse por separado para distinguir rango mal calibrado de ruido/acoplamiento o fallo del sensor: [PROB-0007](../../05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0007_variacion_y_centro_direccion.md).

Cuando esté disponible: respaldo/identidad antes de instalar; repetir OFF/N, reposo/fondo/retorno, D/N/R con freno, modos parado y aceleraciones graduales con volante quieto en banco. Registrar ADC bruto, mando, marcha, flags/errores y RPM por Wi-Fi en integrado; no exigir USB ESP32 instalado. Medir inicio de giro frente a ADC y confirmar tiempo de corte/respuesta. Repetir la dirección como ensayo independiente. Brownout/MPU y R sin limitador en el ESP32 v1.2 siguen abiertos; esta revisión no los resuelve.
