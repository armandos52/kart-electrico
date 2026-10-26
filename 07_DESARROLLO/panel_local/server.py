"""Panel local de lectura: puerto serie -> HTTP de bucle local, sin TX al micro."""
from __future__ import annotations

import argparse
from collections import deque
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import ipaddress
import json
import math
from pathlib import Path
import re
import secrets
import sys
import threading
import time
from urllib.parse import urlsplit, parse_qs
from calibracion import SteeringCalibration
from historial import History, MAX_FILE

BASE = Path(__file__).resolve().parent
sys.path.insert(0, str(BASE / ".vendor"))
try:
    import serial
    from serial.tools import list_ports
except ImportError:
    serial = None
    list_ports = None

# Rangos de representación/protocolo, no límites autorizados de actuación.
SIGNALS = {
    "accelerator_pct": ("Pedal procesado", "%", 0, 100, 1500),
    "steer_deg": ("Dirección", "°", -180, 180, 1500),
    "brake": ("Freno leído", "", 0, 1, 1500),
    "rpm_fl": ("Rueda delantera izquierda", "rpm", -20000, 20000, 1500),
    "rpm_fr": ("Rueda delantera derecha", "rpm", -20000, 20000, 1500),
    "rpm_rl": ("Rueda trasera izquierda", "rpm", -20000, 20000, 1500),
    "rpm_rr": ("Rueda trasera derecha", "rpm", -20000, 20000, 1500),
    "yaw_dps": ("Guiñada medida", "°/s", -2000, 2000, 1500),
    "pwm_left_raw": ("PWM bruto izquierdo", "/255", 0, 255, 1500),
    "pwm_right_raw": ("PWM bruto derecho", "/255", 0, 255, 1500),
    "relay1": ("Relé 1 ordenado", "", 0, 1, 1500),
    "relay2": ("Relé 2 ordenado", "", 0, 1, 1500),
    "charger_connected": ("Cargador detectado", "", 0, 1, 1500),
    "soc_pct": ("Carga estimada BMS", "%", 0, 100, 8000),
    "battery_v": ("Tensión reportada", "V", 0, 200, 8000),
    "battery_a": ("Corriente reportada", "A", -5000, 5000, 8000),
    "battery_temp_c": ("Temperatura reportada", "°C", -100, 200, 8000),
    "sd_ready": ("SD disponible", "", 0, 1, 3000),
    "can_rx_age_ms": ("Edad de orden CAN", "ms", 0, 4294967295, 1500),
    "gear": ("Marcha", "", {"N", "D", "R"}, None, 1500),
    "mode": ("Modo diferencial", "", {"DIF", "ESC", "BLQ"}, None, 1500),
    # Diagnóstico del ESP32 trasero v1 (07_DESARROLLO/firmware/esp32_trasero).
    "accelerator_raw": ("Acelerador recibido por CAN", "cuentas", 0, 65535, 1500),
    "steer_raw": ("Dirección recibida por CAN", "cuentas", 0, 65535, 1500),
    "relay_seq": ("Secuencia de relés (0 apagado … 3 listo)", "", 0, 3, 1500),
    "emergency_stop": ("Parada por timeout CAN", "", 0, 1, 1500),
    "can_rx_count": ("Órdenes CAN 0x100 recibidas", "", 0, 4294967295, 1500),
    "loop_max_us": ("Periodo máximo de loop()", "µs", 0, 4294967295, 1500),
    "telemetry_us_max": ("Coste de telemetría en loop()", "µs", 0, 4294967295, 1500),
    "telemetry_dropped": ("Tramas de telemetría descartadas", "", 0, 4294967295, 1500),
    "wifi_clients": ("Clientes Wi-Fi conectados", "", 0, 8, 1500),
    "wifi_ap": ("Punto de acceso Wi-Fi creado", "", 0, 1, 1500),
    "mpu_ok": ("MPU6050 respondió al arrancar", "", 0, 1, 1500),
    "control_ready": ("Control inicializado", "", 0, 1, 1500),
    "can_init_ok": ("CAN inicializado", "", 0, 1, 1500),
    "can_init_attempts": ("Intentos de inicio CAN", "", 0, 4294967295, 1500),
    "start_requested": ("START recibido", "", 0, 1, 1500),
    "control_state": ("Fase del programa", "", {"can_wait", "mpu_init", "mpu_calibration", "running"}, None, 1500),
    "drive_direction": ("Marcha ordenada a los Kelly", "", {"N", "D", "R"}, None, 1500),
    "drive_inhibit": ("Motivo de parada del control", "", {"can_init", "initializing", "no_command", "relays", "brake", "pedal_zero", "neutral", "direction_change", "can_timeout", "mpu_unavailable", "none", "invalid_command", "start_off", "charger", "rearm_required", "mode_change", "speed_invalid", "hall_fault", "frontend_invalid", "reverse_limit", "output_fault", "can_fault", "configuration"}, None, 1500),
}
SIGNALS.update({
    'mode_applied': ('Modo aplicando traccion (255 = ninguno)', '', 0, 255, 1500),
    'rearm_required': ('Rearme requerido', '', 0, 1, 1500),
    'reverse_limited': ('Limitacion de marcha atras activa', '', 0, 1, 1500),
    'speed_valid': ('Calidad de velocidad Hall', '', 0, 1, 1500),
    'speed_kph_max_wheel': ('Velocidad estimada rueda mas rapida', 'km/h', 0, 300, 1500),
    'reverse_limit_kph': ('Umbral configurado marcha atras', 'km/h', 0, 100, 1500),
    'frontend_health_seen': ('Frontal envia diagnostico ADC', '', 0, 1, 1500),
    'frontend_health_ok': ('Salud frontal aceptada', '', 0, 1, 1500),
    'watchdog_ok': ('Watchdog configurado', '', 0, 1, 1500),
    'can_rx_invalid': ('Recepciones CAN invalidas', '', 0, 4294967295, 1500),
    'can_tx_errors': ('Errores envio CAN', '', 0, 4294967295, 1500),
    'can_error_flags': ('Bits error MCP2515', '', 0, 255, 1500),
    'mpu_errors': ('Errores lectura/calibracion MPU', '', 0, 4294967295, 1500),
    'accelerator_adc_physical': ('ADC fisico pedal (65535 = fallo)', 'cuentas', 0, 65535, 1500),
    'steer_adc_physical': ('ADC fisico direccion (65535 = fallo)', 'cuentas', 0, 65535, 1500),
})
DISCRETE = {"brake", "relay1", "relay2", "charger_connected", "sd_ready", "pwm_left_raw", "pwm_right_raw", "can_rx_age_ms",
            "accelerator_raw", "steer_raw", "relay_seq", "emergency_stop", "can_rx_count", "loop_max_us",
            "telemetry_us_max", "telemetry_dropped", "wifi_clients", "wifi_ap", "mpu_ok",
            "control_ready", "can_init_ok", "can_init_attempts", "start_requested"}

