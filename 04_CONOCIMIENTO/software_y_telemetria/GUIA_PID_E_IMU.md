---
estado: vigente
actualizado: 2026-10-23
resumen: Guía de aprendizaje del PID y la IMU (Steppe School) aplicada al ESC del kart, con simulador local; sin cambios de firmware.
---

# Guía de PID e IMU para el kart

## Resumen

- **Qué es:** fase de aprendizaje pedida por Armando el 23-10-2026 para preparar la futura IMU. Se estudian el artículo de PID en C de Steppe School, su visor IMU y el artículo de estimación de orientación que enlaza. Todo se explica con el control ESC real del kart.
- **Hallazgo principal (análisis de código, sin ensayo):** el ESC v1.3 ya es un lazo cerrado, pero solo con término **P**. Tiene zona muerta de 2 °/s, ganancia 1/200 por °/s y corrección máxima del 10 %. No tiene I ni D. Por eso deja **error estacionario**: el kart gira algo menos de lo que pide la consigna.
- **Segundo hallazgo:** la consigna de guiñada `v·tan(δ)/L` no está limitada por adherencia. A 36 km/h y 20° pide ≈160 °/s, que exigiría ≈2,8 g laterales. Es imposible, y con un término I provocaría *windup*.
- **Recomendación para cuando haya IMU:** un **PI** sobre la guiñada con anti-windup, consigna limitada por μ·g/v, sin D al principio (el ADC de dirección tiene ruido, PROB-0015) y lazo a periodo fijo.
- **Simulador:** [[04_CONOCIMIENTO/software_y_telemetria/simulador_pid_imu.html|simulador_pid_imu.html]] muestra el kart desde arriba girando, con sus fuerzas, el giróscopo, el acelerómetro y los modos DIF/ESC/PID. Es un **modelo didáctico con valores supuestos**, no la dinámica medida del kart.
- **Límites:** no se ha tocado firmware ni hardware. La IMU sigue sin funcionar en el kart ([[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0017_imu_no_disponible|PROB-0017]]). Las cifras del simulador son sintéticas.

## Fuentes consultadas (23-10-2026)

