"""Regresión del asistente: datos sintéticos, ningún puerto físico."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from server import Monitor
from calibracion import PLAN


class GuidedTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.now=0
        self.seq=0
        self.monitor=Monitor(clock=lambda:self.now,session_dir=self.temp.name)
        self.monitor.status='connected'
        self.monitor.port='QA-SINTETICO'
        self.cal=self.monitor.calibration
        self.frame()
        self.action('guided-begin',prepared=True)

    def frame(self,raw=2300,dt=.2,session='qa',**signals):
        self.now=round(self.now+dt,5)
        self.seq+=1
        self.monitor.ingest(json.dumps({'v':1,'type':'telemetry','node':'esp32','fw':'QA',
            'session':session,'seq':self.seq,'tick_ms':round(self.now*1000),
            'signals':{'steer_raw':raw,'can_rx_count':self.seq*40,'can_rx_age_ms':4,**signals}}))

    def action(self,action,**data):
        return self.monitor.calibrate(action,data)

    def measure(self,raw=2300,jitter=False):
        self.frame(raw)
        self.action('guided-measure')
        for i in range(70):
            self.frame(raw+(i%3-1)*2,dt=(.17,.26,.23)[i%3] if jitter else .2)
            if not self.cal.capture:break
        self.assertIsNone(self.cal.capture)

    def test_movement_before_confirmation_does_not_abort_or_save(self):
        for raw in [2300,1800,0,500,4095,2300]:self.frame(raw)
        self.assertIsNone(self.cal.capture)
        self.assertEqual(self.cal.guide['phase'],'positioning')
        self.assertEqual(self.cal.run['captures'],[])
        rows=(self.cal.directory/self.cal.run['id']/'samples.jsonl').read_text()
        self.assertIn('positioning_sample',rows)

    def test_all_positions_require_explicit_confirmation_with_jitter(self):
        for kind,*_ in PLAN:
            self.assertEqual(self.cal.guide['phase'],'positioning')
            self.measure({'left':550,'right':3600}.get(kind,2300),jitter=True)
        self.assertEqual(self.cal.guide['state'],'complete')
        pid=self.action('guided-use',name='QA pasos')['id']
        self.assertEqual(self.cal.active['id'],pid)
        self.assertEqual(len(self.cal.run['captures']),13)

    def test_limit_retries_only_current_step_and_keeps_original(self):
        self.measure()
        self.frame(550)
        self.action('guided-measure')
        self.frame(0)
        self.assertEqual(self.cal.guide['issue'],'limit')
        self.assertEqual(len(self.cal.run['captures']),1)
        self.assertEqual(self.cal.run['attempts'][-1]['statistics']['min'],0)
        self.frame(550)
        self.action('guided-resume')
        self.assertIsNone(self.cal.capture)
        self.measure(550)
        self.assertEqual(len(self.cal.run['captures']),2)

    def test_noise_has_bounded_end_and_never_saves_unstable_center(self):
        self.action('guided-measure')
        for i in range(60):
            self.frame(2300+(-180 if i%2 else 180))
            if not self.cal.capture:break
        self.assertIsNone(self.cal.capture)
        self.assertLess(self.now,13)
        self.assertEqual(self.cal.guide['issue'],'noise')
        self.assertEqual(self.cal.run['captures'],[])
        self.assertEqual(self.cal.run['attempts'][-1]['statistics']['peak_to_peak'],360)

    def test_wrong_side_can_be_repositioned_without_losing_previous(self):
        self.measure()
        self.measure(550)
        self.measure()
        self.measure(550)  # pide derecha, vuelve a izquierda
        self.assertEqual(self.cal.guide['issue'],'position')
        self.assertEqual(len(self.cal.run['captures']),3)
        self.action('guided-resume')
        self.measure(3600)
        self.assertEqual(len(self.cal.run['captures']),4)

    def test_connection_loss_keeps_steps_but_new_session_requires_restart(self):
        self.measure()
        self.now+=2
        snap=self.monitor.snapshot()['calibration']
        self.assertEqual(snap['guide']['state'],'paused')
        self.frame(550)
        self.action('guided-resume')
        self.measure(550)
        self.frame(session='new')
        self.assertFalse(self.monitor.snapshot()['calibration']['same_context'])
        with self.assertRaises(ValueError):self.action('guided-resume')
        self.assertEqual(len(self.cal.run['captures']),2)

    def test_sequence_gap_during_measurement_cannot_be_accepted(self):
        self.action('guided-measure')
        self.frame()
        self.seq+=1
        self.frame()
        self.assertEqual(self.cal.guide['issue'],'connection')
        self.assertEqual(self.cal.run['captures'],[])
        self.assertIn('secuencia',self.cal.guide['detail'])

    def test_sparse_but_fresh_data_has_bounded_sample_timeout(self):
        self.action('guided-measure')
        for _ in range(20):
            self.frame(dt=1)
            if not self.cal.capture:break
        self.assertEqual(self.cal.guide['issue'],'samples')
        self.assertIsNone(self.cal.capture)

    def test_cancel_preserves_readings_and_prevents_measuring(self):
        self.action('guided-measure')
        self.frame()
        self.action('guided-cancel')
        with self.assertRaises(ValueError):self.action('guided-measure')
        self.assertEqual(self.cal.run['attempts'][-1]['status'],'rejected')

if __name__=='__main__':unittest.main()
