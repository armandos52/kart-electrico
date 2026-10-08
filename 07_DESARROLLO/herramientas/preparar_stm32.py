"""Copia íntegra y verificada del proyecto frontal; nunca modifica el original."""
from pathlib import Path
import hashlib,json,shutil
ROOT=Path(__file__).resolve().parents[2]
SRC=ROOT/'02_ORIGINALES/entrega_2026-09-28/Smart Eletric Kart Cross-2223406-2223403/Smart Eletric Kart Cross-2223406-2223403/STM32-final'
DST=ROOT/'07_DESARROLLO/firmware/stm32_frontal'
if DST.exists():
    raise SystemExit('La copia ya existe; no se sobrescribe. Revisar su manifiesto y estado.')
shutil.copytree(SRC,DST)
manifest=[]
for path in sorted(SRC.rglob('*')):
    if not path.is_file(): continue
    rel=path.relative_to(SRC)
    original=hashlib.sha256(path.read_bytes()).hexdigest()
    copied=hashlib.sha256((DST/rel).read_bytes()).hexdigest()
    if original!=copied: raise RuntimeError(f'No coincide: {rel}')
    manifest.append({'path':rel.as_posix(),'bytes':path.stat().st_size,'sha256':original})
(DST/'ORIGEN_V0.json').write_text(json.dumps({'source':str(SRC.relative_to(ROOT)),
    'installed_firmware_identified':False,'files':manifest},indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
print(json.dumps({'copied_and_verified':len(manifest),'bytes':sum(p['bytes'] for p in manifest),'destination':str(DST)}))
