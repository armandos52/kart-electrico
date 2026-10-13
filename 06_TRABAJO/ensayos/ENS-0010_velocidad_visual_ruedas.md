---
estado: vigente
actualizado: 2026-10-09
resumen: Velocidad visual de las ruedas del panel. Iteración 1 (escala fija 3 %) corrigió la saturación pero Armando la juzgó demasiado lenta; iteración 2 gira a velocidad real 1:1 con desenfoque de movimiento. Pruebas sintéticas, sin hardware.
---

# ENS-0010 · Velocidad visual de las ruedas

Fecha: 09-10-2026. Tipo: diagnóstico y corrección de interfaz en dos iteraciones; pruebas automáticas sintéticas y revisión visual. **Sin hardware ni datos del kart.**

## Solicitud y configuración

1. Armando declara que, probando marcha adelante y atrás con las ruedas al 30 % e incrementando, «la animación sigue siendo igual», y pide poder distinguir los aumentos de velocidad en las pruebas reales. Es una **declaración del usuario**: no hay registro de esa prueba (ni si fue en demo o con telemetría real).
2. Tras la iteración 1, Armando declara que es «muy muy lento»: el kart va a unos 50 km/h y la animación se siente como 1 km/h; pide que sea realista y añade que **la parte visual (marcas, aspecto) está bien**. Declaración del usuario, no medición.

Versiones visuales (etiquetas documentales; el código no lleva cadena de versión): `0.3.3-velocidad-ruedas` (iteración 1) y `0.3.4-velocidad-real` (iteración 2). Base Git `4f7f150` con los cambios locales previos sin commit; no se hace commit ni push. Panel normal en `127.0.0.1:8766`, desconectado del kart; los módulos JS se sirven al recargar. Chrome/Playwright en Windows. Fuentes anteriores: [iteración 1](../evidencias/velocidad_ruedas_2026-10-09/antes/) (estado de ENS-0008) y [iteración 2](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/antes/) (estado de la iteración 1, byte a byte).

La sesión que implementó la iteración 1 terminó sin registro; la siguiente la revisó contra las copias `antes/`, repitió las pruebas y la documentó. La iteración 2 se hizo a continuación, con contrato de pruebas escrito antes del código.

## Diagnóstico

**Iteración 1.** Causa en `wheelMotion()` ([nucleo.js](../../07_DESARROLLO/panel_local/static/nucleo.js)), introducida en [ENS-0008](ENS-0008_lados_y_sentido_ruedas_visual.md) para evitar la inversión aparente por repetición de tacos: escala `0,9 / max(150, RPM de la rueda más rápida)` con tope de 0,9 rad/s. Con la demo de entonces (pedal × 4 = RPM): 30 % → 0,72 rad/s; 45 %, 60 %, 80 % y 100 % → 0,90 rad/s. A partir de ~45 % la animación dejaba de acelerar, coherente con lo observado. Además acoplaba I/D (acelerar una rueda ralentizaba la otra). La corrección fue una **escala fija del 3 %** de la velocidad angular real.

**Resultado negativo de la iteración 1.** Matemáticamente correcta (proporcional, independiente por rueda), pero a 100 % de pedal de la demo (400 RPM) la rueda daba 0,2 vueltas por segundo: con el Ø 0,48 m del firmware equivale a unos **1,1 km/h**. Coincide con la sensación descrita por Armando. Aprendizaje: proporcional no basta; el giro tiene que parecerse al real.

**Iteración 2.** Giro real: 50 km/h con Ø 0,48 m son ≈ 553 RPM de rueda, **9,2 vueltas por segundo** (58 rad/s). La escena 3D se redibuja a 30 fps (110° por fotograma) y el esquema a 60 fps (55°). Los tacos están cada 10° y los pernos/radios son repetitivos: sin tratamiento parpadean o parecen girar al revés. Una cámara real resuelve esto con **desenfoque de movimiento**; se reproduce de forma aproximada.

## Cambios (iteración 2)

- **Giro 1:1**: velocidad angular de cada rueda = RPM·2π/60. `wheelRates()` devuelve rad/s con signo (D negativo, R positivo); `wheelMotion()` es rad/s × paso (hasta 0,1 s). Duplicar RPM duplica el giro, I/D independientes, mismo giro por segundo a cualquier fps, RPM no finitas/caducadas dejan la rueda quieta.
- **Desenfoque** (`wheelBlur()`): *nitidez* = 1 hasta 0,35 vueltas/s (≈ 21 RPM, ≈ 1,9 km/h) y 0 desde 1,8 vueltas/s (≈ 108 RPM, ≈ 9,8 km/h), con transición suave; *estela* = giro de la marca durante una exposición simulada de 1/30 s (a 50 km/h ≈ 110°; nunca una vuelta completa).
  - 3D: tacos, pernos y radios van a un grupo con materiales propios que se desvanece y se oculta; la marca deja una estela (shader sobre la banda y los flancos) detrás del sentido de giro. Con el giro rápido marca y estela se atenúan hasta un 40 % para que se lea como difuminado y no como destello.
  - Esquema superior: la banda avanza lo que giran su radio en píxeles reales, se desvanece a un tono uniforme y la marca deja una estela de 16 tramos; mismas funciones y umbrales que el 3D.
  - Pausa y «movimiento reducido»: ruedas nítidas, sin estela y sin giro.
