"""Compilación ARM de la copia completa, sin alterar Debug ni ejecutar/grabar hardware."""
from pathlib import Path
import argparse, concurrent.futures, hashlib, json, os, re, subprocess, sys
from verify_configuration import verify

ROOT=Path(__file__).resolve().parents[1]
DEBUG=ROOT/'STM32CubeIDE/Debug'
OUT=ROOT/'.build'

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--toolchain',type=Path,default=ROOT.parents[1]/'herramientas/toolchains/bin')
    parser.add_argument('--jobs',type=int,default=4)
    args=parser.parse_args()
    verify(ROOT)
    gcc=args.toolchain/'arm-none-eabi-gcc.exe'
    gxx=args.toolchain/'arm-none-eabi-g++.exe'
    print(subprocess.check_output([gcc,'--version'],text=True).splitlines()[0],flush=True)
    OUT.mkdir(exist_ok=True)
    sources=set()
    for makefile in DEBUG.rglob('subdir.mk'):
        content=makefile.read_text()
        for block in re.findall(r'^(?:C_SRCS|CPP_SRCS|S_SRCS|S_UPPER_SRCS)\s*\+=\s*\\\n((?:.*\\\n)*.*)',content,re.M):
            for item in block.replace('\\','').splitlines():
                item=item.strip()
                if not item:continue
                if item.startswith('C:/TouchGFXProjects/25-06/'):
                    source=ROOT/item.removeprefix('C:/TouchGFXProjects/25-06/')
                else:source=(DEBUG/item).resolve()
                if not source.is_relative_to(ROOT) or not source.is_file():raise ValueError(source)
                sources.add(source)
    sources.update(ROOT.glob('Core/Src/kart_*.c'))
    sources.update(ROOT.glob('TouchGFX/gui/src/**/*.cpp'))
    sources.add(ROOT/'Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_iwdg.c')
    make=(DEBUG/'Application/User/Core/subdir.mk').read_text()
    includes=list(dict.fromkeys(re.findall(r'-I(\S+)',make)))
    include_flags=['-I'+str((DEBUG/item).resolve()) for item in includes]
    common=['-mcpu=cortex-m7','-mthumb','-mfpu=fpv5-sp-d16','-mfloat-abi=hard','-Os','-g3','-DUSE_HAL_DRIVER','-DUSE_BPP=16','-DSTM32F746xx','-DDEBUG','-ffunction-sections','-fdata-sections','-Wall','-Wextra','-fstack-usage','--specs=nano.specs']+include_flags
    # Un cambio de cabecera invalida toda la caché (incluye dependencias y protocolo común).
    headers=hashlib.sha256()
    for p in sorted(list(ROOT.rglob('*.h'))+list(ROOT.rglob('*.hpp'))+list((ROOT.parent/'common').glob('*.h'))):headers.update(p.read_bytes())
    digest=headers.hexdigest()+json.dumps(common)+subprocess.check_output([gcc,'-dumpversion'],text=True)
    def compile_one(source):
        key=hashlib.sha256(str(source.relative_to(ROOT)).encode()).hexdigest()[:16]
        obj=OUT/(key+'.o');stamp=OUT/(key+'.sha256')
        signature=hashlib.sha256(source.read_bytes()+digest.encode()).hexdigest()
        if obj.exists() and stamp.exists() and stamp.read_text()==signature:return obj,''
        cpp=source.suffix=='.cpp'
        flags=['-std=gnu++14','-fno-exceptions','-fno-rtti','-fno-use-cxa-atexit'] if cpp else ['-std=gnu11'] if source.suffix=='.c' else ['-x','assembler-with-cpp']
        result=subprocess.run([gxx if cpp else gcc,*common,*flags,'-c',source,'-o',obj],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        if result.returncode:raise RuntimeError(str(source)+'\n'+result.stdout)
        stamp.write_text(signature)
        return obj,result.stdout
    print(f'Compilando {len(sources)} unidades en {OUT}',flush=True)
    objects=[]
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for obj,output in pool.map(compile_one,sorted(sources)):
            objects.append(obj)
            if output:print(output,flush=True)
    response=OUT/'objects.rsp';response.write_text('\n'.join('"'+p.as_posix()+'"' for p in objects))
    version_header=(ROOT/'Core/Inc/kart_frontend.h').read_text(encoding='utf-8')
    version=re.search(r'^#define KART_FRONTEND_VERSION "([A-Za-z0-9_.-]+)"$',version_header,re.M)
    if not version:raise RuntimeError('Falta KART_FRONTEND_VERSION valida')
    elf=OUT/(version.group(1)+'.elf')
    link=[gxx,'@'+str(response),'-o',elf,'-mcpu=cortex-m7','-mthumb','-mfpu=fpv5-sp-d16','-mfloat-abi=hard','-T',ROOT/'STM32F746XX_FLASH.ld','--specs=nosys.specs','--specs=nano.specs','-Wl,--gc-sections','-Wl,-Map='+str(OUT/'firmware.map'),'-static','-L'+str(ROOT/'Middlewares/ST/touchgfx/lib/core/cortex_m7/gcc'),'-l:libtouchgfx-float-abi-hard.a','-Wl,--start-group','-lc','-lm','-lstdc++','-lsupc++','-Wl,--end-group']
    link.append('-Wl,--fatal-warnings')
    subprocess.run(link,check=True)
    symbols=subprocess.check_output([args.toolchain/'arm-none-eabi-nm.exe',elf],text=True)
    if not re.search(r'^[0-9a-f]+ [TW] Reset_Handler$',symbols,re.M):raise RuntimeError('Falta Reset_Handler')
    subprocess.run([args.toolchain/'arm-none-eabi-size.exe',elf],check=True)
    # HEX conserva las direcciones de FLASH interna y QSPI; no generar BIN plano de 2 GB.
    subprocess.run([args.toolchain/'arm-none-eabi-objcopy.exe','-O','ihex',elf,elf.with_suffix('.hex')],check=True)
    manifest={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(sources)}
    manifest['headers_digest']=headers.hexdigest()
    for p in [elf,elf.with_suffix('.hex')]:manifest[str(p.relative_to(ROOT))]=hashlib.sha256(p.read_bytes()).hexdigest()
    (OUT/'hashes.json').write_text(json.dumps(manifest,indent=2))
    print('BUILD OK: ELF + HEX; sin upload.',flush=True)

if __name__=='__main__':main()
