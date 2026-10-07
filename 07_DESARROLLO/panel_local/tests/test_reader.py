import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
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


if __name__ == "__main__":
    unittest.main()
