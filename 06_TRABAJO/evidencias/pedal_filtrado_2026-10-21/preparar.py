"""ENS-0027: paquete local; no conexión ni programación de hardware."""
from pathlib import Path
import csv, difflib, hashlib, importlib.util, json, shutil, statistics, sys

E=Path(__file__).resolve().parent
ROOT=E.parents[2]
FW=ROOT/'07_DESARROLLO/firmware/stm32_frontal'
OLD=E.parent/'marchas_2026-10-21'
VERSION='stm32_frontal-v1.3-dev10-ui'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def copy(p,q): q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
def save(p,obj): p.write_text(json.dumps(obj,ensure_ascii=False,indent=2),encoding='utf-8')

def package():
    for pattern in ['Core/Inc/kart*.h','Core/Src/kart*.c','tests/*.h','tests/*.cpp','tests/*.cmd','tests/stubs/*.h','tools/build.py']:
        for p in FW.glob(pattern):copy(p,E/'fuentes/stm32_frontal'/p.relative_to(FW))
    for p in (FW.parent/'common').glob('*.h'):copy(p,E/'fuentes/common'/p.name)
    for p in (FW.parent/'esp32_trasero/lib/kart_control').glob('*'):
        if p.suffix in ('.h','.cpp'):copy(p,E/'fuentes/esp32_trasero/lib/kart_control'/p.name)
    for name in [VERSION+'.elf',VERSION+'.hex','firmware.map','hashes.json']:copy(FW/'.build'/name,E/'binarios'/name)
    patch=[]
    for p in (E/'fuentes/stm32_frontal').rglob('*'):
        old=OLD/'despues'/p.relative_to(E/'fuentes/stm32_frontal')
        if p.is_file() and p.suffix in ('.c','.h','.cpp'):
            before=old.read_text(encoding='utf-8-sig').splitlines(True) if old.exists() else []
            patch.extend(difflib.unified_diff(before,p.read_text(encoding='utf-8-sig').splitlines(True),fromfile='dev9/'+str(p.relative_to(E/'fuentes')),tofile='dev10/'+str(p.relative_to(E/'fuentes'))))
    (E/'cambios.patch').write_text(''.join(patch),encoding='utf-8')
    spec=importlib.util.spec_from_file_location('old',OLD/'preparar.py')
    mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
    regions=mod.old.ihex(E/'binarios'/(VERSION+'.hex'))
    previous=mod.old.ihex(OLD/'binarios/stm32_frontal-v1.3-dev9-ui.hex')
    save(E/'identidad.json',{'version':VERSION,'instalada':False,'elf_sha256':sha(E/'binarios'/(VERSION+'.elf')),'hex_sha256':sha(E/'binarios'/(VERSION+'.hex')),'qspi_identica_dev9':regions[0x90000000]==previous[0x90000000],'regiones':{hex(b):len(d) for b,d in regions.items()}})

def analysis():
    summary={}
    with (E/'reposo_replay.csv').open('w',newline='',encoding='utf-8') as f:
        w=csv.writer(f);w.writerow(['fase','tick_ms_original','adc_original'])
        for phase in ['reposo_OFF_N_2','retorno_1','transicion_selector_freno','dev9_reposo']:
            p=OLD/phase/'muestras.csv'
            rows=list(csv.DictReader(p.open(encoding='utf-8-sig')))
            values=[int(r['accel_adc']) for r in rows]
            summary[phase]={'fuente':str(p.relative_to(ROOT)),'sha256':sha(p),'n':len(rows),'min_adc':min(values),'max_adc':max(values),'media_adc':statistics.mean(values),'sigma_adc':statistics.pstdev(values),'duracion_s':float(rows[-1]['t_s'])-float(rows[0]['t_s'])}
            for r in rows:w.writerow([phase,r['tick'],r['accel_adc']])
    save(E/'datos_originales_resumen.json',summary)

def manifest():
    save(E/'manifest.json',{p.relative_to(E).as_posix():{'bytes':p.stat().st_size,'sha256':sha(p)} for p in sorted(E.rglob('*')) if p.is_file() and p.name!='manifest.json' and p.suffix not in ('.exe','.obj','.pyc')})

if __name__=='__main__':{'package':package,'analysis':analysis,'manifest':manifest}[sys.argv[1]]()
