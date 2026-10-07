"""Evalúa los literales printf reales: trama completa, capacidad y lector del panel.

No ejecuta el ESP32 ni simula su control. El ensayo CAN físico sigue siendo necesario.
"""
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
PROJECT = ROOT / "07_DESARROLLO/firmware/esp32_trasero"
sys.path.insert(0, str(ROOT / "07_DESARROLLO/panel_local"))
from server import Monitor

VALUES = dict(session="0123456789abcdef", seq=4294967295, tick_ms=4294967295,
              fw="esp32_trasero-v1.2-diagnostico", reset_reason="watchdog_interrupcion",
              control_ready=1, can_init_ok=1, can_init_attempts=4294967295,
              control_state="running", drive_inhibit="mpu_unavailable", start_requested=1,
              accelerator_raw=65535, accelerator_pct=100, steer_raw=65535, brake=1,
              gear="R", mode="ESC", can_rx_age_ms=4294967295, yaw_dps=-2000.0,
              pwm_left_raw=255, pwm_right_raw=255, rpm_rl=-20000, rpm_rr=-20000,
              drive_direction="R", loop_max_us=4294967295, relay1=1, relay2=1,
              relay_seq=3, charger_connected=1, emergency_stop=1, can_rx_count=4294967295,
              mpu_ok=1, telemetry_us_max=4294967295, telemetry_dropped=4294967295,
              wifi_ap=1, wifi_clients=2)

def complete_frame(source, values):
    chunks = re.findall(r'agregar\(l,\s*((?:"(?:\\.|[^"\\])*"\s*)+)', source)
    assert chunks, "No se encuentran formatos del emisor"
    output = []
    for chunk in chunks:
        fmt = "".join(json.loads(part) for part in re.findall(r'"(?:\\.|[^"\\])*"', chunk))
        def fill(match):
            key, opening_quote, specifier = match.groups()
            return '"' + key + '":' + (opening_quote or '') + (specifier % values[key])
        output.append(re.sub(r'"([a-z_0-9]+)":(")?(%(?:\.\d+)?l?[dusf])', fill, fmt))
    return "".join(output)

def measure(source, header, values):
    line = complete_frame(source, values)
    data = json.loads(line)
    capacity = int(re.search(r'#define TELEMETRIA_LINEA_MAX\s+(\d+)', header)[1])
    return line, data, capacity

def main():
    old_source = subprocess.check_output(["git", "show", "esp32-trasero-v1.1:07_DESARROLLO/firmware/esp32_trasero/src/telemetria.cpp"], cwd=ROOT).decode()
    old_header = subprocess.check_output(["git", "show", "esp32-trasero-v1.1:07_DESARROLLO/firmware/esp32_trasero/include/telemetria.h"], cwd=ROOT).decode()
    old_line, _, old_capacity = measure(old_source, old_header, {**VALUES, "fw":"esp32_trasero-v1.1-telemetria"})
    line, data, capacity = measure((PROJECT / "src/telemetria.cpp").read_text(encoding="utf-8"),
                                   (PROJECT / "include/telemetria.h").read_text(encoding="utf-8"), VALUES)
    assert len(line.encode()) + 1 <= capacity, "La trama completa no cabe con terminador NUL"
    monitor = Monitor()
    monitor.ingest(line)
    assert monitor.stats["valid_frames"] == 1 and monitor.stats["unknown_signals"] == 0, monitor.stats
    result = {"v11_full_frame_bytes": len(old_line.encode()), "v11_capacity": old_capacity,
              "v11_would_drop":len(old_line.encode()) >= old_capacity,
              "v12_full_frame_bytes":len(line.encode()), "v12_capacity":capacity,
              "panel_accepts_all_signals":True,
              "scope":"Literales de formato del fuente; extremos de representación. No prueba CAN físico."}
    print(json.dumps(result, indent=2, ensure_ascii=False))
    if len(sys.argv)>1:
        target=Path(sys.argv[1]);target.mkdir(parents=True,exist_ok=True)
        (target / "limites_tramas.json").write_text(json.dumps(result,indent=2,ensure_ascii=False),encoding="utf-8")
        (target / "trama_completa_sintetica.jsonl").write_text(line,encoding="utf-8")

if __name__ == "__main__":
    main()
