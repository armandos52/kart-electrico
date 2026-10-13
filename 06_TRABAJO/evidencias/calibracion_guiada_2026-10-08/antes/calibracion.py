"""Calibración de visualización en PC. Nunca escribe en el transporte del kart.

Todos los métodos se invocan bajo Monitor.lock. El reloj monotónico se inyecta
para probar ventanas y pérdidas sin alterar duraciones en producción.
"""
from __future__ import annotations

import csv
from datetime import datetime, timezone
import hashlib
import io
import json
import math
from pathlib import Path
import statistics
import uuid
import zipfile

SCHEMA = "steering-cal-v1"
PANEL_VERSION = "0.2.0-direccion"
PLAN = [("center", 0, 10, 50)] + [(kind, cycle, 5, 25)
        for cycle in range(1, 4) for kind in ("left", "center_left", "right", "center_right")]
LABELS = {"center": "Centro físico", "left": "Tope izquierdo", "right": "Tope derecho",
          "center_left": "Centro desde izquierda", "center_right": "Centro desde derecha"}


def utc():
    return datetime.now(timezone.utc).isoformat()


def finite(value):
    return type(value) in (int, float) and math.isfinite(value)


def software_evidence():
    base=Path(__file__).resolve().parent
    names=["server.py","calibracion.py","static/app.js","static/nucleo.js","static/calibracion.js",
           "static/canales.js","static/graficas.js","static/vista_kart.js","static/index.html","static/style.css"]
    return {"panel_version":PANEL_VERSION, "sha256":{name:hashlib.sha256((base/name).read_bytes()).hexdigest() for name in names}}


def write_json(path, data):
    temp = path.with_suffix(".tmp")
    temp.write_text(json.dumps(data, ensure_ascii=False, allow_nan=False, indent=2), encoding="utf-8")
    temp.replace(path)


def stats(rows):
    values = [row["frame"]["signals"]["steer_raw"] for row in rows]
    return {"n": len(values), "median": statistics.median(values), "min": min(values),
            "max": max(values), "peak_to_peak": max(values)-min(values),
            "stdev": statistics.stdev(values) if len(values)>1 else 0,
            "duration_s": rows[-1]["elapsed_s"]-rows[0]["elapsed_s"]}


def references(captures):
    if len(captures) != len(PLAN):
        raise ValueError("Completa el centro y los tres ciclos de retorno antes de guardar un perfil")
    c = captures[0]["statistics"]["median"]
    l = statistics.median(x["statistics"]["median"] for x in captures if x["kind"] == "left")
    r = statistics.median(x["statistics"]["median"] for x in captures if x["kind"] == "right")
    sign = 1 if r>l else -1
    if not sign*l < sign*c < sign*r:
        raise ValueError("El centro no está entre los topes. Revisa posiciones, señal y saturación")
    return {"left": l, "center": c, "right": r, "orientation": sign,
            "left_span": abs(c-l), "right_span": abs(r-c)}


def convert(raw, profile):
    ref = profile["references"]
    x, c = raw*ref["orientation"], ref["center"]*ref["orientation"]
    u = (x-c)/(ref["left_span"] if x<c else ref["right_span"])
    outside = not -1 <= u <= 1
    angles = profile.get("angles")
    return {"percent": 100*u, "outside": outside,
            "angle": None if outside or not angles else u*angles["left" if u<0 else "right"],
            "target": angles["target"] if angles else None}


