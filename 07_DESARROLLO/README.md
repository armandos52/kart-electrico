# Desarrollo y verificación

Entrega para revisión académica del 13-10-2026. Los ensayos automáticos siguientes se ejecutan en el ordenador sin conectar ni programar placas.

| Componente | Entrada | Estado |
|---|---|---|
| Panel Python + JavaScript | [README y arranque](panel_local/README.md) | 0.4.0; diagnóstico, gráficas, registro y calibración visual |
| ESP32 / PlatformIO | [README](firmware/esp32_trasero/README.md) | v1.3-dev3; compilado, aceptación física pendiente |
| STM32 / TouchGFX | [README](firmware/stm32_frontal/README_ROBUSTEZ.md) | v1.3-dev3; compilado con dependencias locales; instalado sin identificar |
| CAN y telemetría | [Contrato v1.3](firmware/common/PROTOCOLO_V13.md), [protocolo del panel](panel_local/PROTOCOLO.md) | Unidades, validez y límites del contrato |
| Investigación 2026/27 | [Propuesta](propuesta_2026-27/Proposta_Smart_Electric_Kart_V2_2026-27.docx), [cronograma resumido](propuesta_2026-27/Cronograma_Smart_Electric_Kart_proposta_resumida.docx) | Alcance y criterios a acordar con orientadores |

## Pruebas desde una copia limpia

Requisitos: Python 3.12 o posterior, Node.js 22 o posterior y un compilador C++17 (GCC en Linux o MSVC Build Tools en Windows). El panel utiliza `pyserial==3.5` para adquirir datos; la demo no necesita hardware.

Desde la raíz, con `python` apuntando al intérprete seleccionado:

```sh
python -m pip install -r 07_DESARROLLO/panel_local/requirements.txt
python -m unittest discover -s 07_DESARROLLO/panel_local/tests -v
node 07_DESARROLLO/panel_local/qa/check-robustez.mjs
node 07_DESARROLLO/panel_local/qa/check-velocidad.mjs
node 07_DESARROLLO/panel_local/qa/check-idiomas.mjs
python 07_DESARROLLO/herramientas/verificar_publicacion.py
```

Si el intérprete tiene otro nombre/ruta, definir `PYTHON` para la comprobación de idiomas. En Windows, ejecutar `compilar.cmd` desde cada carpeta `firmware/esp32_trasero/tests` y `firmware/stm32_frontal/tests`. Los comandos GCC equivalentes están en el [workflow](../.github/workflows/verificacion.yml).

El workflow repite Python, JavaScript, lógica C++ y navegación documental desde el repositorio público. No compila imágenes completas de placas ni valida comportamiento físico. [Ensayos registrados](../06_TRABAJO/ensayos/README.md).

Las verificaciones de navegador vigentes y sus requisitos se explican en [qa/README](panel_local/qa/README.md). Los verificadores antiguos se conservan como antecedentes; no se presentan como controles del flujo actual.

## Compilar firmware

ESP32: instalar el entorno fijado en su README, copiar `include/secretos.example.h` a `include/secretos.h` y definir credenciales locales antes de compilar con PlatformIO. `secretos.h` está excluido de Git. Compilar no graba hardware.

STM32: disponer de la entrega histórica local, restaurar dependencias por hash y ejecutar `tools/build.py` con ARM GNU 14.3.Rel1. El repositorio público contiene la lógica y configuración propias; los recursos heredados que no se distribuyen están identificados en `ORIGEN_V0.json`.

Consultar [VERSIONES](firmware/VERSIONES.md) y cada ensayo antes de elegir una imagen. USB ESP32 solo fuera del kart; adquisición integrada por Wi-Fi. Frontal y QSPI pendientes de identificación/respaldo previo.

## Procedencia

El sistema hereda código y diseños académicos anteriores: [inventario](../04_CONOCIMIENTO/software_y_telemetria/INVENTARIO_CODIGO.md). Las copias, cambios y límites se registran en Git y en el diario; no se atribuye todo el sistema a esta continuación.

Three.js r180 conserva [MIT y procedencia](panel_local/static/vendor/README.md); mcp_can conserva su [licencia](firmware/esp32_trasero/lib/mcp_can/license.txt) y la modificación de DLC documentada; HAL IWDG conserva [licencia y origen](firmware/stm32_frontal/Drivers/STM32F7xx_HAL_Driver/LICENSE_IWDG.md). No se asigna en esta entrega una licencia global nueva al material académico heredado.
