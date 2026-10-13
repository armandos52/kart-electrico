"""Conservar fuentes, productos y trazas de ENS-0013 sin leer ni grabar hardware."""
from pathlib import Path
import csv
import hashlib
import json
import re
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FRONT = ROOT / '07_DESARROLLO/firmware/stm32_frontal'
BINARY = ROOT / '07_DESARROLLO/respaldos/stm32/v1.3-dev3'
SOURCES = [
    'Core/Src/kart_frontend.c', 'Core/Inc/kart_frontend.h',
    'tests/frontend_test.cpp', 'tests/compilar.cmd', 'tools/build.py',
]


def preserve(source, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        if destination.read_bytes() != source.read_bytes():
            raise RuntimeError(f'No sobrescribir evidencia distinta: {destination}')
    else:
        shutil.copy2(source, destination)


for name in SOURCES:
    preserve(FRONT / name, HERE / 'despues' / name)
for suffix in ('elf', 'hex'):
    name = f'stm32_frontal-v1.3-dev3.{suffix}'
    preserve(FRONT / '.build' / name, BINARY / name)
preserve(FRONT / '.build/hashes.json', HERE / 'build_hashes.json')
diff = subprocess.run(
    ['git', 'diff', '--no-ext-diff', '--', *[str(FRONT / name) for name in SOURCES]],
    cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True,
)
(HERE / 'cambio_start.diff').write_bytes(diff.stdout)

rows = []
pattern = r'^TRACE tick_ms=(\d+) raw=(\d+) start=(\d+) relay=(\d+) inhibit=(\S+)'
for version, filename in [('antes_dev2', 'frontend_regresion_antes.log'),
                          ('despues_dev3', 'frontend_dev3.log')]:
    for line in (HERE / filename).read_text(encoding='utf-8-sig').splitlines():
        match = re.match(pattern, line)
        if match:
            rows.append(['sintetico_PC', version, *match.groups()])
with (HERE / 'trazas_sinteticas.csv').open('w', encoding='utf-8', newline='') as stream:
    writer = csv.writer(stream)
    writer.writerow(['tipo', 'version', 'tick_ms', 'raw', 'start', 'relay', 'inhibit'])
    writer.writerows(rows)

extra = [
    '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.cpp',
    '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.h',
    '07_DESARROLLO/firmware/stm32_frontal/tests/frontend_test.exe',
    '07_DESARROLLO/firmware/esp32_trasero/tests/control_test.exe',
    '06_TRABAJO/ensayos/ENS-0013_start_stop_momentaneo.md',
    '05_DECISIONES_Y_PROBLEMAS/decisiones/DEC-0008_start_momentaneo_con_memoria.md',
    '05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0009_start_pulsador_momentaneo.md',
]
paths = [p for p in HERE.rglob('*') if p.is_file() and p.name != 'hashes.json']
paths += list(BINARY.glob('*')) + [ROOT / p for p in extra]
manifest = [{'path': p.relative_to(ROOT).as_posix(), 'bytes': p.stat().st_size,
             'sha256': hashlib.sha256(p.read_bytes()).hexdigest()} for p in sorted(set(paths))]
(HERE / 'hashes.json').write_text(json.dumps(manifest, indent=2), encoding='utf-8')
print(f'Evidencia conservada: {len(rows)} trazas sinteticas, {len(manifest)} archivos con SHA-256.')
for entry in manifest:
    if entry['path'].endswith(('.elf', '.hex')):
        print(json.dumps(entry))
