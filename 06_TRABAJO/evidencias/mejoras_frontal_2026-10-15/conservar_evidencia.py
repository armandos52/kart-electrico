"""Conservación de ENS-0018. No sustituye datos históricos ni accede a placas."""
from pathlib import Path
import difflib
import hashlib
import json
import shutil
import subprocess

E=Path(__file__).resolve().parent
ROOT=E.parents[2]
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
paths=[p.relative_to(E/'antes') for p in (E/'antes').rglob('*') if p.is_file()]
paths.append(Path('07_DESARROLLO/firmware/stm32_frontal/tests/display_cases.h'))
manifest={'date_local':'2026-10-15','kind':'PC; sin grabacion',
          'head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
          'sources':{},'artifacts':{}}
diff=[]
for rel in sorted(paths):
    src=ROOT/rel;dst=E/'despues'/rel;dst.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(src,dst)
    before=E/'antes'/rel
    manifest['sources'][rel.as_posix()]={'before':sha(before) if before.exists() else None,'after':sha(dst)}
    diff.extend(difflib.unified_diff(before.read_text(encoding='utf-8').splitlines(True) if before.exists() else [],
        dst.read_text(encoding='utf-8').splitlines(True),fromfile='antes/'+rel.as_posix(),tofile='despues/'+rel.as_posix()))
(E/'cambios.patch').write_text(''.join(diff),encoding='utf-8')
compiled=[('stm32_frontal/.build/stm32_frontal-v1.3-dev4.elf','stm32_frontal-v1.3-dev4.elf'),
          ('stm32_frontal/.build/stm32_frontal-v1.3-dev4.hex','stm32_frontal-v1.3-dev4.hex'),
          ('stm32_frontal/.build/firmware.map','stm32_dev4.map'),
          ('stm32_frontal/.build/hashes.json','hashes_build_stm32.json'),
          ('nucleo_f411_banco/.pio/build/nucleo_f411re/firmware.elf','f411-v0.4-dev.elf'),
          ('nucleo_f411_banco/.pio/build/nucleo_f411re/firmware.bin','f411-v0.4-dev.bin')]
for relative,name in compiled:
    source=ROOT/'07_DESARROLLO/firmware'/relative
    dst=E/'compilacion'/name;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dst)
    if name.startswith('stm32_frontal-v1.3-dev4'):
        backup=ROOT/'07_DESARROLLO/respaldos/stm32/v1.3-dev4'/name
        backup.parent.mkdir(parents=True,exist_ok=True)
        if backup.exists():assert sha(backup)==sha(source),'Respaldo anterior distinto: '+str(backup)
        else:shutil.copy2(source,backup)
for p in E.rglob('*'):
    if p.is_file() and p.suffix not in ('.exe','.obj') and p.name!='manifest.json' and 'antes' not in p.parts and 'despues' not in p.parts:
        manifest['artifacts'][p.relative_to(E).as_posix()]=sha(p)
for relative in ['07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c',
                 '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.cpp',
                 '07_DESARROLLO/firmware/common/kart_protocol.h']:
    manifest.setdefault('unchanged_control_hashes',{})[relative]=sha(ROOT/relative)
(E/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps({'sources':len(paths),'firmware':{k:v for k,v in manifest['artifacts'].items() if k.endswith(('.elf','.hex','.bin'))}},indent=2))
