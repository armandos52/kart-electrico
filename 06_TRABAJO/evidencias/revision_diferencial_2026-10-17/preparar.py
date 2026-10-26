"""Conserva fuentes de alcance; no modifica firmware ni conecta a dispositivos."""
from pathlib import Path
import hashlib, json, shutil, subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
SOURCES = [
    '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.h',
    '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.cpp',
    '07_DESARROLLO/firmware/esp32_trasero/tests/control_test.cpp',
    '07_DESARROLLO/firmware/esp32_trasero/src/main.cpp',
    '07_DESARROLLO/firmware/esp32_trasero/include/hardware_kart.h',
    '07_DESARROLLO/firmware/esp32_trasero/platformio.ini',
    '07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c',
    '07_DESARROLLO/firmware/stm32_frontal/Core/Src/main.c',
]
if __name__ == '__main__':
    snapshot = HERE / 'fuentes'
    snapshot.mkdir(exist_ok=False)
    entries = []
    for name in SOURCES:
        # Conservar rutas para los dos main; no copiar secretos ni binarios de placa.
        target = snapshot / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(ROOT / name, target)
        entries.append({'path': name, 'sha256': hashlib.sha256(target.read_bytes()).hexdigest()})
    old = subprocess.check_output(['git', 'show', '905b09e:07_DESARROLLO/firmware/esp32_trasero/src/ESP32-final.ino'], cwd=ROOT)
    (snapshot / 'ESP32-v12.ino').write_bytes(old)
    entries.append({'path': 'git:905b09e:ESP32-final.ino', 'sha256': hashlib.sha256(old).hexdigest()})
    meta = {'head': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
            'tipo': 'PC; fuentes, sin hardware ni modelo dinamico', 'fuentes': entries}
    (HERE / 'fuentes.json').write_text(json.dumps(meta, indent=2), encoding='utf-8')
    (HERE / 'estado_inicial.txt').write_bytes(subprocess.check_output(['git', 'status', '--short'], cwd=ROOT))
    print(json.dumps(meta, indent=2))