DISCRETE.update({'can_tx_errors', 'frontend_health_seen', 'accelerator_adc_physical', 'speed_valid', 'watchdog_ok', 'reverse_limited', 'frontend_health_ok', 'steer_adc_physical', 'mpu_errors', 'can_error_flags', 'mode_applied', 'can_rx_invalid', 'rearm_required'})

STATIC_MODULES = ("historial.js", "replay.js", "app.js", "nucleo.js", "i18n.js", "textos.js", "canales.js", "graficas.js", "vista_kart.js", "calibracion.js",
                  "modelo_kart.js", "escena_kart.js", "esquema_kart.js", "vendor/three.module.min.js",
                  "vendor/three.core.min.js", "vendor/OrbitControls.js")

# Fuentes TCP (Wi-Fi directo con el ESP32) añadidas con --wifi; solo direcciones privadas.
WIFI_URLS = []
DEFAULT_WIFI_URL = "socket://192.168.4.1:3333"
WIFI_URL = re.compile(r"^socket://(\d{1,3}(?:\.\d{1,3}){3}):(\d{1,5})$")


def wifi_url(text):
    match = WIFI_URL.match(text)
    if not match or not 0 < int(match.group(2)) < 65536:
        raise argparse.ArgumentTypeError("Formato: socket://IP:PUERTO, p. ej. socket://192.168.4.1:3333")
    if not ipaddress.ip_address(match.group(1)).is_private:
        raise argparse.ArgumentTypeError("Solo direcciones privadas (red directa del kart)")
    return text


