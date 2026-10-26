"""Lee GRAM del banco v0.3 por UART. Guarda RGB565, PNG, CRC y log original.

No recrea la interfaz: los pixeles vienen del controlador ILI9341. Esta captura
no es una fotografia del cristal. Requiere pyserial; PNG usa biblioteca estandar.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import struct
import time
import zlib
import serial
from serial.tools import list_ports

def png_from_rgb565(data, width, height):
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            p = (y * width + x) * 2
            c = data[p] | data[p + 1] << 8
            r, g, b = c >> 11, (c >> 5) & 63, c & 31
            rows.extend(((r << 3) | (r >> 2), (g << 2) | (g >> 4), (b << 3) | (b >> 2)))
    def chunk(kind, payload):
        return struct.pack('>I', len(payload)) + kind + payload + struct.pack('>I', zlib.crc32(kind + payload))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>2I5B', width, height, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(rows, 9)) + chunk(b'IEND', b''))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', default='COM5')
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--scenes', default='0123456789', help='Indices 0-9; por defecto todas')
    args = parser.parse_args()
    if not args.scenes or any(c not in '0123456789' for c in args.scenes) or len(set(args.scenes)) != len(args.scenes):
        parser.error('Escenas: indices distintos 0-9')
    ports = [p for p in list_ports.comports() if p.device.upper() == args.port.upper()]
    if len(ports) != 1 or ports[0].serial_number != '0672FF564881774867244057' or ports[0].vid != 0x0483 or ports[0].pid != 0x374B:
        raise RuntimeError('El puerto no identifica la NUCLEO-F411RE de este banco')
    args.out.mkdir(parents=True, exist_ok=False)
    results = []
    ser = serial.Serial(port=None, baudrate=115200, timeout=1, write_timeout=2)
    ser.dtr = False; ser.rts = False; ser.port = args.port
    with ser, (args.out/'transporte.raw').open('xb') as raw, (args.out/'sesion.log').open('x', encoding='utf-8') as log:
        def note(text):
            line = datetime.now(timezone.utc).isoformat() + ' ' + text
            log.write(line + '\n'); log.flush(); print(text, flush=True)
        def read(n):
            data = ser.read(n);raw.write(data);raw.flush();return data
        def line_until(prefix, seconds=8):
            limit = time.monotonic() + seconds
            while time.monotonic() < limit:
                line = bytearray()
                while time.monotonic() < limit:
                    b = read(1)
                    if b: line.extend(b)
                    if b == b'\n': break
                text = line.decode('ascii', errors='replace').strip()
                if text:
                    note('RX ' + text)
                    if text.startswith(prefix): return text
            raise TimeoutError(prefix)
        def send(command):
            note('TX ' + command);ser.write(command.encode('ascii'));ser.flush()
        # Drena solo texto pendiente (se conserva); no envia nada hasta reconocer v0.3.
        time.sleep(0.2)
        pending = read(ser.in_waiting)
        if pending: note('RX_PENDING ' + pending.decode('ascii', errors='replace'))
        send('?'); identity = line_until('VIEW ')
        if 'v=0.3 ' not in identity: raise RuntimeError('Version incorrecta: ' + identity)
        line_until('LINE4 ')
        for digit in args.scenes:
            send(digit)
            state = line_until('VIEW ')
            if f'id={digit} ' not in state: raise RuntimeError('Escena inesperada: ' + state)
            line_until('LINE4 ')
            send('C'); header = line_until('CAPTURE_BEGIN ')
            match = re.fullmatch(r'CAPTURE_BEGIN v=0.3 scene=(\d) w=320 h=240 bytes=153600 format=RGB565LE source=ILI9341_GRAM flow=ROWACK', header)
            if not match or match[1] != digit: raise RuntimeError(header)
            payload = bytearray()
            for y in range(240):
                row = bytearray();limit = time.monotonic() + 5
                while len(row) < 640 and time.monotonic() < limit:
                    row.extend(read(640-len(row)))
                payload.extend(row)
                if len(row) != 640: break
                ser.write(b'A')
            # Guardar incluso una captura incompleta para diagnostico.
            base = args.out / f'escena_{int(digit)+1:02d}'
            base.with_suffix('.rgb565').write_bytes(payload)
            if len(payload) != 153600: raise RuntimeError(f'Captura incompleta: {len(payload)}')
            footer = line_until('CAPTURE_END ')
            crc = zlib.crc32(payload)
            if footer != f'CAPTURE_END crc32={crc:08X}': raise RuntimeError('CRC no coincide: ' + footer)
            png = png_from_rgb565(payload, 320, 240);base.with_suffix('.png').write_bytes(png)
            record = dict(scene=int(digit), state=state, utc=datetime.now(timezone.utc).isoformat(),
                          source='ILI9341_GRAM', format='RGB565LE', flow='ROWACK', width=320, height=240,
                          bytes=len(payload), crc32=f'{crc:08X}', sha256_rgb565=hashlib.sha256(payload).hexdigest(),
                          sha256_png=hashlib.sha256(png).hexdigest(), binary_version='v0.3')
            base.with_suffix('.json').write_text(json.dumps(record, indent=2)+'\n', encoding='utf-8')
            results.append(record);note(f'OK escena={int(digit)+1} bytes={len(payload)} CRC={crc:08X}')
        send('0');line_until('LINE4 ')
    (args.out/'manifest.json').write_text(json.dumps(results, indent=2)+'\n', encoding='utf-8')
    print(f'Completadas {len(results)} capturas. LCD en pagina 01/10; puerto cerrado.', flush=True)

if __name__ == '__main__': main()
