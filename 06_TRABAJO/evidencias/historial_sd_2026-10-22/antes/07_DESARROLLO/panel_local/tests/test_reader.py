import argparse
import json
from pathlib import Path
import socket
import sys
import tempfile
import threading
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import server
from server import Monitor


def frame(seq=1, session="boot-a", **signals):
    return json.dumps({"v": 1, "type": "telemetry", "node": "esp32", "session": session,
                       "seq": seq, "tick_ms": 100, "signals": signals})


class ReaderTests(unittest.TestCase):
    def setUp(self):
        self.now = 10
        self.monitor = Monitor(clock=lambda: self.now)
        self.monitor.status = "connected"

    def test_partial_telemetry_does_not_refresh_an_older_field(self):
        self.monitor.ingest(frame(rpm_rl=123, relay1=0))
        self.now += 2
        self.monitor.ingest(frame(seq=2, relay1=1))
        data = self.monitor.snapshot()["signals"]
        self.assertEqual(data["rpm_rl"]["quality"], "stale")
        self.assertEqual(data["relay1"]["quality"], "fresh")
        self.monitor.status = "disconnected"
        self.assertEqual(self.monitor.snapshot()["signals"]["relay1"]["quality"], "stale")

    def test_invalid_frame_is_atomic_and_console_text_is_not_measurement(self):
        self.monitor.ingest(frame(rpm_rl=123))
        self.monitor.ingest(frame(seq=2, rpm_rl=200, relay1=3))
        self.assertEqual(self.monitor.samples["rpm_rl"]["value"], 123)
        self.monitor.ingest("CAN OK!")
        self.assertEqual(self.monitor.stats["valid_frames"], 1)
        self.assertIn("CAN OK!", self.monitor.messages[-1]["text"])
        self.assertEqual(self.monitor.snapshot()["signals"]["battery_v"]["quality"], "missing")

    def test_nonfinite_boolean_duplicate_and_out_of_order_are_rejected(self):
        self.monitor.ingest(frame(seq=10, rpm_rl=123))
        for seq, value in [(11, float("nan")), (11, True), (10, 200), (9, 201)]:
            self.monitor.ingest(frame(seq=seq, rpm_rl=value))
        wrong_version = json.loads(frame(seq=11, rpm_rl=200))
        wrong_version["v"] = True
        self.monitor.ingest(json.dumps(wrong_version))
        self.assertEqual(self.monitor.stats["invalid_frames"], 5)
        self.assertEqual(self.monitor.samples["rpm_rl"]["value"], 123)

    def test_new_boot_invalidates_previous_fields_and_sequence_rollover_is_allowed(self):
        self.monitor.ingest(frame(seq=0xFFFFFFFF, rpm_rl=100))
        self.monitor.ingest(frame(seq=0, rpm_rl=110))
        self.assertEqual(self.monitor.stats["valid_frames"], 2)
        self.monitor.ingest(frame(seq=1, session="boot-b", relay1=0))
        self.assertNotIn("rpm_rl", self.monitor.samples)
        self.assertEqual(self.monitor.samples["relay1"]["value"], 0)

    def test_invalid_new_boot_cannot_clear_previous_samples(self):
        self.monitor.ingest(frame(rpm_rl=123))
        broken = json.loads(frame(session="boot-b", rpm_rl=456))
        broken["fw"] = ["invalid"]
        self.monitor.ingest(json.dumps(broken))
        self.assertEqual(self.monitor.samples["rpm_rl"]["value"], 123)
        self.assertEqual(self.monitor.nodes["esp32"]["session"], "boot-a")

    def test_fragmented_lines_and_oversize_recovery_without_transmission(self):
        line = frame(rpm_rl=123).encode() + b"\n"
        chunks = iter([line[:30], b"", line[30:], b"x"*8200+b"\n", frame(seq=2,rpm_rr=124).encode()+b"\n"])
        monitor = self.monitor
        class ReadOnlyPort:
            def read(self, size):
                try:
                    return next(chunks)
                except StopIteration:
                    monitor.stop.set()
                    return b""
            def close(self):
                pass
            def write(self, data):
                raise AssertionError("El lector no debe transmitir al micro")
        monitor._read_loop(ReadOnlyPort())
        self.assertEqual(monitor.stats["valid_frames"], 2)
        self.assertEqual(monitor.stats["discarded_lines"], 1)
        self.assertEqual(monitor.samples["rpm_rr"]["value"], 124)

    def test_registers_received_telemetry_with_origin_tick(self):
        with tempfile.TemporaryDirectory() as directory:
            target = Path(directory)/"session.jsonl"
            with target.open("w",encoding="utf-8") as log:
                self.monitor.log=log
                self.monitor.ingest(frame(rpm_rl=123))
            self.monitor.log=None
            data=json.loads(target.read_text(encoding="utf-8").splitlines()[0])
            self.assertEqual(data["frame"]["tick_ms"],100)
            self.assertIn("received_utc",data)


