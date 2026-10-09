"""Auditoría aislada: compila el .ino real con periféricos simulados, sin modificarlo.

No valida tiempos, buses, drivers, compilador Xtensa ni respuesta física de motores.
"""
from pathlib import Path
import hashlib
import json

OUT = Path(__file__).resolve().parent
ROOT = OUT.parents[2]
SRC = OUT / 'fuentes_v12/src/ESP32-final.ino'  # referencia verificada, no firmware actual
source = SRC.read_text(encoding='utf-8-sig')
# Solo se sustituyen includes de dispositivos por stubs; cuerpos de funciones intactos.
lines = source.splitlines(keepends=True)
body = ''.join('\n' if line.startswith('#include ') else line for line in lines)
generated = '#include "stubs.hpp"\n#line 1 "ESP32-final.ino"\n' + body
generated += '\n#include "casos.cpp"\n'
(OUT / 'control_host.cpp').write_text(generated, encoding='utf-8')
p25 = ROOT / '02_ORIGINALES/entrega_2026-09-28/Smart Eletric Kart Cross-2223406-2223403/Smart Eletric Kart Cross-2223406-2223403'
files = [SRC, SRC.parent / 'telemetria.cpp', SRC.parents[1] / 'include/telemetria.h',
         SRC.parents[1] / 'platformio.ini', SRC.parents[1] / 'lib/mcp_can/mcp_can.cpp',
         p25 / 'STM32-final/Core/Src/main.c',
         p25 / 'STM32-final/TouchGFX/gui/src/screen1_screen/Screen1View.cpp',
         p25 / 'STM32-final/TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp',
         OUT / 'generar.py', OUT / 'stubs.hpp', OUT / 'casos.cpp', OUT / 'control_host.cpp']
manifest = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(OUT / 'hashes_reproduccion.json').write_text(json.dumps(manifest, indent=2, ensure_ascii=False)+'\n', encoding='utf-8')
print('Generado control_host.cpp; funciones originales sin editar; dispositivos simulados.')