| Fuente | Qué contiene |
|---|---|
| [PID implementation in C](https://www.steppeschool.com/blog/pid-implementation-in-c) (Steppe School, 13-10-2025) | Lazo abierto y cerrado, P/I/D, ecuación discreta, código C con saturación y anti-windup, ejemplo de STM32, tabla de efectos, simulador de escalón, Ziegler–Nichols, tabla de problemas |
| [IMU Orientation Viewer](https://imu.steppeschool.com/) | Visor 3D en el navegador por USB serie; filtros Acc/Mag directo, Complementario, Madgwick y EKF; gráficas en bruto; formato de paquete |
| [Attitude Estimation](https://www.steppeschool.com/blog/attitude-estimation-rotation-matrices-quaternions-sensor-fusion) (25-05-2026) | Marcos de referencia, matrices de rotación, alabeo/cabeceo desde el acelerómetro, guiñada desde el magnetómetro, integración del giróscopo, filtro complementario, cuaterniones, Madgwick y EKF |
| [Firmware de ejemplo](https://github.com/Steppeschool/stm32-simple-imu-reading) | Enlazado desde el visor; **no revisado** |

Esta guía resume con palabras propias y aplica las ideas al kart. No copia el código de la web.

## 1. Lazo abierto y lazo cerrado en el kart

- **Lazo abierto:** se manda una orden y no se comprueba el resultado. En el kart es el modo **DIF**: reparte el esfuerzo entre las ruedas traseras según el ángulo de dirección, con la fórmula geométrica `split = (vía/2 / batalla)·tan δ`, sin mirar si el kart gira de verdad.
- **Lazo cerrado:** se mide la salida y se corrige. En el kart es el modo **ESC**. Compara la guiñada ideal con la que mide el giróscopo y añade una corrección diferencial: más esfuerzo en una rueda y menos en la otra.

Con el ESC el kart puede compensar cosas que DIF no ve: un motor más débil, asfalto mojado o subviraje. Su riesgo es que, con ganancias mal elegidas, puede oscilar.

## 2. Error, P, I y D aplicados a la guiñada

`error = guiñada_ideal − guiñada_medida` (°/s)

| Término | Pregunta | En el kart | Riesgo |
|---|---|---|---|
| **P** = Kp·e | ¿Cuánto me falta ahora? | Si el kart gira 6 °/s menos de lo pedido, empuja más la rueda exterior | Solo con P siempre queda un error residual. Demasiado Kp hace oscilar |
| **I** = Ki·∑e·dt | ¿Cuánto tiempo llevo con error? | Va acumulando el déficit de giro hasta que el error medio es cero | *Windup*: si la consigna es imposible, crece sin límite y después da un tirón al contrario |
| **D** = Kd·de/dt | ¿Con qué rapidez cambia? | Frena el cambio brusco de la guiñada y amortigua | Amplifica el ruido del giróscopo y del ADC de dirección |

**Por qué P sola deja error:** si el error fuese cero, la corrección también sería cero. Pero el kart necesita una corrección constante para girar más de lo que giraría sin control. El sistema se queda donde un error pequeño produce justo esa corrección. En el ESC actual además existe la zona muerta de 2 °/s: dentro de ella la corrección vale cero.

## 3. De la ecuación al microcontrolador

Forma continua: `u(t) = Kp·e + Ki·∫e dt + Kd·de/dt`.

En el microcontrolador se calcula en pasos de periodo `dt`:

- integral: `integral += e·dt`
- derivada: `(e − e_anterior) / dt`, o mejor `−(medida − medida_anterior) / dt`

**El `dt` debe ser fijo.** Si el lazo corre a ritmo variable, I y D cambian solos y la sintonía deja de valer. La web recomienda ejecutar el lazo desde una interrupción de temporizador. En el ESP32 trasero el giróscopo se lee cada `GYRO_PERIOD_MS = 10` ms ([hardware_kart.h](../../07_DESARROLLO/firmware/esp32_trasero/include/hardware_kart.h)). Hay que **verificar** que el control también corre a periodo fijo y que `dt` es el real. Es un punto pendiente: no se ha medido.

## 4. Mejoras que propone la web y por qué importan en el kart

1. **Saturación de la salida:** la corrección no puede pasar de lo que los motores pueden dar. El ESC v1.3 ya lo hace (`esc_max_correction = 0.10`).
2. **Anti-windup:** si la salida está saturada, no se sigue acumulando la integral. Es imprescindible en el kart, porque la adherencia hace que muchas consignas sean imposibles.
3. **Derivada sobre la medida:** evita la «patada» cuando cambia la consigna. En el kart la consigna cambia cada vez que se mueve el volante, y lleva el ruido del ADC de dirección.
4. **Filtro en la derivada:** pasabajos sobre D. Coincide con la línea de Armando para PROB-0015: solo pasabajos.
5. **Estado en una estructura:** un controlador por estructura permite tener varios lazos con el mismo código.

### Esbozo para el kart (aprendizaje, no integrado en el firmware)

Escrito al estilo de `kart_control` (C++ sin periféricos). Es una propuesta para estudiar, no código validado:

```cpp
struct YawPi {
    float kp=0.006f, ki=0.02f;   // por °/s y por °; valores del simulador, NO validados
    float limit=0.10f;           // igual que esc_max_correction
    float integral=0;            // °
    void reset() { integral=0; }
    // dt en segundos, medido; error en °/s. Devuelve corrección relativa [-limit, limit].
    float update(float error, float dt) {
        if(!(dt>0 && dt<0.05f) || !isfinite(error)) { reset(); return 0; }
        const float candidate=integral+error*dt;
        const float u=kp*error+ki*candidate;
        if(u>limit)  return limit;     // saturado: no se acumula (anti-windup)
        if(u<-limit) return -limit;
        integral=candidate;
        return u;
    }
};
```

Reglas que debería cumplir en el firmware:

- `reset()` siempre que el control se inhiba: pedal en cero, freno, N, cambio de modo, MPU no válida. Así no queda integral vieja guardada.
- Consigna limitada: `|r_ideal| ≤ k·μ·g / v` (k ≈ 0,85), con un μ prudente.
- Con `yaw_valid = false`, el ESC inhibe la marcha. Ya lo hace la v1.3 (`MpuUnavailable`) y conviene mantenerlo.

## 5. Efecto de cada ganancia (resumen de la tabla de la web)

| Si subes… | Subida | Sobreoscilación | Asentamiento | Error estacionario |
|---|---|---|---|---|
| Kp | más rápida | peor | casi igual | menor |
| Ki | más rápida | peor | peor | lo elimina |
| Kd | casi igual | mejor | mejor | sin efecto |

Son tendencias. Los términos se influyen entre sí, así que siempre hay que iterar.

## 6. Cómo sintonizar sin riesgo

**Método manual (el de la web):** Ki = Kd = 0. Sube Kp hasta que empiece a oscilar y déjalo en la mitad. Sube Ki hasta que el error estacionario desaparezca en un tiempo aceptable. Añade un Kd pequeño solo si queda oscilación. Comprueba con perturbaciones reales.

**Ziegler–Nichols:** con Ki = Kd = 0, se busca el Kp que da una oscilación sostenida (Ku) y su periodo (Tu). Después:

| Tipo | Kp | Ki | Kd |
|---|---|---|---|
| P | 0,50·Ku | — | — |
| PI | 0,45·Ku | 0,54·Ku/Tu | — |
| PID | 0,60·Ku | 1,2·Ku/Tu | 0,075·Ku·Tu |

**Importante en el kart:** buscar una oscilación sostenida de guiñada a propósito es peligroso con piloto. Ziegler–Nichols solo debe usarse en el simulador. En pista: método manual, empezando por ganancias bajas, a baja velocidad, en espacio libre y con el USB fuera del kart (registro por Wi-Fi).

## 7. ¿P, PI, PD o PID para el kart?

| Tipo | Uso típico | En el kart |
|---|---|---|
| P | sistemas sencillos que toleran un error | Es el ESC actual: deja error |
| **PI** | velocidad, temperatura, caudal | **Recomendado** para la guiñada: elimina el error y no amplifica el ruido |
| PD | posición con error tolerado | Poco útil aquí |
| PID | drones, robots que se equilibran | Solo si el PI oscila y el ruido está resuelto (PROB-0015) |

## 8. Síntomas y causas, traducidos al kart

| Síntoma | Causa probable | Qué hacer |
|---|---|---|
| El kart «culebrea» en recta o la corrección oscila | Kp o Ki altos | Bajar ganancias |
| Gira siempre algo menos de lo pedido | Falta I, o la zona muerta es grande | Añadir I, reducir la zona muerta |
| Tirón al salir de una curva en suelo resbaladizo | *Windup* | Anti-windup y consigna limitada por adherencia |
| Motores con zumbido o par a tirones | D amplifica ruido de giróscopo o dirección | Bajar Kd, filtrar, D sobre la medida |
| Golpe al mover el volante | «Patada» de la derivada | D sobre la medida |
| La corrección está siempre en el tope y no basta | Falta autoridad del diferencial | No es un problema de ganancias: límite físico del par |

## 9. Lectura del ESC v1.3 como un PID

Fuente: [kart_control.cpp](../../07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.cpp), líneas 125–137, y [kart_control.h](../../07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.h). Configuración: fuente local; la ESP32 física sigue siendo v1.2-diagnostico (ENS-0026). En v1.2 la lógica equivalente está en `legacy_v12.cpp` (`modo_esc`), con otra forma de corrección.

| Elemento | v1.3 | Comentario |
|---|---|---|
| Consigna | `ideal = v·tan(δ)/L` (°/s), L = 1,3 m | Modelo de bicicleta cinemático, sin deslizamiento ni límite de adherencia |
| Medida | `yaw_sign·yaw_dps`, MPU-6050 eje Z, 131 LSB/(°/s) | Rango ±250 °/s. Signo de montaje **sin verificar** |
| P | `error/200` | Kp = 0,005 por °/s |
| Zona muerta | \|e\| ≤ 2 °/s → 0 | Añade error estacionario |
| I, D | no existen | — |
| Saturación | ±0,10, multiplicada por el esfuerzo base | Con pedal bajo hay muy poca autoridad |
| Después | rampa de subida 300 rpm/s, bajadas inmediatas | La rampa limita también las subidas de la corrección |

**Ejemplo numérico de consigna imposible** (cálculo, no medición): v = 10 m/s (36 km/h), δ = 20° (`steer_max_deg`):
`r = 10·tan20°/1,3 = 2,80 rad/s ≈ 160 °/s`, con aceleración lateral `v·r = 28 m/s² ≈ 2,85 g`. Con μ ≈ 1 el máximo físico es `r ≈ μ·g/v ≈ 0,98 rad/s ≈ 56 °/s`. Con P sola esto solo satura. Con I sin anti-windup produciría un tirón al enderezar.

## 10. La IMU: qué mide y qué enseña el visor

**Qué mide una IMU:**

- **Acelerómetro (ax, ay, az):** fuerza específica. En reposo mide la gravedad; en marcha, la gravedad **más** la aceleración del kart. En curva, `ay` es la aceleración lateral (`v·r`): sirve para ver el círculo de adherencia.
- **Giróscopo (gx, gy, gz):** velocidad angular. `gz` es la guiñada, lo único que usa hoy el kart.
- **Magnetómetro (opcional):** campo magnético para el rumbo absoluto. El MPU-6050 no tiene, y en el kart lo perturbarían los motores y las corrientes de la batería.

**El problema de fondo (artículo de orientación):** el acelerómetro da ángulos absolutos pero ruidosos y contaminados por la aceleración. El giróscopo da ángulos suaves pero que **derivan** al integrarlos. La solución es fusionarlos:

| Filtro (visor) | Idea | Para el kart |
|---|---|---|
| Acc/Mag directo | Ángulos solo desde acelerómetro y magnetómetro | Inútil en marcha: la aceleración lateral falsea el alabeo |
| Complementario | `ángulo = α·(ángulo + gyro·dt) + (1−α)·ángulo_acc`, con α ≈ 0,95–0,98 | Sencillo. Basta para alabeo y cabeceo (pendientes) si hiciera falta |
| Madgwick | Descenso de gradiente con cuaterniones, sin bloqueo de cardán | Más robusto en 3D. Útil si se quiere orientación completa |
| EKF (Kalman extendido) | Predice con el giróscopo y corrige con acc/mag, con covarianzas | El más preciso, pero cuesta más sintonizar y calcular |

**Clave para el ESC:** el ESC usa la **velocidad** de guiñada (°/s), no el ángulo. No necesita ningún filtro de orientación. Le basta el giróscopo con buen offset, escala y signo. Los filtros harían falta si algún día se quiere el rumbo o la inclinación (rampas, peralte).

**Lo que muestra el visor:** gráficas en bruto (LSB) del acelerómetro y del giróscopo; alabeo, cabeceo y guiñada filtrados; selector de filtro; selector de rango del giróscopo (±250 °/s = 131,07 LSB/(°/s); ±500 = 65,54; ±1000 = 32,77; ±2000 = 16,38); botones *Demo Mode*, *Reset yaw* y *Mag Cal*. Se conecta por **USB serie** (Web Serial) a 9600–250000 baudios.

**Formato del paquete (20 bytes):** `0xAA 0xFF` + nueve `int16` little-endian: ax, ay, az, gx, gy, gz, mx, my, mz (magnetómetro = 0 si no hay).

**Uso en este proyecto:** como el USB no es accesible con el ESP32 montado (regla del 07-10), el visor solo vale **en banco, con el módulo fuera del kart**. Un firmware de banco que envíe ese paquete permitiría ver el MPU en 3D, medir ruido y sesgo en reposo y comprobar ejes y signo girando la placa a mano. En el kart montado, los datos deben ir por Wi-Fi al panel local, que ya tiene vista 3D.

## 11. Rango del giróscopo para el kart

En una curva normal la guiñada está limitada por la adherencia: unos 56 °/s a 36 km/h con μ ≈ 1, y más a baja velocidad. ±250 °/s sobra para conducir, pero un trompo puede superarlo y la medida quedaría recortada. Opciones para estudiar: ±500 °/s (pierde resolución, 65,5 LSB/(°/s), suficiente para el ESC) o detectar la saturación y marcar `yaw_valid = false`. Es una propuesta, no una decisión.

## 12. Simulador

[[04_CONOCIMIENTO/software_y_telemetria/simulador_pid_imu.html|Abrir el simulador]] (archivo local; se abre con doble clic en el navegador).

- **Muestra:** vista superior con estela; fuerzas de tracción de cada rueda, fuerzas laterales de los neumáticos, la aceleración que siente la IMU, la velocidad y la guiñada; círculo de adherencia (acelerómetro); brújula con el rumbo real frente al integrado del giróscopo; gráficas de consigna, giróscopo, corrección, término I y aceleración lateral.
- **Modos:** DIF, ESC firmware v1.3 (copia de la fórmula), ESC PID (con anti-windup, D sobre la medida y filtro activables) e Igual.
- **Perturbaciones:** μ, motor izquierdo al 80 %, ruido del ADC de dirección, ruido del giróscopo, sesgo y calibración en reposo, empujón de guiñada, rango del giróscopo, fuerza máxima por rueda.
- **Modelo:** bicicleta plana con neumáticos saturables (tanh) y círculo de adherencia trasero. Masa 150 kg, inercia 35 kg·m², rigideces 9000/11000 N/rad y fuerza máxima de 600 N por rueda **supuestas**. Batalla 1,3 m, vía 1,0 m, rueda 0,48 m y 570 rpm tomados del firmware (valores históricos, pendientes de contraste físico).

**Comprobaciones automáticas del 23-10 (simulación sintética, no medida):** «Círculo» (10°, acelerador 30 %, μ 0,9):

| Control | Error medio a los 20 s |
|---|---|
| Firmware v1.3 | 6,4 °/s (consigna 62, real ≈56) |
| PID solo P (Kp 0,006, tope 30 %) | 6,2 °/s |
| PI (Ki 0,02, tope 30 %) | 0,6 °/s, bajando, con una oscilación lenta que se amortigua |

*Windup* (μ 0,5, escalón): integral máxima 4,63 sin anti-windup frente a 0,24 con anti-windup. Sin él, la corrección siguió en el tope varios segundos después de enderezar. Con fuerza de 300 N por rueda y tope del 10 % el PI satura y no corrige: es el experimento de autoridad. Se corrigieron dos fallos del simulador durante la prueba: no se aplicaba el ángulo de dirección a la física y la brújula tenía otro origen de ángulos. Su utilidad es didáctica. Las ganancias no son transferibles al kart sin medir.

## 13. Siguientes pasos propuestos (no ejecutados)

1. Resolver [[05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0017_imu_no_disponible|PROB-0017]]: alimentación, masa, I²C e identidad del módulo.
2. **Banco (módulo fuera del kart):** firmware de lectura que envíe el paquete del visor; registrar sesgo, ruido y deriva térmica en reposo; verificar ejes y `yaw_sign` girando a mano.
3. Medir que el lazo de control corre a periodo fijo y registrar `dt` real.
4. En el simulador, ajustar el modelo con datos reales cuando existan (respuesta a escalón de dirección registrada por Wi-Fi).
5. Diseñar con Armando el PI con anti-windup y consigna limitada; probarlo en PC (pruebas de `kart_control`) antes de cualquier ensayo en el kart.

## Glosario rápido

- **Guiñada (yaw):** giro alrededor del eje vertical; el kart girando en planta.
- **Alabeo (roll) / cabeceo (pitch):** inclinación lateral / longitudinal.
- **Subviraje / sobreviraje:** gira menos / más de lo que indica la dirección.
- **Windup:** la integral se acumula mientras la salida está saturada.
- **Sesgo (bias):** valor que da el giróscopo quieto. Se resta tras calibrar; si cambia con la temperatura, el ángulo integrado deriva.
- **LSB:** unidad mínima del sensor. A ±250 °/s, 1 °/s = 131 LSB.
