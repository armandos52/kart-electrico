"""Prueba del capturador con transporte serie simulado; no abre puertos reales."""
from contextlib import redirect_stdout
import importlib.util
import io
import json
from pathlib import Path
import sys
from types import SimpleNamespace
from unittest.mock import patch
import zlib

EVIDENCE=Path(__file__).resolve().parent
ROOT=EVIDENCE.parents[2]
spec=importlib.util.spec_from_file_location('capture_under_test', ROOT/'07_DESARROLLO/firmware/nucleo_f411_banco/capturar_pantallas.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

class FakeSerial:
    def __init__(self,version,header_version=None,state_version=None):
        self.version=version;self.header_version=header_version or version
        self.state_version=state_version or version;self.data=bytearray();self.rows=0;self.commands=[]
    def __enter__(self):return self
    def __exit__(self,*args):pass
    @property
    def in_waiting(self):return len(self.data)
    def read(self,n):
        part=self.data[:n];del self.data[:n];return bytes(part)
    def write(self,data):
        self.commands.append(data)
        if data in (b'?',b'0'):
            version=self.version if data==b'?' else self.state_version
            self.data.extend(f'VIEW v={version} id=0 synthetic=1\nLINE4 SIMULADO\n'.encode())
        elif data==b'C':
            self.data.extend(f'CAPTURE_BEGIN v={self.header_version} scene=0 w=320 h=240 bytes=153600 format=RGB565LE source=ILI9341_GRAM flow=ROWACK\n'.encode())
            self.rows=1;self.data.extend(bytes(640))
        elif data==b'A':
            if self.rows<240:self.rows+=1;self.data.extend(bytes(640))
            else:self.data.extend(f'\nCAPTURE_END crc32={zlib.crc32(bytes(153600)):08X}\n'.encode())
        else:raise AssertionError(data)
    def flush(self):pass

port=SimpleNamespace(device='COM_TEST',serial_number='0672FF564881774867244057',vid=0x0483,pid=0x374B)
cases=[('old','0.3',None,None,True),('new','0.4-dev',None,None,True),
       ('unknown','9.9',None,None,False),('mixed_header','0.4-dev','0.3',None,False),
       ('mixed_state','0.4-dev',None,'0.3',False)]
for name,version,header,state,success in cases:
    serial=FakeSerial(version,header,state);out=EVIDENCE/'captura_simulada'/name
    # Capturador exige directorio nuevo: una segunda ejecucion usa otro nombre.
    suffix=1
    while out.exists():out=EVIDENCE/'captura_simulada'/f'{name}_{suffix}';suffix+=1
    with patch.object(module.list_ports,'comports',return_value=[port]), \
         patch.object(module.serial,'Serial',return_value=serial), \
         patch.object(module.time,'sleep'), \
         patch.object(sys,'argv',['capture','--port','COM_TEST','--scenes','0','--out',str(out)]), \
         redirect_stdout(io.StringIO()):
        error=None
        try:module.main()
        except RuntimeError as exc:error=str(exc)
    assert (error is None)==success,(name,error)
    if success:
        manifest=json.loads((out/'manifest.json').read_text())
        assert manifest[0]['binary_version']=='v'+version
        assert len((out/'escena_01.rgb565').read_bytes())==153600
        assert serial.commands.count(b'A')==240
    else:
        assert b'A' not in serial.commands
        if name in ('unknown','mixed_state'):assert b'C' not in serial.commands
    print(f'PASS {name}: '+('version/metadatos/240 filas/CRC' if success else error))
print('TOTAL 5; FALLOS 0; TRANSPORTE SIMULADO, SIN HARDWARE')