def port_inventory():
    if list_ports is None:
        return []
    return [{"device": p.device, "description": p.description, "hwid": p.hwid}
            for p in list_ports.comports()
            if "Active Management Technology" not in p.description] + \
           [{"device": url, "description": "Wi-Fi directo ESP32 (TCP, solo lectura)", "hwid": "socket"}
            for url in WIFI_URLS]


class Monitor:
    def __init__(self, clock=time.monotonic, session_dir=None, calibration_dir=None):
        self.clock = clock
        self.lock = threading.RLock()
        self.samples = {}
        self.nodes = {}
        self.messages = deque(maxlen=250)
        self.message_id = 0
        self.frame_id = 0
        self.stats = {"valid_frames": 0, "invalid_frames": 0, "sequence_gaps": 0,
                      "discarded_lines": 0, "unknown_signals": 0}
        self.status = "disconnected"
        self.port = None
        self.error = None
        self.log_error = None
        self.log = None
        self.session_file = None
        self.session_dir = Path(session_dir) if session_dir else BASE / "sesiones"
        self.calibration = SteeringCalibration(calibration_dir or self.session_dir / "calibraciones", clock)
        self.connection_lock = threading.Lock()
        self.handle = None
        self.worker = None
        self.stop = threading.Event()

    def event(self, text, kind="serial"):
        with self.lock:
            self.message_id += 1
            item = {"id": self.message_id, "time": datetime.now(timezone.utc).isoformat(),
                    "kind": kind, "text": str(text)[:8192]}
            self.messages.append(item)
            self._log({"kind": "event", **item})

    def _log(self, record):
        if self.log:
            try:
                self.log.write(json.dumps(record, ensure_ascii=False, allow_nan=False) + "\n")
                self.log.flush()
            except (OSError, ValueError) as exc:
                self.log_error = str(exc)

    @staticmethod
    def _uint(value):
        return type(value) is int and 0 <= value <= 4294967295

    def ingest(self, line):
        """Solo acepta telemetría versionada; texto legado se conserva como evento."""
        with self.lock:
            if not line.strip():
                return
            if not line.lstrip().startswith("{"):
                self.event(line)
                return
            try:
                def no_constant(value):
                    raise ValueError("Constante JSON no finita: " + value)
                data = json.loads(line, parse_constant=no_constant)
                if not isinstance(data, dict) or type(data.get("v")) is not int or data.get("v") != 1 or data.get("type") != "telemetry":
                    raise ValueError("Formato/version de telemetría no soportado")
                node = data.get("node")
                session = data.get("session")
                seq = data.get("seq")
                tick = data.get("tick_ms")
                values = data.get("signals")
                if node not in {"esp32", "stm32", "bms"}:
                    raise ValueError("Nodo no reconocido")
                if not isinstance(session, str) or not 1 <= len(session) <= 64:
                    raise ValueError("Falta identificador de arranque/sesión")
                if not self._uint(seq) or not self._uint(tick) or not isinstance(values, dict) or len(values) > 64:
                    raise ValueError("Secuencia, tick o señales inválidos")
                clean = {}
                unknown = 0
                for key, value in values.items():
                    if key not in SIGNALS:
                        unknown += 1
                        continue
                    definition = SIGNALS[key]
                    if key == "mode_applied" and (type(value) is not int or value not in {0, 1, 2, 255}):
                        raise ValueError("Modo aplicado invalido")
                    if isinstance(definition[2], set):
                        if not isinstance(value, str) or value not in definition[2]:
                            raise ValueError("Valor inválido: " + key)
                    else:
                        if type(value) not in {int, float} or not math.isfinite(value) or not definition[2] <= value <= definition[3]:
                            raise ValueError("Valor fuera de representación: " + key)
                        if key in DISCRETE and value != int(value):
                            raise ValueError("Valor discreto inválido: " + key)
                    clean[key] = value
                if not clean:
                    raise ValueError("La trama no contiene señales conocidas")
                fw = data.get("fw")
                if fw is not None and (not isinstance(fw, str) or len(fw) > 100):
                    raise ValueError("Versión de firmware inválida")
                reset_reason = data.get("reset_reason")
                if reset_reason is not None and (not isinstance(reset_reason, str) or len(reset_reason) > 64):
                    raise ValueError("Motivo de reinicio inválido")
                previous = self.nodes.get(node)
                delta = None
                if previous and previous["session"] == session:
                    delta = (seq - previous["seq"]) & 0xFFFFFFFF
                    if not 0 < delta < 0x80000000:
                        raise ValueError("Trama duplicada o fuera de orden")
                elif previous:
                    self.samples = {k: v for k, v in self.samples.items() if v["node"] != node}
                    self.event("Nuevo arranque/sesión reportado por " + node, "session")
                now = self.clock()
                self.frame_id += 1
                for key, value in clean.items():
                    self.samples[key] = {"value": value, "node": node, "tick_ms": tick,
                                         "received": now, "session": session, "seq": seq, "sample_id": self.frame_id}
                self.nodes[node] = {"session": session, "seq": seq, "tick_ms": tick,
                                    "reset_reason": reset_reason or (previous.get("reset_reason") if previous and previous["session"] == session else None),
                                    "fw": fw or (previous.get("fw") if previous and previous["session"] == session else None), "received": now}
                self.stats["valid_frames"] += 1
                self.stats["unknown_signals"] += unknown
                if delta is not None:
                    self.stats["sequence_gaps"] += delta - 1
                self._log({"kind": "telemetry", "received_utc": datetime.now(timezone.utc).isoformat(),
                           "frame": data})
                try:
                    self.calibration.observe(data, self.port, self.session_file)
                except (OSError, ValueError) as exc:
                    self.calibration.error = "Registro de calibración fallido: " + str(exc)
                    self.calibration.active = None
                    self.calibration.capture = None
            except (ValueError, TypeError, OverflowError) as exc:
                self.stats["invalid_frames"] += 1
                self.event("Telemetría rechazada: " + str(exc), "invalid")
                if self.calibration.capture:
                    try:
                        self.calibration.interrupt("Telemetría inválida: " + str(exc))
                    except OSError as error:
                        self.calibration.error = str(error)

    def snapshot(self):
        with self.lock:
            now = self.clock()
            fields = {}
            for key, definition in SIGNALS.items():
                sample = self.samples.get(key)
                age = round((now - sample["received"]) * 1000) if sample else None
                quality = ("fresh" if self.status == "connected" and age <= definition[4] else "stale") if sample else "missing"
                fields[key] = {"label": definition[0], "unit": definition[1], "quality": quality,
                               "stale_after_ms": definition[4],
                               "value": sample["value"] if sample else None, "age_ms": age,
                               "node": sample["node"] if sample else None,
                               "tick_ms": sample["tick_ms"] if sample else None,
                               "sample_id": sample["sample_id"] if sample else None}
            esp_version = self.nodes.get("esp32", {}).get("fw") or ""
            if "v1.3" in esp_version:
                fields["mpu_ok"]["label"] = "MPU: lectura reciente y calibrada"
                for flag, keys in [("speed_valid", ("rpm_rl", "rpm_rr", "speed_kph_max_wheel")),
                                   ("mpu_ok", ("yaw_dps",)),
                                   ("frontend_health_ok", ("accelerator_adc_physical", "steer_adc_physical"))]:
                    valid = fields[flag]
                    if valid["quality"] != "fresh" or valid["value"] == 0:
                        for key in keys:
                            if fields[key]["quality"] == "fresh":fields[key]["quality"] = "stale"
            return {"status": self.status, "port": self.port, "error": self.error,
                    "log_error": self.log_error, "session_file": self.session_file,
                    "frame_id": self.frame_id, "signals": fields, "stats": dict(self.stats),
                    "nodes": {key: {k: v for k, v in value.items() if k != "received"}
                              for key, value in self.nodes.items()}, "messages": list(self.messages),
                    "serial_available": serial is not None, "read_only": True,
                    "calibration": self.calibration.snapshot(self.status == "connected")}

    def calibrate(self, action, data):
        """API del PC; nunca entrega estos mensajes a serial/TCP."""
        with self.lock:
            reason = self.calibration.tick(self.status == "connected")
            if action in {"begin", "capture", "activate", "guided-begin", "guided-measure", "guided-resume", "guided-use"} and reason:
                raise ValueError(reason)
            if self.log_error and action in {"begin", "capture", "guided-begin", "guided-measure", "guided-resume"}:
                raise ValueError("Resuelve el error del registro de sesión antes de capturar")
            if action == "guided-begin":
                self.calibration.begin_guided(data)
            elif action == "guided-measure":
                self.calibration.measure_guided()
            elif action == "guided-resume":
                self.calibration.resume_guided()
            elif action == "guided-cancel":
                self.calibration.cancel_guided()
            elif action == "guided-use":
                return {"id":self.calibration.use_guided(data)}
            elif action == "begin":
                self.calibration.begin(data)
            elif action == "capture":
                self.calibration.start_capture()
            elif action == "cancel":
                self.calibration.finish("rejected", "Cancelada por el usuario")
            elif action == "save":
                return {"id": self.calibration.save(data)}
            elif action == "activate":
                self.calibration.activate(data)
            elif action == "deactivate":
                if self.calibration.capture:
                    raise ValueError("Termina o cancela la captura antes de cambiar perfil")
                self.calibration.active = None
            else:
                raise ValueError("Operación de calibración desconocida")
            return {"ok": True}

    def connect(self, port, bench_ready):
        if bench_ready is not True:
            raise ValueError("Confirma el banco con tracción inhabilitada antes de abrir el puerto")
        if serial is None:
            raise ValueError("Falta pySerial; instala requirements.txt")
        if not isinstance(port, str) or port not in {p["device"] for p in port_inventory()}:
            raise ValueError("Selecciona un puerto presente en el inventario")
        with self.connection_lock:
            if self.worker and self.worker.is_alive():
                raise ValueError("Ya hay una lectura abierta; desconecta primero")
            if port in WIFI_URLS:
                # TCP: mismo flujo de líneas JSON que por USB; no hay líneas DTR/RTS ni reset.
                handle = serial.serial_for_url(port, do_not_open=True, timeout=0.2, write_timeout=0.2)
                description = port + " (Wi-Fi)"
            else:
                handle = serial.Serial(port=None, baudrate=115200, timeout=0.2, write_timeout=0.2,
                                       xonxoff=False, rtscts=False, dsrdtr=False)
                # No transmitir bytes; DTR/RTS desactivados antes de abrir. El driver puede generar un pulso.
                handle.dtr = False
                handle.rts = False
                handle.port = port
                description = port + " a 115200 bit/s"
            try:
                handle.open()
                self.session_dir.mkdir(parents=True, exist_ok=True)
                session_name = datetime.now().strftime("%Y%m%d-%H%M%S-") + secrets.token_hex(3) + ".jsonl"
                log = (self.session_dir / session_name).open("x", encoding="utf-8")
            except Exception:
                handle.close()
                raise
            with self.lock:
                self.samples.clear()
                self.calibration.interrupt("Nueva conexión: confirma de nuevo el perfil")
                self.calibration.latest = None
                self.nodes.clear()
                self.messages.clear()
                for key in self.stats:
                    self.stats[key] = 0
                self.error = None
                self.log_error = None
                self.status = "connected"
                self.port = port
                self.handle = handle
                self.log = log
                self.session_file = session_name
                self.stop.clear()
            self.event("Lectura abierta en " + description + "; sin TX al micro", "connection")
            self.worker = threading.Thread(target=self._read_loop, args=(handle,), daemon=True)
            self.worker.start()

    def _read_loop(self, handle):
        buffer = bytearray()
        dropping = False
        try:
            while not self.stop.is_set():
                chunk = handle.read(512)
                for byte in chunk:
                    if byte == 10:
                        if not dropping:
                            self.ingest(buffer.decode("utf-8", errors="replace").rstrip("\r"))
                        buffer.clear()
                        dropping = False
                    elif not dropping:
                        buffer.append(byte)
                        if len(buffer) > 8192:
                            buffer.clear()
                            dropping = True
                            with self.lock:
                                self.stats["discarded_lines"] += 1
                            self.event("Línea excesiva descartada hasta el siguiente salto", "invalid")
        except Exception as exc:
            if not self.stop.is_set():
                with self.lock:
                    self.error = str(exc)
                self.event("Lectura interrumpida: " + str(exc), "connection")
        finally:
            handle.close()
            with self.lock:
                self.status = "disconnected"
                self.handle = None
                try:
                    self.calibration.interrupt("Fuente desconectada")
                except OSError as exc:
                    self.calibration.error = str(exc)
                self.event("Puerto cerrado; señales marcadas sin datos vigentes", "connection")
                if self.log:
                    self.log.close()
                    self.log = None

    def disconnect(self):
        with self.connection_lock:
            self.stop.set()
            if self.worker:
                self.worker.join(timeout=2)
                if self.worker.is_alive():
                    raise ValueError("Lectura aún finalizando; vuelve a comprobar el estado")
            with self.lock:
                self.status = "disconnected"
                self.calibration.interrupt("Fuente desconectada")


