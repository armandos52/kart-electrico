"""Preserva fuentes y manifiestos de ENS-0015; no accede a hardware."""
from pathlib import Path
import hashlib
import json
import re
import shutil
from datetime import datetime, timezone

ev=Path(__file__).resolve().parent
root=ev.parents[2]
project=root/'07_DESARROLLO/firmware/nucleo_f411_banco'
backup=root/'07_DESARROLLO/respaldos/nucleo_f411/2026-10-14'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

sources=[*sorted((project/'src').glob('*')),project/'platformio.ini',project/'capturar_serie.py']
sources += [root/p for p in [
    '07_DESARROLLO/firmware/stm32_frontal/tests/frontend_test.cpp',
    '07_DESARROLLO/firmware/stm32_frontal/tests/stubs/main.h',
    '07_DESARROLLO/firmware/stm32_frontal/tests/stubs/fatfs.h',
    '07_DESARROLLO/firmware/stm32_frontal/tests/stubs/cmsis_os.h',
    '07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_frontend.c',
    '07_DESARROLLO/firmware/stm32_frontal/Core/Src/kart_display.c',
    '07_DESARROLLO/firmware/stm32_frontal/Core/Inc/kart_frontend.h',
    '07_DESARROLLO/firmware/stm32_frontal/Core/Inc/kart_display.h',
    '07_DESARROLLO/firmware/common/kart_protocol.h',
    '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.cpp',
    '07_DESARROLLO/firmware/esp32_trasero/lib/kart_control/kart_control.h',
]]
for source in sources:
    dest=ev/'fuentes_banco_v02'/source.relative_to(root)
    if dest.exists() and sha(dest)!=sha(source):
        raise RuntimeError(f'No sobrescribir fuente conservada: {dest}')
    dest.parent.mkdir(parents=True,exist_ok=True)
    if not dest.exists():shutil.copy2(source,dest)

# Conservar licencias/manifiestos de las dependencias usadas; los paquetes son locales.
for library in ['MCUFRIEND_kbv','Adafruit GFX Library','Adafruit BusIO']:
    lib=project/'.pio/libdeps/nucleo_f411re'/library
    for f in lib.iterdir():
        if f.is_file() and (f.name.lower().startswith('license') or f.name in ['library.properties','library.json']):
            dest=ev/'dependencias'/library/f.name
            dest.parent.mkdir(parents=True,exist_ok=True)
            if dest.exists() and sha(dest)!=sha(f):raise RuntimeError(dest)
            if not dest.exists():shutil.copy2(f,dest)

startup=(ev/'serie_banco_v02_arranque.log').read_text(encoding='utf-8')
button=(ev/'serie_banco_v02_boton.log').read_text(encoding='utf-8')
states=[dict(zip(['ms','raw','start','relay','inhibit','checks','failures'],map(int,m))) for m in re.findall(
    r'STATE ms=(\d+) raw=(\d+) start=(\d+) relay=(\d+) inhibit=(\d+) checks=(\d+) failures=(\d+)',button)]
edges=[]
prev=0
for s in states:
    if s['start']!=prev:edges.append(s)
    prev=s['start']
assert 'TOTAL 83; FALLOS 0' in startup
assert len(re.findall(r'\bPASS ',startup))==83
assert len(re.findall(r'LCD_PIXEL.*pass=1',startup))==3
assert [s['start'] for s in edges]==[1,0,1,0,1]
assert any(s['relay']==3 and s['raw']==0 for s in states)
originals=[backup/name for name in ['flash_lectura1.bin','flash_lectura2.bin','flash_cubeprogrammer.bin']]
assert all(p.stat().st_size==524288 and sha(p)=='8d88e79ec69e18e0d3038e93306b7bb4e4ad0ef1cd04b2f73c3d7b9ce3ac3722' for p in originals)
assert sha(backup/'banco_v0.2.bin')==sha(project/'.pio/build/nucleo_f411re/firmware.bin')
summary={'utc':datetime.now(timezone.utc).isoformat(),'physical_board':'NUCLEO-F411RE',
    'bench_firmware':'v0.2','lcd_id':'0x9341','checks':83,'failures':0,'lcd_rgb_matches':3,
    'synthetic_peripherals':['CAN','ADC','SD','Hall','MPU','relays'],
    'button_records':len(states),'start_edges':edges,'complete_on_off_cycles_captured':2,
    'third_cycle_ready_captured':True,'third_cycle_stop_captured':False,
    'user_confirmation':'Se ve bien y START cambia ON/OFF',
    'base_git_commit':'43b0c299af47e30aabcf9b5acd7677b86754a00b',
    'sources':{str(p.relative_to(root)).replace('\\','/'):sha(p) for p in sources},
    'binaries':{p.name:{'bytes':p.stat().st_size,'sha256':sha(p)} for p in backup.glob('*') if p.suffix in ['.bin','.elf']}}
(ev/'resultado.json').write_text(json.dumps(summary,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
manifest={str(p.relative_to(ev)).replace('\\','/'):sha(p) for p in sorted(ev.rglob('*')) if p.is_file() and p.name!='hashes_evidencia.json'}
(ev/'hashes_evidencia.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps({k:summary[k] for k in ['checks','failures','lcd_rgb_matches','button_records','complete_on_off_cycles_captured','third_cycle_ready_captured']},ensure_ascii=False))
print('Fuentes, binarios y registros cotejados; manifiesto de evidencia generado.')
