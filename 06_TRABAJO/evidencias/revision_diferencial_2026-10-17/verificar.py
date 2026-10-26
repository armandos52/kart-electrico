"""Verifica alcance y procedencia sin alterar las fuentes de producción."""
from pathlib import Path
import hashlib, json, re
from urllib.parse import unquote

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
meta = json.loads((HERE / 'fuentes.json').read_text(encoding='utf-8'))
checks = {}
for item in meta['fuentes']:
    if item['path'].startswith('git:'):
        continue
    current = ROOT / item['path']
    checks['intacto:' + item['path']] = hashlib.sha256(current.read_bytes()).hexdigest() == item['sha256']
legacy = HERE.parent / 'AUD_V12_2026-10-08'
original = (HERE / 'fuentes/ESP32-v12.ino').read_text(encoding='utf-8-sig')
body = ''.join('\n' if line.startswith('#include ') else line for line in original.splitlines(keepends=True))
expected = '#include "stubs.hpp"\n#line 1 "ESP32-final.ino"\n' + body + '\n#include "casos.cpp"\n'
checks['host_v12_equivale_a_git_905b09e_salvo_includes_de_dispositivos'] = expected == (legacy / 'control_host.cpp').read_text(encoding='utf-8')
checks['reverse_speed_limit_solo_definido_una_vez'] = len(re.findall(r'\bREVERSE_SPEED_LIMIT\b', original)) == 1

docnames = [
    '00_INICIO/ESTADO_ACTUAL.md', '06_TRABAJO/TAREAS.md',
    '06_TRABAJO/REVISION_DIFERENCIAL_Y_REVERSA_2026-10-17.md',
    '06_TRABAJO/ensayos/ENS-0025_revision_diferencial_y_reversa.md',
    '06_TRABAJO/evidencias/revision_diferencial_2026-10-17/README.md',
    '05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0014_reversa_y_arquitectura_diferencial.md',
    '05_DECISIONES_Y_PROBLEMAS/problemas/PROB-0015_ruido_electromagnetico_sensores.md',
]
bad_links = []
for name in docnames:
    p = ROOT / name
    for target in re.findall(r'\]\(([^)]+)\)', p.read_text(encoding='utf-8')):
        if '://' in target or target.startswith('#'):
            continue
        target = unquote(target.split('#')[0])
        if not (p.parent / target).exists():
            bad_links.append({'documento': name, 'destino': target})
checks['enlaces_locales_documentos_de_entrega'] = not bad_links
result = {'verificaciones': checks, 'enlaces_rotos': bad_links,
          'nota': 'Ninguna verificacion de hardware; hashes prueban preservacion del alcance de firmware revisado.'}
(HERE / 'verificacion_integridad.json').write_text(json.dumps(result, indent=2, ensure_ascii=False), encoding='utf-8')
files = sorted(p for p in HERE.rglob('*') if p.is_file() and p.name != 'hashes.json')
files += [legacy / name for name in ['control_host.cpp', 'stubs.hpp', 'casos.cpp']]
hashes = {str(p.relative_to(ROOT)).replace('\\', '/'): hashlib.sha256(p.read_bytes()).hexdigest() for p in files}
(HERE / 'hashes.json').write_text(json.dumps(hashes, indent=2, ensure_ascii=False), encoding='utf-8')
print(json.dumps({'verificaciones': len(checks), 'fallos': [k for k, v in checks.items() if not v],
                  'enlaces_rotos': bad_links, 'archivos_con_hash': len(hashes)}, ensure_ascii=False))
raise SystemExit(0 if all(checks.values()) else 1)