class Handler(BaseHTTPRequestHandler):
    server_version = "KartPanel/0.1"

    def _allowed_host(self):
        return self.headers.get("Host") in {f"127.0.0.1:{self.server.server_port}", f"localhost:{self.server.server_port}"}

    def _respond(self, status, payload, content_type="application/json; charset=utf-8"):
        data = json.dumps(payload, ensure_ascii=False, allow_nan=False).encode("utf-8") if isinstance(payload, (dict, list)) else payload
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; connect-src 'self'; frame-ancestors 'none'")
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if not self._allowed_host():
            return self._respond(403, {"error": "Host local requerido"})
        path = urlsplit(self.path).path
        if path == "/api/state":
            data = self.server.monitor.snapshot()
            data["token"] = self.server.token
            return self._respond(200, data)
        if path.startswith("/api/history/"):
            query = parse_qs(urlsplit(self.path).query)
            sid = query.get("id", [None])[0]
            try:
                history = self.server.history
                action = path.rsplit("/", 1)[-1]
                if action == "list": return self._respond(200, {"sessions": history.list()})
                if action == "manifest": return self._respond(200, history.manifest(sid))
                if action == "state": return self._respond(200, history.state(sid, query.get("at", [0])[0]))
                if action == "window": return self._respond(200, history.window(sid, query.get("start", [0])[0], query.get("end", [60000])[0]))
                if action == "events": return self._respond(200, history.events(sid, query.get("after", [0])[0]))
                if action == "export": return self._respond(200, history.export(sid), "application/zip")
                return self._respond(404, {"error": "Ruta de historial desconocida"})
            except (ValueError, OSError, KeyError) as exc:
                return self._respond(400, {"error": str(exc)})
        if path == "/api/ports":
            return self._respond(200, {"ports": port_inventory(), "serial_available": serial is not None})
        if path == "/api/download":
            name = self.server.monitor.session_file
            if not name:
                return self._respond(404, {"error": "No hay sesión serie registrada"})
            return self._respond(200, (self.server.monitor.session_dir / name).read_bytes(), "application/x-ndjson; charset=utf-8")
        if path == "/api/calibration/export":
            query = parse_qs(urlsplit(self.path).query)
            try:
                with self.server.monitor.lock:
                    payload = self.server.monitor.calibration.export(query.get("run", [None])[0], query.get("profile", [None])[0])
                return self._respond(200, payload, "application/zip")
            except (ValueError, OSError) as exc:
                return self._respond(400, {"error": str(exc)})
        routes = {"/": ("index.html", "text/html; charset=utf-8"), "/style.css": ("style.css", "text/css; charset=utf-8")}
        # Lista cerrada de módulos del navegador; no se sirve ningún otro archivo de la carpeta.
        routes.update({f"/{name}": (name, "text/javascript; charset=utf-8") for name in STATIC_MODULES})
        routes.update({f"/referencias/{name}.jpg": (f"referencias/{name}.jpg", "image/jpeg")
                       for name in ("frontal", "lateral", "trasera", "superior")})
        if path not in routes:
            return self._respond(404, {"error": "No encontrado"})
        name, content_type = routes[path]
        return self._respond(200, (BASE / "static" / name).read_bytes(), content_type)

    def do_POST(self):
        if not self._allowed_host() or self.headers.get("X-Kart-Token") != self.server.token:
            return self._respond(403, {"error": "Sesión local requerida"})
        origin = self.headers.get("Origin")
        if origin and origin not in {f"http://127.0.0.1:{self.server.server_port}", f"http://localhost:{self.server.server_port}"}:
            return self._respond(403, {"error": "Origen no autorizado"})
        try:
            path = urlsplit(self.path).path
            if path == "/api/history/upload":
                length = int(self.headers.get("Content-Length", "0"))
                if not 0 < length <= MAX_FILE: raise ValueError("Archivo mayor de 32 MiB o vacío")
                query = parse_qs(urlsplit(self.path).query)
                content = self.rfile.read(length)
                if len(content) != length: raise ValueError("Carga incompleta")
                return self._respond(200, self.server.history.upload(query.get("batch", [None])[0], query.get("name", [""])[0], content))
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 4096:
                raise ValueError("Petición excesiva o vacía")
            data = json.loads(self.rfile.read(length))
            if not isinstance(data, dict):
                raise ValueError("Objeto requerido")
            path = urlsplit(self.path).path
            if path == "/api/history/start":
                return self._respond(200, self.server.history.start())
            elif path == "/api/history/finish":
                return self._respond(200, self.server.history.finish(data.get("batch")))
            elif path == "/api/history/annotate":
                return self._respond(200, self.server.history.annotate(data.get("id"), data))
            elif path == "/api/connect":
                self.server.monitor.connect(data.get("port"), data.get("bench_ready"))
            elif path == "/api/disconnect":
                self.server.monitor.disconnect()
            elif path.startswith("/api/calibration/"):
                return self._respond(200, self.server.monitor.calibrate(path.rsplit("/", 1)[-1], data))
            else:
                return self._respond(404, {"error": "No hay comandos de actuación al micro"})
            return self._respond(200, {"ok": True})
        except Exception as exc:
            return self._respond(400, {"error": str(exc)})

    def log_message(self, fmt, *args):
        if args and str(args[1]) != "200":
            super().log_message(fmt, *args)


class PanelHTTPServer(ThreadingHTTPServer):
    # Margen para los módulos que Chrome solicita en paralelo y varias pestañas,
    # frente a la cola predeterminada de cinco conexiones de este runtime.
    request_queue_size = 32


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--list-ports", action="store_true")
    parser.add_argument("--wifi", type=wifi_url, action="append", default=[], metavar="socket://IP:PUERTO",
                        help="Añade una fuente TCP del ESP32 (p. ej. socket://192.168.4.1:3333)")
    args = parser.parse_args()
    WIFI_URLS.extend(args.wifi or [DEFAULT_WIFI_URL])
    if args.list_ports:
        print(json.dumps({"ports": port_inventory(), "serial_available": serial is not None}, ensure_ascii=False))
        return
    monitor = Monitor()
    server = PanelHTTPServer(("127.0.0.1", args.port), Handler)
    server.monitor = monitor
    server.history = History(BASE / "sesiones" / "historial", SIGNALS)
    server.token = secrets.token_urlsafe(32)
    print(f"Panel local: http://127.0.0.1:{server.server_port}/ — lectura sin TX; sin puerto serie abierto", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        monitor.disconnect()
        server.server_close()


if __name__ == "__main__":
    main()
