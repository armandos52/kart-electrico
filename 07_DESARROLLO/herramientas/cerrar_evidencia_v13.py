"""Conserva entregables v1.3-dev2 ya compilados; no compila ni graba hardware."""
from pathlib import Path
import hashlib,json,shutil,subprocess

ROOT=Path(__file__).resolve().parents[2]
DATA=ROOT/'06_TRABAJO/ensayos/datos/ENS-0009'
if (DATA/'hashes_dev2.json').exists():
    raise SystemExit('La evidencia dev2 ya existe; no sobrescribir. Crear otra versión para nuevos cambios.')
ESP=ROOT/'07_DESARROLLO/firmware/esp32_trasero'
STM=ROOT/'07_DESARROLLO/firmware/stm32_frontal'
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
manifest={'version':'v1.3-dev2','physical_test':False,'flashed':False,'date':'2026-10-09','files':{}}
for node,folder,names in [
    ('esp32',ESP/'.pio/build/usb',['firmware.bin','firmware.elf','bootloader.bin','partitions.bin']),
    ('stm32',STM/'.build',['stm32_frontal-v1.3-dev2.elf','stm32_frontal-v1.3-dev2.hex'])]:
    dest=ROOT/'07_DESARROLLO/respaldos'/node/'v1.3-dev2'
    dest.mkdir(parents=True,exist_ok=True)
    (dest/'.gitignore').write_text('*\n!.gitignore\n')
    for name in names:
        original=folder/name;backup=dest/name
        if backup.exists() and sha(backup)!=sha(original):raise RuntimeError('No sobrescribir evidencia '+str(backup))
        if not backup.exists():shutil.copy2(original,backup)
        assert sha(original)==sha(backup)
        manifest['files'][str(backup.relative_to(ROOT))]={'bytes':backup.stat().st_size,'sha256':sha(backup)}
sources=[]
for folder in [ESP/'src',ESP/'lib',ESP/'include',STM/'Core',STM/'TouchGFX/gui',STM/'tools',ROOT/'07_DESARROLLO/firmware/common']:
    for p in folder.rglob('*'):
        if p.is_file() and p.suffix in {'.h','.hpp','.c','.cpp','.py','.md'} and p.name!='secretos.h':sources.append(p)
sources.extend([ESP/'platformio.ini',STM/'STM32F746G_DISCO.ioc'])
panel=ROOT/'07_DESARROLLO/panel_local'
sources.extend([panel/x for x in ['server.py','calibracion.py','static/nucleo.js','static/canales.js','tests/test_robustez.py','qa/check-robustez.mjs']])
for p in sorted(set(sources)):
    manifest['files'][str(p.relative_to(ROOT))]={'bytes':p.stat().st_size,'sha256':sha(p)}
# El manifiesto de la compilación incluye TODAS las unidades dependientes y digest de cabeceras.
shutil.copy2(STM/'.build/hashes.json',DATA/'hashes_compilacion_stm32_dev2.json')
manifest['git_head_at_capture']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
manifest['note']='Fuentes de trabajo identificadas por SHA-256; cabecera Wi-Fi privada excluida. Binarios privados, sin push.'
(DATA/'hashes_dev2.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
origin=json.loads((STM/'ORIGEN_V0.json').read_text(encoding='utf-8'))
original_root=ROOT/origin['source']
for item in origin['files']:
    if sha(original_root/item['path'])!=item['sha256']:raise RuntimeError('Original modificado '+item['path'])
(DATA/'originales_verificados.json').write_text(json.dumps({'files_verified':len(origin['files']),'date':'2026-10-09','match':True},indent=2))
print('Originales STM32 verificados:',len(origin['files']))
print('Manifestados:',len(manifest['files']),'archivos, binarios incluidos; secretos excluidos.')
