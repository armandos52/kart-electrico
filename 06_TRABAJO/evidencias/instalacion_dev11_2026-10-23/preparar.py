"""Conserva binarios y verifica respaldos/relectura; no escribe en la placa."""
from pathlib import Path
import importlib.util, hashlib, json, shutil, sys
E=Path(__file__).resolve().parent
ROOT=E.parents[2]
FW=ROOT/'07_DESARROLLO/firmware/stm32_frontal'
B=ROOT/'07_DESARROLLO/respaldos/stm32_f746/2026-10-23'
V='stm32_frontal-v1.3-dev11-log'
spec=importlib.util.spec_from_file_location('old',E.parent/'frontal_bugs_2026-10-16/preparar_instalacion.py')
old=importlib.util.module_from_spec(spec);spec.loader.exec_module(old)
def save(p,obj):
    p.write_text(json.dumps(obj,indent=2,ensure_ascii=False),encoding='utf-8');print(json.dumps(obj,indent=2))
def respaldo():
    a=(B/'flash_interna_lectura1.bin').read_bytes()
    assert len(a)==0x100000 and a==(B/'flash_interna_lectura2.bin').read_bytes()
    q=(B/'qspi_lectura1.bin').read_bytes();assert len(q)==0x1000000
    prev=old.ihex(E.parent/'marchas_2026-10-21/binarios/stm32_frontal-v1.3-dev9-ui.hex')
    for addr,data in prev.items():assert (a if addr==0x08000000 else q)[:len(data)]==data
    save(B/'verificacion.json',{'version_identificada':'stm32_frontal-v1.3-dev9-ui','regiones_usadas_iguales':True,'interna_dos_lecturas_iguales':True,'archivos':{p.name:old.sha(p) for p in B.glob('*.bin')}})
def paquete():
    for pat in ['Core/Inc/kart*.h','Core/Src/kart*.c','Core/Src/*.inc','FATFS/Target/*','tests/*.h','tests/*.cpp','tests/stubs/*.h','tools/observar_swd.py']:
        for p in FW.glob(pat):
            if p.is_file():old.copy(p,E/'fuentes'/p.relative_to(FW))
    old.copy(FW.parent/'common/kart_log_protocol.h',E/'fuentes/common/kart_log_protocol.h')
    for n in [V+'.elf',V+'.hex','firmware.map','hashes.json']:old.copy(FW/'.build'/n,E/'binarios'/n)
    regions=old.ihex(E/'binarios'/(V+'.hex'))
    save(E/'identidad.json',{'version':V,'elf_sha256':old.sha(E/'binarios'/(V+'.elf')),'hex_sha256':old.sha(E/'binarios'/(V+'.hex')),'regiones':{hex(b):len(d) for b,d in regions.items()}})
    cfg=(E.parent/'marchas_2026-10-21/grabar_dev9.cfg').read_text()
    cfg=cfg.replace('ENS-0026','ENS-0029').replace('Snapshot dev8','Snapshot dev9').replace((E.parent/'marchas_2026-10-21/binarios/stm32_frontal-v1.3-dev9-ui.hex').as_posix(),(E/'binarios'/(V+'.hex')).as_posix())
    (E/'grabar.cfg').write_text(cfg,encoding='ascii')
    cfg=(E/'respaldar.cfg').read_text().split('set R {')[0]
    cfg+=f'set D {{{(E/"relectura").as_posix()}}}\nfile mkdir $D\n'
    for b,d in regions.items():cfg+=f'dump_image "$D/region_{b:08x}.bin" {b:#x} {len(d):#x}\n'
    (E/'releer.cfg').write_text(cfg+'echo RELECTURA_FIN\nshutdown\n',encoding='ascii')
def relectura():
    result={}
    for b,d in old.ihex(E/'binarios'/(V+'.hex')).items():
        p=E/'relectura'/f'region_{b:08x}.bin';assert p.read_bytes()==d
        result[hex(b)]={'bytes':len(d),'diferencias':0,'sha256':old.sha(p)}
    save(E/'relectura/verificacion.json',result)
if __name__=='__main__':{'respaldo':respaldo,'paquete':paquete,'relectura':relectura}[sys.argv[1]]()
