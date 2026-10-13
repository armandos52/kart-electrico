"""Reproduce por TCP local las tramas de una sesión JSONL grabada por el panel, con su cadencia original.

Herramienta de QA: no abre puertos serie ni contacta con el kart. El panel la lee como una fuente
Wi-Fi más (`server.py --wifi socket://127.0.0.1:PUERTO`). Los datos son una repetición de una captura
anterior, no una medición nueva: el tick y la secuencia originales se conservan.
"""
import argparse
from datetime import datetime
import json
from pathlib import Path
import socket
import time


def frames(path):
    for line in Path(path).read_text(encoding="utf-8").splitlines():
        record = json.loads(line)
        if record.get("kind") == "telemetry":
            yield datetime.fromisoformat(record["received_utc"]).timestamp(), record["frame"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sesion", help="Archivo sesiones/*.jsonl")
    parser.add_argument("--puerto", type=int, default=3334)
    parser.add_argument("--velocidad", type=float, default=1.0, help="Factor de tiempo (2 = el doble de rápido)")
    args = parser.parse_args()
    data = list(frames(args.sesion))
    with socket.create_server(("127.0.0.1", args.puerto)) as listener:
        print(f"Reproductor en socket://127.0.0.1:{args.puerto} · {len(data)} tramas de {args.sesion}", flush=True)
        while True:
            client, _ = listener.accept()
            with client:
                start, first = time.monotonic(), data[0][0]
                try:
                    for stamp, frame in data:
                        delay = (stamp - first) / args.velocidad - (time.monotonic() - start)
                        if delay > 0:
                            time.sleep(delay)
                        client.sendall((json.dumps(frame, ensure_ascii=False) + "\n").encode("utf-8"))
                except OSError:
                    pass
            print("Cliente desconectado; esperando otro", flush=True)


if __name__ == "__main__":
    main()
