"""Captura RX del banco; nunca envia comandos. Requiere pyserial."""
import argparse
from datetime import datetime, timezone
from pathlib import Path
import time
import serial

p = argparse.ArgumentParser()
p.add_argument('--port', default='COM5')
p.add_argument('--seconds', type=float, default=20)
p.add_argument('--out', type=Path, required=True)
a = p.parse_args()
if a.seconds <= 0 or a.seconds > 3600:
    p.error('seconds debe estar entre 0 y 3600')
if a.out.exists():
    p.error('no sobrescribir registros anteriores')
a.out.parent.mkdir(parents=True, exist_ok=True)
uart = serial.Serial(port=None, baudrate=115200, timeout=0.25)
uart.dtr = False
uart.rts = False
uart.port = a.port
uart.open()
started = time.monotonic()
with uart, a.out.open('w', encoding='utf-8') as log, a.out.with_suffix('.raw').open('xb') as raw:
    log.write(f'# UTC={datetime.now(timezone.utc).isoformat()} port={a.port} baud=115200 RX_only\n')
    pending = b''
    while time.monotonic() - started < a.seconds:
        data = uart.read(uart.in_waiting or 1)
        if not data:
            continue
        raw.write(data)
        pending += data
        while b'\n' in pending:
            line, pending = pending.split(b'\n', 1)
            text = line.rstrip(b'\r').decode('utf-8', errors='replace')
            stamped = f'{time.monotonic()-started:9.3f}s {text}'
            log.write(stamped+'\n')
            log.flush()
            print(stamped, flush=True)
    if pending:
        log.write(f'# parcial: {pending!r}\n')
