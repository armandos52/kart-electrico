"""ENS-0024: conservar dev8-ui y preparar grabacion completa, sin acceso al hardware."""
from pathlib import Path
import difflib
import hashlib
import importlib.util
import json
import shutil
import sys

E=Path(__file__).resolve().parent
ROOT=E.parents[2]
FW=ROOT/'07_DESARROLLO/firmware/stm32_frontal'
PREV=E.parent/'frontal_bugs_2026-10-16'
VERSION='stm32_frontal-v1.3-dev8-ui'
spec=importlib.util.spec_from_file_location('previous',PREV/'preparar_instalacion.py')
previous=importlib.util.module_from_spec(spec);spec.loader.exec_module(previous)

def manifest():
    files={p.relative_to(E).as_posix():{'bytes':p.stat().st_size,'sha256':previous.sha(p)}
           for p in sorted(E.rglob('*')) if p.is_file() and p.name!='manifest.json' and p.suffix not in ('.pyc','.exe','.obj')}
    (E/'manifest.json').write_text(json.dumps(files,indent=2),encoding='utf-8')
    print(f'{len(files)} archivos documentados')

def archive():
    for pattern in ('Core/Inc/kart*.h','Core/Src/kart*.c','TouchGFX/gui/**/*.cpp','TouchGFX/gui/**/*.hpp',
                    'tests/*.h','tests/*.cpp','tests/*.cmd','tests/stubs/*.h'):
        for source in FW.glob(pattern):previous.copy(source,E/'despues'/source.relative_to(FW))
    diff=[]
    for old in (E/'antes').rglob('*'):
        if not old.is_file():continue
        rel=old.relative_to(E/'antes');new=E/'despues'/rel
        diff.extend(difflib.unified_diff(old.read_text(encoding='utf-8-sig').splitlines(True),
                   new.read_text(encoding='utf-8-sig').splitlines(True),fromfile='antes/'+rel.as_posix(),tofile='despues/'+rel.as_posix()))
    (E/'cambios.patch').write_text(''.join(diff),encoding='utf-8')
    for name in (VERSION+'.elf',VERSION+'.hex','firmware.map','hashes.json'):
        previous.copy(FW/'.build'/name,E/'binarios'/name)
    regions=previous.ihex(E/'binarios'/(VERSION+'.hex'))
    old_regions=previous.ihex(PREV/'binarios/stm32_frontal-v1.3-dev7-ui.hex')
    assert regions[0x90000000]==old_regions[0x90000000], 'Recursos QSPI cambiados: revisar'
    cfg=(PREV/'grabar_dev7.cfg').read_text().replace('ENS-0023','ENS-0024').replace(
        (PREV/'binarios/stm32_frontal-v1.3-dev7-ui.hex').as_posix(),(E/'binarios'/(VERSION+'.hex')).as_posix())
    (E/'grabar_dev8.cfg').write_text(cfg,encoding='ascii')
    cfg=(PREV/'respaldar.cfg').read_text().split('set R {')[0]
    cfg+=f'set D {{{(E/"relectura").as_posix()}}}\nfile mkdir $D\n'
    for base,data in regions.items():cfg+=f'dump_image "$D/region_{base:08x}.bin" {base:#x} {len(data):#x}\n'
    (E/'releer_dev8.cfg').write_text(cfg+'echo "RELECTURA_FIN"\nshutdown\n',encoding='ascii')
    identity={'version':VERSION,'base':'dev7-ui instalada y releida en ENS-0023','qspi_sin_cambios':True,
              'hex_sha256':previous.sha(E/'binarios'/(VERSION+'.hex')),
              'elf_sha256':previous.sha(E/'binarios'/(VERSION+'.elf')),
              'regiones':{hex(base):len(data) for base,data in regions.items()}}
    (E/'identidad.json').write_text(json.dumps(identity,indent=2),encoding='utf-8');print(json.dumps(identity,indent=2))

def verify():
    previous.E=E;previous.VERSION=VERSION;previous.verify_new()

if __name__=='__main__':{'archive':archive,'verify':verify,'manifest':manifest}[sys.argv[1]]()
