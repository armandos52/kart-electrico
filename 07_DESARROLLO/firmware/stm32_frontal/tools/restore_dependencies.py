"""Después de clonar: repone solo archivos ausentes desde la entrega verificada."""
from pathlib import Path
import hashlib,json,shutil
DEST=Path(__file__).resolve().parents[1]
ROOT=DEST.parents[2]
manifest=json.loads((DEST/'ORIGEN_V0.json').read_text(encoding='utf-8'))
source=ROOT/manifest['source']
count=0
for entry in manifest['files']:
    dest=DEST/entry['path']
    if dest.exists():continue # Nunca sobrescribir desarrollo, ni regenerar desde v0.
    # Screen2 eliminado deliberadamente: no resucitar restos de código.
    if entry['path'].replace('\\','/').startswith('TouchGFX/gui/include/gui/screen2_screen/'):continue
    original=source/entry['path']
    if hashlib.sha256(original.read_bytes()).hexdigest()!=entry['sha256']:raise RuntimeError(original)
    dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(original,dest);count+=1
print(f'Repuestos {count} archivos ausentes; originales y desarrollo conservados.')
