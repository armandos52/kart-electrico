"""Conservación ENS-0023 y comprobaciones de lectura. No programa hardware."""
from pathlib import Path
import difflib
import hashlib
import json
import shutil
import sys

E = Path(__file__).resolve().parent
ROOT = E.parents[2]
FW = ROOT / '07_DESARROLLO/firmware/stm32_frontal'
BACK = ROOT / '07_DESARROLLO/respaldos/stm32_f746/2026-10-16'
VERSION = 'stm32_frontal-v1.3-dev7-ui'

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def ihex(path):
    regions = {0x08000000: bytearray(), 0x90000000: bytearray()}
    upper = 0
    for line in path.read_text().splitlines():
        rec = bytes.fromhex(line[1:])
        assert line.startswith(':') and sum(rec) % 256 == 0
        n, addr, kind = rec[0], int.from_bytes(rec[1:3], 'big'), rec[3]
        assert len(rec) == n + 5
        data = rec[4:4+n]
        if kind == 4:
            upper = int.from_bytes(data, 'big') << 16
        elif kind == 0:
            absolute = upper + addr
            base = next(b for b in regions if b <= absolute < b + (0x100000 if b == 0x08000000 else 0x1000000))
            out = regions[base]; offset = absolute - base
            if len(out) < offset + n:
                out.extend(b'\xff' * (offset + n - len(out)))
            out[offset:offset+n] = data
        else:
            assert kind in (1, 5), f'Tipo HEX no soportado: {kind}'
    assert all(regions.values())
    return regions

def copy(source, dest):
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, dest)

def archive():
    for pattern in ('Core/Inc/kart*.h', 'Core/Src/kart*.c', 'TouchGFX/gui/**/*.cpp',
                    'TouchGFX/gui/**/*.hpp', 'tests/**/*.h', 'tests/*.cpp', 'tests/*.cmd',
                    'tests/*.py', 'tools/*.py', 'assets/dashboard/*'):
        for path in FW.glob(pattern):
            if path.is_file(): copy(path, E/'despues'/path.relative_to(FW))
    patch = []
    for old in sorted((E/'antes').rglob('*')):
        if not old.is_file(): continue
        rel = old.relative_to(E/'antes'); new = E/'despues'/rel
        if new.exists():
            patch.extend(difflib.unified_diff(old.read_text(encoding='utf-8-sig').splitlines(True),
                new.read_text(encoding='utf-8-sig').splitlines(True), fromfile='antes/'+rel.as_posix(), tofile='despues/'+rel.as_posix()))
    (E/'cambios.patch').write_text(''.join(patch), encoding='utf-8')
    for name in (VERSION+'.hex', VERSION+'.elf', 'firmware.map', 'hashes.json'):
        copy(FW/'.build'/name, E/'binarios'/name)
    for ext in ('bin', 'elf'):
        copy(ROOT/'07_DESARROLLO/firmware/nucleo_f411_banco/.pio/build/nucleo_f411re'/('firmware.'+ext),
             E/'binarios'/('f411-v0.4-dev-con-dev7-ui.'+ext))
    cfg = (ROOT/'06_TRABAJO/evidencias/frontal_f746_2026-10-15/grabar_dev4.cfg').read_text()
    cfg = cfg[cfg.index('source [find'):].replace(
        'C:/kart proyecto/07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.hex',
        (E/'binarios'/(VERSION+'.hex')).as_posix())
    (E/'grabar_dev7.cfg').write_text('# ENS-0023: interna + QSPI; UID/RDP comprobados.\n'+cfg, encoding='ascii')
    cfg = (E/'respaldar.cfg').read_text().split('set R {')[0]
    cfg += f'set D {{{(E/"relectura").as_posix()}}}\nfile mkdir $D\n'
    regions = ihex(E/'binarios'/(VERSION+'.hex'))
    for base, data in regions.items():
        cfg += f'dump_image "$D/region_{base:08x}.bin" {base:#x} {len(data):#x}\n'
    cfg += 'echo "RELECTURA_FIN"\nshutdown\n'
    (E/'releer_dev7.cfg').write_text(cfg, encoding='ascii')
    print(json.dumps({'version': VERSION, 'hex_sha256': sha(E/'binarios'/(VERSION+'.hex')),
                      'regiones': {hex(b): len(d) for b,d in regions.items()}}, indent=2))

def verify_backup():
    result = {}
    for name, size in [('flash_interna', 0x100000), ('qspi', 0x1000000)]:
        a, b = (BACK/f'{name}_lectura{i}.bin' for i in (1,2))
        assert a.stat().st_size == b.stat().st_size == size
        assert a.read_bytes() == b.read_bytes(), f'Lecturas diferentes: {name}'
        result[name] = {'bytes': size, 'sha256': sha(a), 'lecturas_identicas': 2}
    old = ROOT/'07_DESARROLLO/respaldos/stm32/v1.3-dev4/stm32_frontal-v1.3-dev4.hex'
    for base, data in ihex(old).items():
        name = 'flash_interna' if base == 0x08000000 else 'qspi'
        assert (BACK/f'{name}_lectura1.bin').read_bytes()[:len(data)] == data, 'La imagen instalada no coincide con dev4'
    result['firmware_identificado'] = 'stm32_frontal-v1.3-dev4; coincidencia byte a byte en ambas regiones HEX'
    (BACK/'verificacion.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(result, indent=2))

def verify_new():
    result = {}
    for base, data in ihex(E/'binarios'/(VERSION+'.hex')).items():
        actual = (E/'relectura'/f'region_{base:08x}.bin').read_bytes()
        assert actual == data, f'Relectura diferente: {base:#x}'
        result[hex(base)] = {'bytes': len(data), 'diferencias': 0, 'sha256': hashlib.sha256(actual).hexdigest()}
    (E/'relectura/verificacion.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
    print(json.dumps(result, indent=2))

def manifest():
    data = {p.relative_to(E).as_posix(): {'bytes':p.stat().st_size, 'sha256':sha(p)}
            for p in sorted(E.rglob('*')) if p.is_file() and p.name != 'manifest.json' and p.suffix not in ('.exe','.obj','.pyc')}
    (E/'manifest.json').write_text(json.dumps(data, indent=2), encoding='utf-8')
    print(f'{len(data)} archivos con SHA-256')

if __name__ == '__main__':
    {'archive':archive, 'backup':verify_backup, 'verify':verify_new, 'manifest':manifest}[sys.argv[1]]()
