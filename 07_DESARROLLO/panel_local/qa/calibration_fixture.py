"""Servidor EXCLUSIVO de QA sintético; jamás abre puertos del kart.
Las rutas /qa/* no están en server.py ni se cargan en la aplicación normal.
"""
import argparse
import json
from pathlib import Path
import secrets
import sys

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from server import Monitor, Handler, PanelHTTPServer

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--port',type=int,default=8877)
    parser.add_argument('--data',required=True)
    args=parser.parse_args()
    clock=[0.0]
    monitor=Monitor(clock=lambda:clock[0],session_dir=args.data)
    monitor.status='connected'
    monitor.port='QA-SINTETICO-SIN-HARDWARE'
    sequence=[0]
    def frame(raw=2180,can_age=4):
        clock[0]=round(clock[0]+.2,5);sequence[0]+=1
        monitor.ingest(json.dumps({'v':1,'type':'telemetry','node':'esp32','session':'qa-boot',
          'fw':'QA-SINTETICO','seq':sequence[0],'tick_ms':round(clock[0]*1000),
          'signals':{'steer_raw':raw,'can_rx_age_ms':can_age,'can_rx_count':sequence[0]*40,'control_state':'running'}}))
    frame()
    class QAHandler(Handler):
        def do_POST(self):
            if self.path.startswith('/qa/'):
                if not self._allowed_host() or self.headers.get('X-Kart-Token')!=self.server.token:
                    return self._respond(403,{'error':'Token QA requerido'})
                with monitor.lock:
                    if self.path=='/qa/reset-workflow':
                        monitor.calibration.cancel_guided()
                        monitor.calibration.run=None
                        monitor.calibration.guide=None
                        monitor.calibration.active=None
                        frame()
                    elif self.path=='/qa/finish-capture':
                        if not monitor.calibration.capture:return self._respond(400,{'error':'Sin captura'})
                        raw={'left':500,'right':3500}.get(monitor.calibration.capture['kind'],2180)
                        for i in range(60):
                            frame(raw+(i%3-1)*2)
                            if not monitor.calibration.capture:break
                    elif self.path=='/qa/stale':clock[0]+=2
                    elif self.path=='/qa/resume':frame()
                    elif self.path=='/qa/outside':frame(3800)
                    elif self.path=='/qa/limit':frame(0)
                    elif self.path=='/qa/partial':
                        for _ in range(15):frame()
                    elif self.path=='/qa/noise':
                        for i in range(60):
                            frame(2180+(-180 if i%2 else 180))
                            if not monitor.calibration.capture:break
                    else:return self._respond(404,{'error':'QA desconocida'})
                return self._respond(200,{'ok':True})
            super().do_POST()
    http=PanelHTTPServer(('127.0.0.1',args.port),QAHandler)
    http.monitor=monitor;http.token=secrets.token_urlsafe(32)
    try:http.serve_forever()
    finally:http.server_close()

if __name__=='__main__':main()
