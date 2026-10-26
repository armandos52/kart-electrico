"""Conserva solo fuentes revisadas y resultados de ENS-0017, sin acceso a placas."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess

EVIDENCE = Path(__file__).resolve().parent
ROOT = EVIDENCE.parents[2]
FRONT = ROOT / '07_DESARROLLO/firmware/stm32_frontal'
relative_sources = [
    'Core/Inc/kart_frontend.h', 'Core/Inc/kart_display.h',
    'Core/Inc/FreeRTOSConfig.h', 'Core/Inc/stm32f7xx_hal_conf.h',
    'Core/Src/kart_frontend.c', 'Core/Src/kart_display.c', 'Core/Src/main.c',
    'Core/Src/freertos.c', 'Core/Src/stm32f7xx_it.c',
    'TouchGFX/gui/src/model/Model.cpp',
    'TouchGFX/gui/src/screen1_screen/Screen1Presenter.cpp',
    'TouchGFX/gui/src/screen1_screen/Screen1View.cpp',
    'TouchGFX/gui/include/gui/screen1_screen/Screen1View.hpp',
    'TouchGFX/gui/src/common/KartStatus.cpp',
    'TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp',
    'TouchGFX/generated/fonts/src/Table_verdana_50_4bpp.cpp',
    'TouchGFX/assets/texts/texts.xml',
    'FATFS/Target/sd_diskio.c', 'FATFS/Target/bsp_driver_sd.c',
    'FATFS/Target/ffconf.h', 'FATFS/App/fatfs.c',
    'STM32F746XX_FLASH.ld', 'STM32F746G_DISCO.ioc',
    'tools/build.py', 'tools/verify_configuration.py',
    'tests/frontend_test.cpp', 'tests/compilar.cmd',
    'tests/stubs/main.h', 'tests/stubs/cmsis_os.h', 'tests/stubs/fatfs.h',
]
sources = [FRONT / p for p in relative_sources]
sources += [ROOT / '07_DESARROLLO/firmware' / p for p in [
    'common/kart_protocol.h', 'common/PROTOCOLO_V13.md',
    'esp32_trasero/lib/kart_control/kart_control.cpp',
    'esp32_trasero/lib/kart_control/kart_control.h',
    'esp32_trasero/tests/control_test.cpp', 'esp32_trasero/tests/compilar.cmd',
]]
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
manifest = {'date_local': '2026-10-15', 'kind': 'PC; hardware simulado; sin grabacion',
            'commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
            'source_files': {}, 'artifacts': {}, 'comparison_preserved_dev3': {}}
for src in sources:
    relative = src.relative_to(ROOT)
    dst = EVIDENCE / 'fuentes' / relative
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    assert sha(src) == sha(dst)
    manifest['source_files'][relative.as_posix()] = sha(src)
for name in ['stm32_frontal-v1.3-dev3.elf', 'stm32_frontal-v1.3-dev3.hex', 'firmware.map', 'hashes.json']:
    src = FRONT / '.build' / name
    dst = EVIDENCE / 'compilacion' / name
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    manifest['artifacts'][dst.relative_to(EVIDENCE).as_posix()] = sha(dst)
    prior = ROOT / '07_DESARROLLO/respaldos/stm32/v1.3-dev3' / name
    if prior.is_file():
        manifest['comparison_preserved_dev3'][name] = {
            'new_sha256': sha(src), 'preserved_sha256': sha(prior), 'equal': sha(src) == sha(prior)}
for name in ['frontend_tests.log', 'backend_tests.log', 'build_stm32.log', 'audit_display.log',
             'audit_display.cpp', 'compilar_audit.cmd', 'conservar_evidencia.py',
             'estado_git_inicial.txt', 'commit_base.txt']:
    manifest['artifacts'][name] = sha(EVIDENCE / name)
(EVIDENCE / 'manifest.json').write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
print(json.dumps({'sources_preserved': len(sources), 'comparison': manifest['comparison_preserved_dev3']}, indent=2))
