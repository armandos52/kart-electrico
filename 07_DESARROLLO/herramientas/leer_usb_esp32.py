"""Lee el puerto serie N segundos sin transmitir; guarda el log y resume la telemetría."""
import json, sys, time
import serial

puerto, segundos, salida = sys.argv[1], float(sys.argv[2]), sys.argv[3]
s = serial.Serial()
s.port, s.baudrate, s.timeout = puerto, 115200, 0.2
s.dtr = False
s.rts = False
s.open()
fin = time.time() + segundos
buf = b""
with open(salida, "wb") as f:
    while time.time() < fin:
        d = s.read(512)
        f.write(d)
        buf += d
s.close()
lineas = buf.decode("utf-8", "replace").splitlines()
tramas = []
for l in lineas:
    if l.startswith("{"):
        try:
            tramas.append(json.loads(l))
        except ValueError:
            print("JSON inválido:", l[:120])
    elif l.strip():
        print("TEXTO:", l[:160])
print(f"\ntramas JSON válidas: {len(tramas)} en {segundos:.0f} s")
if tramas:
    print("primera:", json.dumps(tramas[0], ensure_ascii=False))
    print("última: ", json.dumps(tramas[-1], ensure_ascii=False))
    seqs = [t["seq"] for t in tramas]
    print("seq:", seqs[0], "->", seqs[-1], "| saltos:", sum(b - a - 1 for a, b in zip(seqs, seqs[1:])))
    lm = [t["signals"].get("loop_max_us") for t in tramas]
    tu = [t["signals"].get("telemetry_us_max") for t in tramas]
    print("loop_max_us: min", min(lm), "max", max(lm), "| telemetry_us_max: max", max(tu))
    print("ticks entre tramas (ms):", sorted({b["tick_ms"] - a["tick_ms"] for a, b in zip(tramas, tramas[1:])}))
