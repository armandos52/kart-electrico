"""Contrato v1.3: datos sintéticos unitarios, sin simulación del vehículo."""
import json
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from server import Monitor, SIGNALS

def frame(seq=1,fw='esp32_trasero-v1.3-dev2',**signals):
    return json.dumps(dict(v=1,type='telemetry',node='esp32',session='unit',seq=seq,tick_ms=seq*200,fw=fw,signals=signals))

class RobustezTests(unittest.TestCase):
    def setUp(self):
        self.monitor=Monitor(clock=lambda:10)
        self.monitor.status='connected'

    def test_all_new_inhibits_and_modes(self):
        for seq,reason in enumerate(sorted(SIGNALS['drive_inhibit'][2]),1):
            self.monitor.ingest(frame(seq,drive_inhibit=reason,mode_applied=255,rearm_required=1,reverse_limited=0))
        self.assertEqual(self.monitor.stats['invalid_frames'],0)
        self.monitor.ingest(frame(100,mode_applied=3))
        self.assertEqual(self.monitor.stats['invalid_frames'],1)

    def test_sensor_failure_masks_old_readings(self):
        self.monitor.ingest(frame(rpm_rl=100,rpm_rr=101,speed_valid=0,speed_kph_max_wheel=9,mpu_ok=0,yaw_dps=15))
        fields=self.monitor.snapshot()['signals']
        for key in ['rpm_rl','rpm_rr','speed_kph_max_wheel','yaw_dps']:self.assertEqual(fields[key]['quality'],'stale')
        self.assertIn('reciente',fields['mpu_ok']['label'])

    def test_legacy_mpu_label_remains_startup(self):
        self.monitor.ingest(frame(fw='esp32_trasero-v1.2-diagnostico',mpu_ok=0,yaw_dps=0,rpm_rl=0))
        fields=self.monitor.snapshot()['signals']
        self.assertEqual(fields['rpm_rl']['quality'],'fresh')
        self.assertIn('arrancar',fields['mpu_ok']['label'])

    def test_calibration_rejects_bad_frontend(self):
        self.monitor.ingest(frame(steer_raw=2047,can_rx_age_ms=1,can_rx_count=10,frontend_health_seen=1,frontend_health_ok=0))
        self.assertIn('STM32',self.monitor.calibration.problem())

    def test_full_v13_field_budget(self):
        values={k:(sorted(v[2])[0] if isinstance(v[2],set) else v[2]) for k,v in SIGNALS.items()}
        # El firmware no transmite los campos de BMS/frontal aún no recibidos.
        for key in ['soc_pct','battery_v','battery_a','battery_temp_c','sd_ready','rpm_fl','rpm_fr','steer_deg']:values.pop(key,None)
        self.assertLessEqual(len(values),64)
        self.monitor.ingest(frame(**values))
        self.assertEqual(self.monitor.stats['valid_frames'],1)

if __name__=='__main__':unittest.main()
