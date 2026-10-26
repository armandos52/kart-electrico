"""Conservación/validación ENS-0026; no accede al hardware."""
import difflib
import hashlib
import importlib.util
import json
import shutil
import sys
from pathlib import Path

E=Path(__file__).resolve().parent
ROOT=E.parents[2]
FW=ROOT/'07_DESARROLLO/firmware/stm32_frontal'
BACK=ROOT/'07_DESARROLLO/respaldos/stm32_f746/2026-10-21'
PREV=E.parent/'bms_dashboard_2026-10-16'
VERSION='stm32_frontal-v1.3-dev9-ui'
spec=importlib.util.spec_from_file_location('old',E.parent/'frontal_bugs_2026-10-16/preparar_instalacion.py')
old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)

def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path,data): path.write_text(json.dumps(data,indent=2,ensure_ascii=False),encoding='utf-8');print(json.dumps(data,indent=2))

def paquete():
    for pattern in ['Core/Inc/kart*.h','Core/Src/kart*.c','tests/*.h','tests/*.cpp','tests/*.cmd','tests/stubs/*.h','tools/registrar_marcha_swd.py']:
        for p in FW.glob(pattern): old.copy(p,E/'despues'/p.relative_to(FW))
    patch=[]
    for p in (E/'antes').iterdir():
        target=next((q for q in (E/'despues').rglob(p.name)),None)
        if target:
            patch.extend(difflib.unified_diff(p.read_text(encoding='utf-8-sig').splitlines(True),target.read_text(encoding='utf-8-sig').splitlines(True),fromfile='antes/'+p.name,tofile='despues/'+target.relative_to(E/'despues').as_posix()))
    (E/'cambios.patch').write_text(''.join(patch),encoding='utf-8')
    for name in [VERSION+'.elf',VERSION+'.hex','firmware.map','hashes.json']: old.copy(FW/'.build'/name,E/'binarios'/name)
    regions=old.ihex(E/'binarios'/(VERSION+'.hex'))
    prev=old.ihex(PREV/'binarios/stm32_frontal-v1.3-dev8-ui.hex')
    assert regions[0x90000000]==prev[0x90000000]
    save(E/'identidad.json',{'version':VERSION,'qspi_identica_dev8':True,'elf_sha256':sha(E/'binarios'/(VERSION+'.elf')),'hex_sha256':sha(E/'binarios'/(VERSION+'.hex')),'regiones':{hex(b):len(d) for b,d in regions.items()}})
    cfg=(PREV/'grabar_dev8.cfg').read_text().replace('ENS-0024','ENS-0026').replace((PREV/'binarios/stm32_frontal-v1.3-dev8-ui.hex').as_posix(),(E/'binarios'/(VERSION+'.hex')).as_posix())
    guard='''# Snapshot dev8 identificado en respaldo; exigir OFF, N y pedal no pisado.
foreach address {0x20000892 0x20000893 0x200008bc 0x200008bd} {
    if {[lindex [read_memory $address 8 1] 0] != 0} {error "START/marcha/selector no OFF-N: cancelar"}
}
if {[lindex [read_memory 0x20000878 16 1] 0] > 320} {error "Pedal no en zona cero: cancelar"}
'''
    cfg=cfg.replace('program {',guard+'program {',1)
    (E/'grabar_dev9.cfg').write_text(cfg,encoding='ascii')
    cfg=(E/'respaldar.cfg').read_text().split('set R {')[0]
    cfg+=f'set D {{{(E/"relectura").as_posix()}}}\nfile mkdir $D\n'
    for b,d in regions.items(): cfg+=f'dump_image "$D/region_{b:08x}.bin" {b:#x} {len(d):#x}\n'
    (E/'releer_dev9.cfg').write_text(cfg+'echo "RELECTURA_FIN"\nshutdown\n',encoding='ascii')

def respaldo():
    a,b=BACK/'flash_interna_lectura1.bin',BACK/'flash_interna_lectura2.bin'
    assert a.stat().st_size==b.stat().st_size==0x100000 and a.read_bytes()==b.read_bytes()
    q=BACK/'qspi_lectura1.bin';assert q.stat().st_size==0x1000000
    for base,data in old.ihex(PREV/'binarios/stm32_frontal-v1.3-dev8-ui.hex').items():
        actual=a if base==0x08000000 else q
        assert actual.read_bytes()[:len(data)]==data,hex(base)
    save(BACK/'verificacion.json',{'firmware':'stm32_frontal-v1.3-dev8-ui, coincidencia byte a byte regiones HEX','interna_dos_lecturas_iguales':True,'qspi_igual_respaldo_completo_16_oct':sha(q)==sha(ROOT/'07_DESARROLLO/respaldos/stm32_f746/2026-10-16/qspi_lectura1.bin'),'archivos':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p)} for p in BACK.glob('*.bin')}})

def relectura():
    result={}
    for b,d in old.ihex(E/'binarios'/(VERSION+'.hex')).items():
        p=E/'relectura'/f'region_{b:08x}.bin';assert p.read_bytes()==d,hex(b)
        result[hex(b)]={'bytes':len(d),'diferencias':0,'sha256':sha(p)}
    save(E/'relectura/verificacion.json',result)

def manifest():
    data={p.relative_to(E).as_posix():{'bytes':p.stat().st_size,'sha256':sha(p)} for p in sorted(E.rglob('*')) if p.is_file() and p.name!='manifest.json' and p.suffix not in ['.pyc','.obj','.exe']}
    (E/'manifest.json').write_text(json.dumps(data,indent=2),encoding='utf-8');print(len(data),'archivos con SHA-256')

if __name__=='__main__': {'paquete':paquete,'respaldo':respaldo,'relectura':relectura,'manifest':manifest}[sys.argv[1]]()