class SteeringCalibration:
    def __init__(self, directory, clock):
        self.directory = Path(directory)
        self.clock = clock
        self.latest = None
        self.run = None
        self.capture = None
        self.active = None
        self.error = None
        self.profiles = {}
        self.load_errors = []
        if self.directory.exists():
            for path in self.directory.glob("profiles/*.json"):
                try:
                    p = json.loads(path.read_text(encoding="utf-8"))
                    # Recalcular referencias y criterios también al cargar disco.
                    if p["schema"] != SCHEMA or p["id"] != path.stem or not self.valid_id(p["id"]):
                        raise ValueError("Identidad/esquema no válido")
                    expected = self.review(p["captures"], p["criteria"], p.get("angles"))
                    if p["references"] != expected["references"]:
                        raise ValueError("Referencias no coherentes con las capturas")
                    self.profiles[p["id"]] = p
                except (ValueError, KeyError, TypeError, AttributeError, OSError) as exc:
                    self.load_errors.append(path.name+": "+str(exc))

    @staticmethod
    def valid_id(value):
        return isinstance(value, str) and len(value)==32 and all(c in "0123456789abcdef" for c in value)

    def append(self, record):
        if not self.run:
            return
        path = self.directory / self.run["id"] / "samples.jsonl"
        with path.open("a", encoding="utf-8") as file:
            file.write(json.dumps(record, ensure_ascii=False, allow_nan=False)+"\n")

    def persist(self):
        if self.run:
            write_json(self.directory/self.run["id"]/"run.json", self.run)
            with (self.directory/self.run["id"]/"summary.csv").open("w", encoding="utf-8", newline="") as file:
                writer=csv.writer(file)
                writer.writerow(["attempt", "kind", "cycle", "status", "reason", "n", "median_counts", "min_counts", "max_counts", "peak_to_peak_counts", "stdev_counts", "duration_s"])
                for row in self.run["attempts"]:
                    s=row.get("statistics", {})
                    writer.writerow([row["id"], row["kind"], row["cycle"], row["status"], row.get("reason", "")]+[s.get(k, "") for k in ("n", "median", "min", "max", "peak_to_peak", "stdev", "duration_s")])

    def interrupt(self, reason):
        self.active = None
        if self.capture:
            self.finish("rejected", reason)

    def finish(self, status, reason=""):
        cap = self.capture
        if not cap:
            return
        result={k:v for k,v in cap.items() if k not in ("rows", "start", "last")}
        result.update(status=status, reason=reason, ended_utc=utc())
        if cap["rows"]:
            result["statistics"]=stats(cap["rows"])
        self.capture=None
        self.run["attempts"].append(result)
        if status=="completed":
            self.run["captures"].append(result)
        self.persist()

    def context(self, frame, port):
        return {"source": port, "node": frame["node"], "session": frame["session"],
                "firmware": frame.get("fw"), "protocol": frame["v"], "channel": "can_steer_normalized"}

    def observe(self, frame, port, session_file):
        if frame["node"] != "esp32":
            return
        now=self.clock()
        ctx=self.context(frame, port)
        previous=self.latest
        self.latest={"frame":frame, "context":ctx, "received":now, "session_file":session_file}
        if previous and ctx != previous["context"]:
            self.interrupt("Cambió la fuente, firmware o sesión; confirma de nuevo la configuración")
        if not self.capture:
            return
        cap=self.capture
        reason=self.problem()
        if not reason and now-cap["last"]>1.5:
            reason="Pérdida de recepción durante la captura"
        if not reason and previous:
            old=previous["frame"]
            if ((frame["seq"]-old["seq"]) & 0xFFFFFFFF)!=1:
                reason="Salto de secuencia durante la captura"
            elif not 0 < ((frame["tick_ms"]-old["tick_ms"]) & 0xFFFFFFFF) < 0x80000000:
                reason="Reloj del nodo repetido o fuera de orden"
            elif not 0 < ((int(frame["signals"]["can_rx_count"])-int(old["signals"].get("can_rx_count",0))) & 0xFFFFFFFF) < 0x80000000:
                reason="La orden CAN no se ha renovado o el contador retrocedió"
        row={"kind":"sample", "attempt":cap["id"], "received_utc":utc(),
             "elapsed_s":now-cap["start"], "frame":frame, "valid":not bool(reason)}
        self.append(row)
        if reason:
            self.interrupt(reason)
            return
        cap["rows"].append(row)
        cap["last"]=now
        if stats(cap["rows"])["duration_s"]>=cap["duration_s"] and len(cap["rows"])>=cap["min_samples"]:
            self.finish("completed")

    def problem(self):
        if not self.latest or self.clock()-self.latest["received"]>1.5:
            return "Sin trama ESP32 reciente"
        frame=self.latest["frame"]
        v=frame["signals"]
        if not frame.get("fw"):
            return "La trama no identifica el firmware"
        if not finite(v.get("steer_raw")) or not 0<=v["steer_raw"]<=4095:
            return "Falta dirección CAN normalizada válida (0–4095 cuentas)"
        if not finite(v.get("can_rx_age_ms")) or v["can_rx_age_ms"]>100:
            return "Orden CAN caducada o sin diagnóstico de edad"
        if not finite(v.get("can_rx_count")) or v["can_rx_count"]<=0:
            return "Sin órdenes CAN identificadas"
        return None

    def tick(self, connected):
        reason=None if connected else "Fuente desconectada"
        reason=reason or self.problem()
        if reason:
            try:
                self.interrupt(reason)
            except OSError as exc:
                self.error="No se pudo conservar el cierre de captura: "+str(exc)
        return reason

    def begin(self, data):
        if self.capture:
            raise ValueError("Espera o cancela la captura actual")
        if data.get("prepared") is not True:
            raise ValueError("Confirma tracción inhabilitada y ruedas preparadas")
        metadata={}
        for key in ("configuration", "alignment", "conditions"):
            value=data.get(key)
            if not isinstance(value,str) or not value.strip() or len(value)>600:
                raise ValueError("Completa configuración, alineación y condiciones (máx. 600 caracteres)")
            metadata[key]=value.strip()
        if self.problem():
            raise ValueError(self.problem())
        rid=uuid.uuid4().hex
        directory=self.directory/rid
        directory.mkdir(parents=True)
        self.run={"schema":SCHEMA, "id":rid, "created_utc":utc(), "timezone":"Europe/Lisbon",
                  "metadata":metadata, "prepared_declared":True, "context":self.latest["context"],
                  "session_file":self.latest["session_file"], "software":software_evidence(), "captures":[], "attempts":[]}
        self.append({"kind":"begin", "run":{k:v for k,v in self.run.items() if k not in ("captures","attempts")}})
        self.persist()
        self.error=None

    def start_capture(self):
        if not self.run or self.capture:
            raise ValueError("Prepara un ensayo o espera a que termine la captura")
        if self.problem():
            raise ValueError(self.problem())
        if self.run["context"]!=self.latest["context"]:
            raise ValueError("Fuente/sesión distinta: prepara un nuevo ensayo")
        i=len(self.run["captures"])
        if i>=len(PLAN):
            raise ValueError("Capturas completas; revisa y guarda el perfil")
        kind,cycle,duration,count=PLAN[i]
        now=self.clock()
        self.capture={"id":uuid.uuid4().hex, "kind":kind, "cycle":cycle, "duration_s":duration,
                      "min_samples":count, "started_utc":utc(), "start":now, "last":now, "rows":[]}
        self.append({"kind":"capture_started", "attempt":self.capture["id"], "position":kind,"cycle":cycle})

    @staticmethod
    def review(captures, criteria, angles):
        for key in ("max_peak_to_peak", "max_return_error", "min_span"):
            if not finite(criteria.get(key)) or criteria[key]<=0:
                raise ValueError("Define tolerancias positivas en cuentas para variación, retorno y recorrido mínimo")
        ref=references(captures)
        maximum=max(c["statistics"]["peak_to_peak"] for c in captures)
        center_error=max(abs(c["statistics"]["median"]-ref["center"]) for c in captures if c["kind"].startswith("center"))
        for cap in captures:
            if cap["statistics"]["min"]<=0 or cap["statistics"]["max"]>=4095:
                raise ValueError("Saturación 0/4095: guarda el ensayo y revisa la señal antes de activar")
        if maximum>criteria["max_peak_to_peak"] or center_error>criteria["max_return_error"]:
            raise ValueError("Variación o retorno superan las tolerancias elegidas; el ensayo queda conservado")
        if min(ref["left_span"],ref["right_span"])<criteria["min_span"]:
            raise ValueError("Recorrido insuficiente para la separación mínima elegida")
        # Las bandas observadas de ambos topes y centros deben ser disjuntas.
        sign=ref["orientation"]
        bands={}
        for key in ("left","center","right"):
            caps=[c for c in captures if c["kind"].startswith(key)]
            values=[sign*c["statistics"][k] for c in caps for k in ("min","max")]
            bands[key]=(min(values),max(values))
        if not bands["left"][1]<bands["center"][0] or not bands["center"][1]<bands["right"][0]:
            raise ValueError("Las bandas de centro y extremos se solapan")
        if angles is not None:
            if angles.get("target") not in ("wheel_left","wheel_right","steering_wheel"):
                raise ValueError("Identifica qué rueda o volante se midió")
            if any(not finite(angles.get(k)) or angles[k]<=0 for k in ("left","right","resolution")):
                raise ValueError("Introduce magnitudes angulares y resolución positivas")
            if not isinstance(angles.get("instrument"),str) or not angles["instrument"].strip() or len(angles["instrument"])>200:
                raise ValueError("Identifica el instrumento de referencia")
        return {"references":ref, "max_peak_to_peak":maximum, "max_return_error":center_error}

    def save(self, data):
        if not self.run or self.capture:
            raise ValueError("Termina las capturas antes de guardar")
        name=data.get("name")
        if not isinstance(name,str) or not name.strip() or len(name)>100:
            raise ValueError("Indica un nombre de perfil (máx. 100 caracteres)")
        if data.get("reviewed") is not True:
            raise ValueError("Confirma revisión de capturas y tolerancias")
        criteria=data.get("criteria",{})
        angles=data.get("angles")
        review=self.review(self.run["captures"],criteria,angles)
        pid=uuid.uuid4().hex
        profile={"schema":SCHEMA,"id":pid,"name":name.strip(),"created_utc":utc(),
                 "run_id":self.run["id"],"metadata":self.run["metadata"],"context":self.run["context"],
                 "captures":self.run["captures"],"criteria":criteria,"angles":angles,**review,
                 "software":self.run["software"],
                 "scope":"pc_visualization_only","intermediate_angles_validation":"pending",
                 "samples_sha256":hashlib.sha256((self.directory/self.run["id"]/"samples.jsonl").read_bytes()).hexdigest()}
        (self.directory/"profiles").mkdir(exist_ok=True)
        write_json(self.directory/"profiles"/(pid+".json"),profile)
        self.profiles[pid]=profile
        return pid

    def activate(self, data):
        if self.capture:
            raise ValueError("No se cambia el perfil durante una captura")
        if self.problem():
            raise ValueError(self.problem())
        profile=self.profiles.get(data.get("id"))
        if not profile or data.get("compatible") is not True:
            raise ValueError("Selecciona perfil y confirma el mismo montaje físico")
        for key in ("source","firmware","protocol","node","channel"):
            if profile["context"][key]!=self.latest["context"][key]:
                raise ValueError("El perfil no coincide con la fuente/firmware actual")
        self.active=profile

    def snapshot(self, connected):
        reason=self.tick(connected)
        captures=self.run["captures"] if self.run else []
        i=len(captures)
        step=PLAN[i] if i<len(PLAN) else None
        current=None
        if self.capture:
            cap=self.capture
            current={"kind":cap["kind"],"n":len(cap["rows"]),"duration_s":cap["duration_s"],
                     "min_samples":cap["min_samples"],"elapsed_s":self.clock()-cap["start"],
                     "statistics":stats(cap["rows"]) if cap["rows"] else None}
        value=None if reason else self.latest["frame"]["signals"]["steer_raw"]
        return {"schema":SCHEMA,"ready":not bool(reason),"reason":reason,"error":self.error,
                "load_errors":self.load_errors,"run":self.run,"capture":current,"total_steps":len(PLAN),
                "next":None if not step else {"label":LABELS[step[0]],"kind":step[0],"cycle":step[1],"duration_s":step[2],"min_samples":step[3]},
                "raw":value,"old_angle":None if value is None else max(-20,min(20,(value-2047)/47.9)),
                "active":self.active,"profiles":list(self.profiles.values()),
                "calibrated":convert(value,self.active) if value is not None and self.active else None}

    def export(self, run_id, profile_id=None):
        if profile_id:
            p=self.profiles.get(profile_id)
            if not p:
                raise ValueError("Perfil no encontrado")
            run_id=p["run_id"]
        if not self.valid_id(run_id) or not (self.directory/run_id/"run.json").is_file():
            raise ValueError("Ensayo no encontrado")
        content={name:(self.directory/run_id/name).read_bytes() for name in ("run.json","samples.jsonl","summary.csv")}
        if profile_id:
            content["profile.json"]=(self.directory/"profiles"/(profile_id+".json")).read_bytes()
        content["sha256.json"]=json.dumps({name:hashlib.sha256(data).hexdigest() for name,data in content.items()},indent=2).encode()
        out=io.BytesIO()
        with zipfile.ZipFile(out,"w",zipfile.ZIP_DEFLATED) as z:
            for name,data in content.items():
                z.writestr(name,data)
        return out.getvalue()
