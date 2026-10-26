"""Conserva el resultado local ENS-0022; no graba ni modifica fuentes de firmware."""
from pathlib import Path
import difflib
import hashlib
import json
import shutil
import subprocess

E = Path(__file__).resolve().parent
ROOT = E.parents[2]
FW = ROOT/'07_DESARROLLO/firmware/stm32_frontal'
VERSION = 'stm32_frontal-v1.3-dev6-ui'

def copy(source, target):
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target)

def main():
    files={FW/'.gitignore', FW/'README_ROBUSTEZ.md', FW/'ORIGEN_V0.json'}
    for pattern in ('Core/Inc/kart*.h','Core/Src/kart*.c','TouchGFX/gui/**/*.cpp','TouchGFX/gui/**/*.hpp',
                    'tests/**/*.h','tests/*.cpp','tests/*.cmd','tests/*.py','tools/*.py','tools/*.ps1','assets/dashboard/*'):
        files.update(FW.glob(pattern))
    for file in sorted(files):
        if file.is_file():copy(file,E/'despues'/file.relative_to(FW))
    # La base del formateador dev5 está documentada en ENS-0021; no sustituir la fuente histórica.
    base=ROOT/'06_TRABAJO/evidencias/sd_reintentos_2026-10-15/despues/Core/Src/kart_display.c'
    if not (E/'antes/Core/Src/kart_display.c').exists():copy(base,E/'antes/Core/Src/kart_display.c')
    patch=[]
    for old in sorted((E/'antes').rglob('*')):
        if not old.is_file():continue
        rel=old.relative_to(E/'antes');new=E/'despues'/rel
        if new.exists():patch.extend(difflib.unified_diff(old.read_text(encoding='utf-8-sig').splitlines(True),new.read_text(encoding='utf-8-sig').splitlines(True),fromfile='antes/'+rel.as_posix(),tofile='despues/'+rel.as_posix()))
    (E/'cambios.patch').write_text(''.join(patch),encoding='utf-8')
    for name in (VERSION+'.elf',VERSION+'.hex','firmware.map','hashes.json'):
        copy(FW/'.build'/name,E/'binarios'/name)
    bank=ROOT/'07_DESARROLLO/firmware/nucleo_f411_banco/.pio/build/nucleo_f411re'
    for suffix in ('bin','elf'):copy(bank/('firmware.'+suffix),E/'binarios'/('f411-v0.4-dev-con-dev6-ui.'+suffix))
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
    (E/'identidad.json').write_text(json.dumps({'head_base':head,'version':VERSION,'instalado_f746':'dev4, última evidencia ENS-0019',
        'tipo':'Compilación/pruebas PC; sin flash ni hardware','font_ttf_origin':'C:/Windows/Fonts/DejaVuSans.ttf; licencia conservada'},indent=2,ensure_ascii=False),encoding='utf-8')
    manifest={str(p.relative_to(E)).replace('\\','/'):{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
              for p in sorted(E.rglob('*')) if p.is_file() and p.name!='manifest.json' and p.suffix not in ('.exe','.obj','.pyc')}
    (E/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False),encoding='utf-8')
    print(f'{len(manifest)} archivos documentados')
    for name in (VERSION+'.elf',VERSION+'.hex'):print(name,manifest['binarios/'+name]['sha256'])

if __name__=='__main__':main()
