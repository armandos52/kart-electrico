"""ENS-0026: telemetría TCP de solo lectura, restaurando eduroam en finally."""
import datetime
import argparse
import json
import socket
import subprocess
import time
from pathlib import Path

ap=argparse.ArgumentParser()
ap.add_argument('--salida',type=Path)
a=ap.parse_args()
root = a.salida.resolve() if a.salida else Path(__file__).resolve().parent
root.mkdir(parents=True,exist_ok=True)
if (root/'wifi_raw.log').exists() or (root/'wifi_eventos.json').exists():
    raise SystemExit('No sobrescribir captura anterior; indicar --salida nueva')
events = []
raw = bytearray()

def event(message):
    events.append({"utc": datetime.datetime.now(datetime.timezone.utc).isoformat(), "evento": message})

try:
    r = subprocess.run(['netsh', 'wlan', 'connect', 'name=kart-esp32', 'interface=Wi-Fi 2'], capture_output=True, text=True)
    event(r.stdout.strip() + r.stderr.strip())
    deadline = time.monotonic() + 20
    conn = None
    while time.monotonic() < deadline:
        try:
            conn = socket.create_connection(('192.168.4.1', 3333), timeout=2)
            break
        except OSError:
            time.sleep(1)
    if conn is None:
        raise RuntimeError('No se pudo abrir TCP en 20 s')
    with conn:
        event('TCP conectado; solo recv, ningún envío')
        conn.settimeout(2)
        deadline = time.monotonic() + 12
        while time.monotonic() < deadline:
            try:
                block = conn.recv(8192)
            except socket.timeout:
                continue
            if not block:
                break
            raw.extend(block)
    event('Captura terminada')
except Exception as exc:
    event(type(exc).__name__ + ': ' + str(exc))
finally:
    (root/'wifi_raw.log').write_bytes(raw)
    r = subprocess.run(['netsh', 'wlan', 'connect', 'name=eduroam', 'interface=Wi-Fi 2'], capture_output=True, text=True)
    event('Restauración: ' + r.stdout.strip() + r.stderr.strip())
    time.sleep(5)
    r = subprocess.run(['netsh', 'wlan', 'show', 'interfaces'], capture_output=True, text=True)
    event('eduroam presente en estado final: ' + str('SSID                   : eduroam' in r.stdout))
    (root/'wifi_eventos.json').write_text(json.dumps(events, indent=2, ensure_ascii=False), encoding='utf-8')

lines = raw.decode('utf-8', errors='replace').splitlines()
samples = []
for line in lines:
    try:
        item = json.loads(line)
        if isinstance(item, dict): samples.append(item)
    except ValueError:
        pass
print(json.dumps({'eventos': events, 'bytes': len(raw), 'muestras_json': len(samples), 'cabecera': lines[:1], 'primera': samples[:1], 'ultima': samples[-1:]}, ensure_ascii=False, indent=2))
