"""Ensayos automáticos sintéticos; no se conecta hardware."""
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
import zipfile

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from server import Monitor
from calibracion import SteeringCalibration, PLAN, convert


class CalibrationTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.now=0.0
        self.seq=0
        self.monitor=Monitor(clock=lambda:self.now,session_dir=self.temp.name)
        self.monitor.status="connected"
        self.monitor.port="socket://127.0.0.1:3333"
        self.cal=self.monitor.calibration
        self.frame()

    def frame(self, raw=2180, session="test-boot", fw="test-fw", **values):
        self.now=round(self.now+.2,5)
        self.seq+=1
        signals={"steer_raw":raw,"can_rx_count":self.seq*40,"can_rx_age_ms":4,**values}
        frame={"v":1,"type":"telemetry","node":"esp32","session":session,"fw":fw,
               "seq":self.seq,"tick_ms":round(self.now*1000),"signals":signals}
        self.monitor.ingest(json.dumps(frame))
        return frame

    def begin(self):
        self.monitor.calibrate("begin",{"prepared":True,"configuration":"SINTÉTICO", "alignment":"Referencia simulada", "conditions":"Sin hardware"})

    def complete(self, reverse=False):
        self.begin()
        for kind,cycle,duration,count in PLAN:
            self.monitor.calibrate("capture",{})
            target={"left":500,"right":3500}.get(kind,2180)
            for n in range(60):
                self.frame((4095-target if reverse else target) + ((n%3)-1)*2)
                if not self.cal.capture:
                    break
            self.assertIsNone(self.cal.capture)
        self.assertEqual(len(self.cal.run["captures"]),13)

    def save(self, **overrides):
        data={"name":"Perfil sintético", "reviewed":True,
              "criteria":{"max_peak_to_peak":10,"max_return_error":5,"min_span":100},"angles":None,**overrides}
        return self.monitor.calibrate("save",data)["id"]

    def test_capture_counts_original_frames_not_browser_polls_and_keeps_noise(self):
        self.begin()
        self.monitor.calibrate("capture",{})
        for i in range(15):
            self.frame(2180+(i%3-1)*20)
            for _ in range(10):self.monitor.snapshot()
        snap=self.monitor.snapshot()["calibration"]["capture"]
        self.assertEqual(snap["n"],15)
        self.assertEqual(snap["statistics"]["peak_to_peak"],40)
        self.assertIsNotNone(self.cal.capture)

    def test_complete_profiles_asymmetry_persistence_and_export_originals(self):
        self.complete()
        pid=self.save()
        self.monitor.calibrate("activate",{"id":pid,"compatible":True})
        profile=self.cal.active
        self.assertEqual(profile["references"]["left_span"],1680)
        self.assertEqual(profile["references"]["right_span"],1320)
        self.assertIsNone(convert(2180,profile)["angle"])
        self.assertEqual(convert(2180,profile)["percent"],0)
        self.assertEqual(convert(500,profile)["percent"],-100)
        self.assertTrue(convert(4000,profile)["outside"])
        loaded=SteeringCalibration(self.cal.directory,lambda:self.now)
        self.assertIn(pid,loaded.profiles)
        self.assertIsNone(loaded.active)
        with zipfile.ZipFile(io.BytesIO(self.cal.export(None,pid))) as z:
            self.assertIn("samples.jsonl",z.namelist())
            self.assertIn("sha256.json",z.namelist())
            lines=[json.loads(line) for line in z.read("samples.jsonl").splitlines()]
            samples=[x for x in lines if x["kind"]=="sample"]
            self.assertGreater(len(samples),300)
            self.assertIn("tick_ms",samples[0]["frame"])
            self.assertIn("received_utc",samples[0])
        pid2=self.save(name="Revisión 2")
        self.assertNotEqual(pid,pid2)
        self.assertEqual(len(self.cal.profiles),2)

    def test_reverse_sensor_and_independent_angular_references(self):
        self.complete(reverse=True)
        pid=self.save(angles={"target":"wheel_left","left":24,"right":19,"resolution":.5,"instrument":"Transportador simulado"})
        p=self.cal.profiles[pid]
        self.assertEqual(p["references"]["orientation"],-1)
        self.assertEqual(convert(4095-500,p)["angle"],-24)
        self.assertEqual(convert(4095-3500,p)["angle"],19)

    def test_disconnect_stale_can_sequence_gap_duplicate_and_reset_reject_attempt(self):
        for scenario in ("disconnect","stale","gap","duplicate","reset","missing","can_stopped","can_backwards"):
            with self.subTest(scenario=scenario):
                self.monitor.status="connected"
                self.frame()
                self.begin()
                self.monitor.calibrate("capture",{})
                last=self.frame()
                if scenario=="disconnect":self.monitor.status="disconnected";self.monitor.snapshot()
                if scenario=="stale":self.frame(can_rx_age_ms=101)
                if scenario=="gap":self.seq+=1;self.frame()
                if scenario=="duplicate":self.monitor.ingest(json.dumps(last))
                if scenario=="reset":self.frame(session="new-boot")
                if scenario=="missing":self.now+=2;self.monitor.snapshot()
                if scenario=="can_stopped":self.frame(can_rx_count=last["signals"]["can_rx_count"])
                if scenario=="can_backwards":self.frame(can_rx_count=last["signals"]["can_rx_count"]-1)
                self.assertIsNone(self.cal.capture)
                self.assertEqual(self.cal.run["attempts"][-1]["status"],"rejected")
                self.assertEqual(len(self.cal.run["captures"]),0)

    def test_integer_valued_float_can_counter_does_not_break_capture(self):
        self.begin()
        self.monitor.calibrate("capture",{})
        self.frame(can_rx_count=float((self.seq+1)*40))
        self.assertEqual(len(self.cal.capture["rows"]),1)

    def test_tolerances_saturation_invalid_angles_and_incomplete_run_block_save(self):
        self.begin()
        with self.assertRaises(ValueError):self.save()
        self.complete()
        with self.assertRaises(ValueError):self.save(criteria={"max_peak_to_peak":1,"max_return_error":5,"min_span":100})
        with self.assertRaises(ValueError):self.save(angles={"target":"wheel_left","left":float("nan"),"right":19,"resolution":.5,"instrument":"test"})
        self.cal.run["captures"][1]["statistics"]["min"]=0
        with self.assertRaises(ValueError):self.save()
        self.assertEqual(len(self.cal.profiles),0)

    def test_active_profile_requires_confirmation_and_is_invalidated_on_loss(self):
        self.complete()
        pid=self.save()
        with self.assertRaises(ValueError):self.monitor.calibrate("activate",{"id":pid})
        self.monitor.calibrate("activate",{"id":pid,"compatible":True})
        self.now+=2
        self.assertIsNone(self.monitor.snapshot()["calibration"]["active"])
        self.frame(fw="other-fw")
        with self.assertRaises(ValueError):self.monitor.calibrate("activate",{"id":pid,"compatible":True})
        self.frame()
        self.monitor.calibrate("activate",{"id":pid,"compatible":True})
        self.frame(session="boot-new")
        self.assertIsNone(self.cal.active)

    def test_no_save_with_nonfinite_criteria_and_no_path_escape(self):
        self.complete()
        with self.assertRaises(ValueError):self.save(criteria={"max_peak_to_peak":float("inf"),"max_return_error":5,"min_span":100})
        with self.assertRaises(ValueError):self.cal.export("../../server.py")

if __name__=="__main__":unittest.main()