FIRMWARE_V1 = ('{"v":1,"type":"telemetry","node":"esp32","session":"1a2b3c4d5e6f7081","seq":7,"tick_ms":5400,'
               '"fw":"esp32_trasero-v1-telemetria","signals":{"accelerator_raw":0,"accelerator_pct":0,"steer_raw":2047,'
               '"brake":0,"gear":"N","mode":"DIF","can_rx_age_ms":12,"yaw_dps":-0.3,"pwm_left_raw":255,'
               '"pwm_right_raw":255,"rpm_rl":0,"rpm_rr":0,"relay1":0,"relay2":0,"relay_seq":0,'
               '"charger_connected":0,"emergency_stop":0,"can_rx_count":53,"loop_max_us":1180,'
               '"mpu_ok":1,"telemetry_us_max":95,"telemetry_dropped":0,"wifi_ap":1,"wifi_clients":1}}')


class WifiTests(unittest.TestCase):
    def tearDown(self):
        server.WIFI_URLS.clear()

    def test_firmware_v1_frame_is_fully_known(self):
        monitor = Monitor(clock=lambda: 0)
        monitor.ingest(FIRMWARE_V1)
        self.assertEqual(monitor.stats["valid_frames"], 1)
        self.assertEqual(monitor.stats["unknown_signals"], 0)
        self.assertEqual(monitor.samples["loop_max_us"]["value"], 1180)

    def test_wifi_url_accepts_only_private_socket_addresses(self):
        self.assertEqual(server.wifi_url("socket://192.168.4.1:3333"), "socket://192.168.4.1:3333")
        for bad in ["socket://8.8.8.8:3333", "http://192.168.4.1:3333", "socket://192.168.4.1:0", "socket://kart:3333"]:
            with self.assertRaises(argparse.ArgumentTypeError):
                server.wifi_url(bad)

    def test_tcp_source_reads_lines_never_transmits_and_detects_disconnection(self):
        listener = socket.socket()
        listener.bind(("127.0.0.1", 0))
        listener.listen(1)
        url = "socket://127.0.0.1:%d" % listener.getsockname()[1]
        server.WIFI_URLS.append(url)
        received = []
        reader_ready = threading.Event()

        def esp32():
            conn, _ = listener.accept()
            # pySerial vacía el búfer al abrir. Sin sincronización, el fixture
            # podía enviar sus únicas tramas antes de que el lector estuviera listo.
            if not reader_ready.wait(timeout=3):
                conn.close()
                return
            conn.sendall(b"# kart esp32 fw=v1 reset=encendido\n" + FIRMWARE_V1.encode() + b"\n"
                         + FIRMWARE_V1.replace('"seq":7', '"seq":8').encode() + b"\n")
            conn.settimeout(0.5)
            try:
                while True:
                    data = conn.recv(64)
                    if not data:
                        break
                    received.append(data)
            except socket.timeout:
                pass
            conn.close()

        thread = threading.Thread(target=esp32, daemon=True)
        thread.start()
        with tempfile.TemporaryDirectory() as directory:
            monitor = Monitor(session_dir=directory)
            self.assertIn(url, [p["device"] for p in server.port_inventory()])
            monitor.connect(url, True)
            reader_ready.set()
            thread.join(timeout=3)
            monitor.worker.join(timeout=3)
            self.assertEqual(monitor.stats["valid_frames"], 2)
            self.assertEqual(monitor.status, "disconnected")
            self.assertEqual(received, [], "El lector no debe transmitir al ESP32")
            self.assertTrue(any("reset=encendido" in m["text"] for m in monitor.messages))
        listener.close()


class DiagnosticTests(unittest.TestCase):
    def test_new_boot_waiting_for_can_does_not_keep_motor_measurements(self):
        monitor = Monitor(clock=lambda: 0)
        monitor.status = "connected"
        monitor.ingest(frame(session="running", rpm_rl=123, pwm_left_raw=150))
        monitor.ingest(frame(session="new-boot", control_ready=0, can_init_ok=0,
                             control_state="can_wait", drive_inhibit="can_init", can_init_attempts=2))
        snapshot = monitor.snapshot()
        self.assertEqual(snapshot["signals"]["rpm_rl"]["quality"], "missing")
        self.assertEqual(snapshot["signals"]["pwm_left_raw"]["quality"], "missing")
        self.assertEqual(snapshot["signals"]["control_state"]["value"], "can_wait")

    def test_diagnostic_fields_and_reset_are_preserved(self):
        monitor = Monitor(clock=lambda: 0)
        sample = json.loads(frame(control_ready=1, can_init_ok=1, can_init_attempts=3,
                                  control_state="running", drive_inhibit="direction_change",
                                  drive_direction="N", start_requested=1))
        sample["reset_reason"] = "brownout_tension"
        monitor.ingest(json.dumps(sample))
        self.assertEqual(monitor.stats["unknown_signals"], 0)
        self.assertEqual(monitor.nodes["esp32"]["reset_reason"], "brownout_tension")
        self.assertEqual(monitor.samples["drive_inhibit"]["value"], "direction_change")

    def test_invalid_reset_metadata_cannot_invalidate_previous_boot(self):
        monitor = Monitor(clock=lambda: 0)
        monitor.ingest(frame(session="boot-a", rpm_rl=123))
        malformed = json.loads(frame(session="boot-b", control_state="can_wait"))
        malformed["reset_reason"] = ["bad"]
        monitor.ingest(json.dumps(malformed))
        self.assertEqual(monitor.nodes["esp32"]["session"], "boot-a")
        self.assertEqual(monitor.samples["rpm_rl"]["value"], 123)


if __name__ == "__main__":
    unittest.main()
