"""Preservacion y comprobacion de evidencia ENS-0016; sin acceso al hardware."""
from pathlib import Path
from datetime import datetime, timezone
import hashlib
import json
import re
import shutil

ev=Path(__file__).resolve().parent
root=ev.parents[2]
bank=root/'07_DESARROLLO/firmware/nucleo_f411_banco'
backup=root/'07_DESARROLLO/respaldos/nucleo_f411/2026-10-14'
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def preserve(source,destination):
    if destination.exists():
        assert sha(source)==sha(destination), f'No sobrescribir evidencia: {destination}'
    else:
        destination.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,destination)

common_paths=[
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
    '07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/screen1_screen/Screen1View.cpp',
    '07_DESARROLLO/firmware/stm32_frontal/TouchGFX/gui/src/common/KartStatus.cpp',
    '07_DESARROLLO/firmware/stm32_frontal/TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp',
]
previous=json.loads((ev.parent/'nucleo_f411_2026-10-14/resultado.json').read_text(encoding='utf-8'))
for path in common_paths:
    p=root/path;preserve(p,ev/'fuentes_comunes'/path)
    if path in previous['sources']: assert sha(p)==previous['sources'][path],f'Fuente de produccion cambio: {path}'
for p in sorted((bank/'src').glob('*')): assert sha(p)==sha(ev/'compilacion_02/src'/p.name)
assert sha(bank/'capturar_pantallas.py')==sha(ev/'compilacion_02/capturar_pantallas.py')
for name in ['capturar_serie.py','README.md']:
    preserve(bank/name,ev/'utilidades'/name)
lib=bank/'.pio/libdeps/nucleo_f411re/MCUFRIEND_kbv'
for name in ['MCUFRIEND_kbv.cpp','MCUFRIEND_kbv.h','utility/mcufriend_shield.h']:
    preserve(lib/name,ev/'biblioteca_lcd'/name)
startup=(ev/'arranque_v03_02.log').read_text(encoding='utf-8')
assert 'VISUAL_CHECKS 54; TOTAL 137; FALLOS 0' in startup
assert len(re.findall(r'\bPASS ',startup))==137
assert len(re.findall(r'LCD_PIXEL.*pass=1',startup))==3
assert 'Download verified successfully' in (ev/'flash_v03_02.log').read_text(encoding='utf-8-sig')
binpath=backup/'vistas_v0.3_02.bin'
assert binpath.stat().st_size==56724
assert sha(binpath)=='fbf86237a4888b81c80508a69d8648d6ae25fa0596848108d972be4f159031a1'
assert sha(binpath)==sha(bank/'.pio/build/nucleo_f411re/firmware.bin')
assert sha(backup/'banco_v0.2.bin')==previous['binaries']['banco_v0.2.bin']['sha256']
for name in ['flash_lectura1.bin','flash_lectura2.bin','flash_cubeprogrammer.bin']:
    assert sha(backup/name)=='8d88e79ec69e18e0d3038e93306b7bb4e4ad0ef1cd04b2f73c3d7b9ce3ac3722'
captures=json.loads((ev/'capturas_finales/manifest.json').read_text())
assert len(captures)==10 and all(r['bytes']==153600 for r in captures)
assert sha(ev/'captura_inicial/escena_03.rgb565')==sha(ev/'capturas_finales/escena_03.rgb565')
transfer=(ev/'capturas_finales/transporte.raw').read_bytes()
assert transfer.count(b'CAPTURE_BEGIN ')==10 and transfer.count(b'CAPTURE_END ')==10
assert b'CAPTURE_ABORT' not in transfer and b'BENCH_F411_' not in transfer
failed=(ev/'capturas/escena_01.rgb565').read_bytes()
assert len(failed)==124513 and b'BENCH_F411_v0.3' in failed
nav=[]
for log in ['arranque_v03_02.log','navegacion_b1_final.log']:
    text=(ev/log).read_text(encoding='utf-8')
    nav.extend(dict(log=log,scene=int(i),uptime_ms=int(ms)) for i,ms in re.findall(r'NAV source=B1 id=(\d+) uptime_ms=(\d+)',text))
summary=dict(utc=datetime.now(timezone.utc).isoformat(),test='ENS-0016',firmware='F411 banco v0.3 / compilacion_02',
    base_git_commit='43b0c299af47e30aabcf9b5acd7677b86754a00b',checks=137,failures=0,original_checks=83,new_checks=54,
    rgb_matches=3,captures_complete=10,capture_bytes_each=153600,capture_source='ILI9341_GRAM',
    repeated_scene_03_identical=True,final_transfer_has_restart=False,physical_button_events=nav,
    physical_wrap_10_to_1_observed=any(a['scene']==9 and b['scene']==0 for a,b in zip(nav,nav[1:]) if a['log']==b['log']),
    interrupted_capture=dict(bytes_received=124513,expected=153600,restart_text_found=True,
        user_statement='Sí, pulsé RESET o desconecté USB',spontaneous_usb_loss_proven=False),
    source_hashes={p:sha(root/p) for p in common_paths},
    binaries={p.name:dict(bytes=p.stat().st_size,sha256=sha(p)) for p in backup.glob('vistas_v0.3_*') if p.suffix in ['.bin','.elf']},
    limitations=['Datos y perifericos del kart simulados','Escenas estaticas, no dinamica fisica',
        'GRAM no es fotografia ni medida de luminancia','Sin prueba de CAN electrico, SD fisica o motores'])
confirmation=ev/'confirmacion_visual_usuario.json'
if confirmation.exists():
    summary['user_visual_confirmation']=json.loads(confirmation.read_text(encoding='utf-8'))
(ev/'resultado.json').write_text(json.dumps(summary,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
manifest={str(p.relative_to(ev)).replace('\\','/'):sha(p) for p in sorted(ev.rglob('*')) if p.is_file() and p.name!='hashes_evidencia.json' and '__pycache__' not in p.parts}
(ev/'hashes_evidencia.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps({k:summary[k] for k in ['checks','failures','captures_complete','repeated_scene_03_identical','physical_wrap_10_to_1_observed']}))
print(f'{len(nav)} eventos B1 fisicos, {len(manifest)} archivos de evidencia; respaldo anterior y fuentes comunes cotejados.')