- **Demo**: el pedal da 5,5 RPM por % (100 % ≈ 550 RPM ≈ 50 km/h con Ø 0,48 m, equivalente mostrado junto al mando). Es una convención sintética basada en lo dicho por Armando, no una velocidad máxima medida.
- **Selector «Giro de ruedas: Real 1:1 / Lento 3 %»** en la cabecera de la vista, para comparar con la iteración 1; la preferencia se guarda en el navegador. Por defecto, real.
- Sin cambios: marcas y aspecto, sentido de giro, cámaras, lados I/D, calibración de dirección, señales y firmware.

## Procedimiento y evidencia

1. **Contrato antes del código** ([check-velocidad.mjs](../../07_DESARROLLO/panel_local/qa/check-velocidad.mjs), datos sintéticos, Node): sobre el [código de la iteración 1](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/antes_contrato.json) → **8 fallos de 35** (giro real 1:1, 50 km/h, `wheelRates`, `wheelBlur`, modo seleccionable). Sobre el [código final](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/despues_contrato.json) → **48/48**, incluidos nitidez monótona, estela = giro/30 s (≈ 110° a 50 km/h), simetría D/R y entradas inválidas. (En la iteración 1 el contrato era de 27 comprobaciones; esta lo amplía.)
2. **Chrome** ([check-velocidad-browser.cjs](../../07_DESARROLLO/panel_local/qa/check-velocidad-browser.cjs); [resultado](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/browser.json)): **53/53**. Giro 3D medido en D y R con pedal 30/60/100 %: 2,8 / 5,6 / 9,3 vueltas por segundo (17,5 / 34,9 / 58,3 rad/s, dentro del 15 % de lo teórico) y, a 100 %, equivalente a **≈ 50 km/h**. Banda del esquema a 3 %: ±95,6 px/s frente a 94,2 esperados. Nitidez 1 → 0,66 → 0 a 3 / 10 / 30 %, estela 33° / 66° / 110° a 30 / 60 / 100 %, esquema coherente con el 3D, estela detrás del sentido, pausa/movimiento reducido/pedal cero sin estela, selector Lento (3 %, nítido) persistente tras recarga, móvil sin desbordes, sin escrituras al servidor ni errores JS.
3. **Regresión** con el código final, en [regresion/](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/regresion/): sentido y lados **19/19**, modelo 3D **32/32**, esquema superior **30/30**; **26 tests Python**.
4. Capturas revisadas: [3D a 60 %](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/3d-60.png), [3D a 100 %](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/3d-100.png), [pausa](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/3d-pausa.png), [esquema](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/esquema-100.png), [móvil](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/movil-demo.png) y ampliaciones de [3D](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/ampliacion-3d-30-y-100.png) y [esquema](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/ampliacion-esquema-10-30-60-100.png). [Hashes](../evidencias/velocidad_ruedas_2026-10-09/iteracion2_real/archivos_sha256.json) de código, QA y evidencia.
5. Evidencia de la iteración 1 (contrato 10 fallos → 27/27, Chrome 25/25, regresión 19/32/30) conservada en [la carpeta principal](../evidencias/velocidad_ruedas_2026-10-09/); sus pruebas de velocidad ya no son vigentes porque la escala cambió.

## Resultados negativos y limitaciones

- **Iteración 1 rechazada por lenta** (1,1 km/h aparentes a 100 % de la demo). Se mantiene como opción «Lento 3 %».
- **Pruebas mal planteadas, corregidas:** el primer recuento de segmentos de estela en el esquema dependía de la fase de la vuelta y falló; se sustituyó por la energía media durante 1 s. La comprobación de movimiento reducido miraba el 3D mientras estaba oculto (la escena no se actualiza oculta, por diseño); se comprueba tras volver a la vista. `check-sentido` muestreaba a 140 ms/65 ms, ambiguo con giro real; ahora usa ventanas de ~60 ms y, para el esquema, pedal 3 %. Los px/s de la banda del esquema a 60 fps solo son fiables hasta 9 px por fotograma, por eso se miden a 3 % y no se registran a alta velocidad.
- **El desenfoque es un modelo visual**, no óptico: umbrales (0,35–1,8 vueltas/s) y exposición (1/30 s) elegidos por criterio y revisados en capturas, sin prueba perceptiva con Armando. Una captura estática no muestra el movimiento: que «se sienta» a 50 km/h solo lo puede juzgar Armando mirando el panel. No se probó en su pantalla real (solo Chrome sin pantalla).
- 50 km/h usa el **Ø 0,48 m de referencia del firmware**, aún sin contrastar; los neumáticos del 3D son de proporción aproximada. Con telemetría real el panel representa las RPM tal como llegan: PPR y calibración de los Hall, pendientes, condicionan que el giro coincida con el de la rueda real.
- **QA antiguos fuera de uso, anteriores a este cambio:** `check-calibration.cjs` (busca `#cal-raw`, inexistente desde la calibración guiada) y `check-panel.cjs` (puerto 8765, espera lista de placas). No se tocaron ni se ejecutaron con éxito; calibración y firmware intactos.

## Decisión y siguiente paso

Se adopta **giro real 1:1 con desenfoque de movimiento** por defecto, conservando «Lento 3 %» como comparación. Siguiente: que Armando lo vea (demo y, después, por Wi-Fi con RPM reales). Si el difuminado resulta excesivo o escaso, se ajustan `BLUR_START`, `BLUR_FULL` y `EXPOSURE` en `nucleo.js`. Contrastar Ø de rueda y PPR para que el equivalente en km/h sea fiable. Actualizar los QA antiguos (ver [tareas](../TAREAS.md)).
