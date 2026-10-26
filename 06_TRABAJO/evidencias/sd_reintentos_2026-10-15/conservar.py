"""Conserva la revision local ENS-0021. Ejecutar solo tras las compilaciones/pruebas."""
from datetime import datetime
from pathlib import Path
import hashlib
import json
import shutil
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
FRONT = ROOT / '07_DESARROLLO/firmware/stm32_frontal'
BANK = ROOT / '07_DESARROLLO/firmware/nucleo_f411_banco'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def save_json(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

files = ['Core/Src/kart_frontend.c', 'Core/Src/kart_display.c', 'Core/Inc/kart_frontend.h',
         'tests/frontend_test.cpp', 'tests/display_cases.h', 'tests/sd_cases.h',
         'tests/stubs/fatfs.h', 'tests/stubs/main.h', 'tests/compilar.cmd', 'tools/build.py']
for relative in files:
    destination = HERE / 'despues' / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(FRONT / relative, destination)

for name in ['stm32_frontal-v1.3-dev5.elf', 'stm32_frontal-v1.3-dev5.hex', 'firmware.map', 'hashes.json']:
    shutil.copy2(FRONT / '.build' / name, HERE / 'binarios' / name)
for suffix in ['bin', 'elf']:
    shutil.copy2(BANK / '.pio/build/nucleo_f411re' / f'firmware.{suffix}',
                 HERE / 'binarios' / f'f411_0.4-dev_core-dev5.{suffix}')

diff = subprocess.check_output(['git', 'diff', '--',
    '07_DESARROLLO/firmware/stm32_frontal/Core',
    '07_DESARROLLO/firmware/stm32_frontal/tests'], cwd=ROOT)
(HERE / 'cambios.patch').write_bytes(diff)
# sd_cases.h es nuevo sin seguimiento: el snapshot es la fuente completa del archivo.
prior = json.loads((ROOT / '06_TRABAJO/evidencias/frontal_f746_2026-10-15/manifest.json').read_text(encoding='utf-8'))
continuity = {}
for relative in [
    '07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.elf',
    '07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.hex',
    '06_TRABAJO/evidencias/frontal_f746_2026-10-15/relectura/comparacion.txt',
    '06_TRABAJO/evidencias/frontal_f746_2026-10-15/dev4_final/observacion.json']:
    expected = prior['fuentes_y_binario_grabado'].get(relative)
    if expected is None:
        expected = prior['evidencias'][relative]['sha256']
    actual = sha(ROOT / relative)
    continuity[relative] = {'sha256': actual, 'coincide_ENS0019': actual == expected}
    if actual != expected:
        raise RuntimeError(f'Fuente previa modificada: {relative}')
save_json(HERE / 'continuidad.json', continuity)
manifest = {str(p.relative_to(HERE)).replace('\\', '/'):
            {'bytes': p.stat().st_size, 'sha256': sha(p)}
            for p in sorted(HERE.rglob('*')) if p.is_file() and p.name != 'manifest.json'}
save_json(HERE / 'manifest.json', {'ensayo': 'ENS-0021',
    'generado_local': datetime.now().astimezone().isoformat(), 'archivos': manifest})
print(f'Conservados {len(manifest)} archivos; 4 contrastes ENS-0019 coinciden.')
for name, data in manifest.items():
    if name.startswith('binarios/') and name.endswith(('.hex', '.elf', '.bin')):
        print(name, data['sha256'])
