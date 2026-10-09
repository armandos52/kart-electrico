# ARM GNU portable para el frontal

09-10-2026. ARM GNU 14.3.Rel1, GCC 14.3.1 20250623. El proyecto histórico ya declara 14.3.rel1 en sus makefiles. Descarga oficial: [ZIP Windows](https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-mingw-w64-i686-arm-none-eabi.zip).

Archivo de 291670457 bytes, SHA-256 `836ebe51fd71b6542dd7884c8fb2011192464b16c28e4b38fddc9350daba5ee8`. MD5 coincide con Content-MD5 de la respuesta oficial: `a3fafaa5fcfe34e9bd30df616316813e`. Extracción de 7665 entradas comprobando permanencia dentro de esta carpeta; binarios y ZIP excluidos de Git. No modifica PATH global ni instala servicios.

El compilador se encuentra en `bin/arm-none-eabi-gcc.exe`. El script `firmware/stm32_frontal/tools/build.py --toolchain <bin>` admite otra ubicación explícita. No se graba hardware.
