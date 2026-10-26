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
PANEL_VERSION = "0.4.0-calibracion-pasos"
# Criterios de adquisición/consistencia del PC; no tolerancias mecánicas.
GUIDED_POLICY = {"stable_peak_cap":200, "minimum_move":80, "minimum_span":100,
                 "noise_fraction":0.10, "return_fraction":0.03, "repeat_fraction":0.03}
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
           "static/canales.js","static/graficas.js","static/vista_kart.js","static/index.html","static/style.css",
           "static/i18n.js","static/textos.js"]
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


def references(captures, center_method="initial"):
    if len(captures) != len(PLAN):
        raise ValueError("Completa el centro y los tres ciclos de retorno antes de guardar un perfil")
    c = (statistics.median(x["statistics"]["median"] for x in captures if x["kind"].startswith("center"))
         if center_method=="median_of_centers" else captures[0]["statistics"]["median"])
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
        self.guide = None
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
        if self.guide and self.guide["state"]=="running":
            self.guide.update(state="paused", progress=0, issue="connection", message="La lectura se ha interrumpido. Reintenta este paso cuando vuelva la conexión.", detail=reason)
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
            result["window"]={"first_seq":cap["rows"][0]["frame"]["seq"],"last_seq":cap["rows"][-1]["frame"]["seq"],
                              "first_elapsed_s":cap["rows"][0]["elapsed_s"],"last_elapsed_s":cap["rows"][-1]["elapsed_s"]}
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
            if self.run and self.guide and self.guide["state"]=="running":
                self.append({"kind":"positioning_sample", "received_utc":utc(), "frame":frame})
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
        if self.guide and self.guide["state"]=="running":
            self.advance_guided(row)
            return
        if stats(cap["rows"])["duration_s"]>=cap["duration_s"] and len(cap["rows"])>=cap["min_samples"]:
            self.finish("completed")

    def problem(self):
        if not self.latest or self.clock()-self.latest["received"]>1.5:
            return "Sin trama ESP32 reciente"
        frame=self.latest["frame"]
        v=frame["signals"]
        if not frame.get("fw"):
            return "La trama no identifica el firmware"
        if "v1.3" in frame.get("fw", "") and v.get("frontend_health_seen") == 1 and v.get("frontend_health_ok") != 1:
            return "Diagnóstico STM32: entradas inválidas o caducadas"
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
        self.guide=None
        rid=uuid.uuid4().hex
        directory=self.directory/rid
        directory.mkdir(parents=True)
        self.run={"schema":SCHEMA, "id":rid, "created_utc":utc(), "timezone":"Europe/Lisbon",
                  "metadata":metadata, "prepared_declared":True, "context":self.latest["context"],
                  "session_file":self.latest["session_file"], "software":software_evidence(), "captures":[], "attempts":[]}
        self.append({"kind":"begin", "run":{k:v for k,v in self.run.items() if k not in ("captures","attempts")}})
        self.persist()
        self.error=None

    def begin_guided(self, data):
        if data.get("prepared") is not True:
            raise ValueError("Confirma las ruedas rectas, el kart parado y la tracción inhabilitada")
        self.begin({"prepared":True,
                    "configuration":data.get("configuration") or "Montaje actual; identificación no indicada",
                    "alignment":"Usuario confirma ruedas rectas; método físico no indicado",
                    "conditions":data.get("conditions") or "Kart parado y tracción inhabilitada declarados; otras condiciones no indicadas"})
        self.active=None
        self.guide={"state":"running","phase":"positioning","message":"Coloca las ruedas rectas y pulsa Medir cuando estés listo.",
                    "progress":0,"from_raw":None,"moved":True,"policy":GUIDED_POLICY.copy(),"detail":"","issue":None}
        self.run["mode"]="guided"
        self.run["acquisition_policy"]=GUIDED_POLICY.copy()
        self.persist()

    def measure_guided(self):
        if not self.guide or self.guide["state"]!="running" or self.guide["phase"]!="positioning":
            raise ValueError("Coloca la dirección antes de iniciar la medición")
        self.start_capture()
        self.guide.update(phase="holding",progress=0,detail="",issue=None,
                          message="Mantén la posición hasta que termine la cuenta atrás.")

    def resume_guided(self):
        if not self.guide or self.guide["state"]!="paused":
            raise ValueError("No hay un paso pendiente de reintentar")
        if self.problem():
            raise ValueError(self.problem())
        if self.run["context"]!=self.latest["context"]:
            raise ValueError("Fuente/sesión distinta: prepara un nuevo ensayo")
        self.guide.update(state="running",phase="positioning",progress=0,detail="",issue=None,
                          message="Coloca de nuevo la dirección y pulsa Medir cuando estés listo.")

    def pause_step(self, issue, message):
        self.finish("rejected",message)
        self.guide.update(state="paused",progress=0,issue=issue,detail=message,message=message)

    def cancel_guided(self):
        if self.guide:
            self.guide.update(state="cancelled",progress=0,message="Calibración cancelada. Las lecturas quedan guardadas.")
        self.finish("rejected","Calibración guiada cancelada por el usuario")

    def advance_guided(self, row):
        cap=self.capture
        g=self.guide
        raw=row["frame"]["signals"]["steer_raw"]
        if raw in (0,4095):
            self.pause_step("limit", "La señal CAN ha llegado a 0 o 4095. No se puede distinguir más recorrido. Revisa la señal antes de repetir este paso.")
            return
        captures=self.run["captures"]
        center=captures[0]["statistics"]["median"] if captures else None
        noise=captures[0]["statistics"]["peak_to_peak"] if captures else 0
        move=max(GUIDED_POLICY["minimum_move"],3*noise)
        if g["from_raw"] is not None and abs(raw-g["from_raw"])>=move:
            g["moved"]=True
        if not g["moved"]:
            self.pause_step("position", "La lectura sigue en la posición anterior. Coloca la dirección en la nueva posición y repite este paso.")
            return
        # Repeticiones: impedir aceptar el centro en un extremo, un lado equivocado
        # o una pausa a mitad del retorno. El usuario sigue identificando el tope físico.
        left=next((x["statistics"]["median"] for x in captures if x["kind"]=="left"),None)
        right=next((x["statistics"]["median"] for x in captures if x["kind"]=="right"),None)
        target=center if cap["kind"].startswith("center") else left if cap["kind"]=="left" else right
        span=min([abs(v-center) for v in (left,right) if v is not None]+[4095]) if center is not None else 4095
        tolerance=max(30,2*noise,span*.15)
        wrong=(target is not None and abs(raw-target)>tolerance)
        if cap["kind"]=="left" and left is None:
            wrong=abs(raw-center)<move
        if cap["kind"]=="right" and right is None and left is not None:
            wrong=(raw-center)*(center-left)<=0 or abs(raw-center)<move
        if wrong:
            self.pause_step("position", "La lectura no coincide con la posición pedida. Revisa el lado o alinea las ruedas y repite este paso.")
            return
        # Ventana explícita desde el clic del usuario: no mezcla el desplazamiento
        # con la medición ni espera indefinidamente una ventana perfecta.
        s=stats(cap["rows"])
        values=[r["frame"]["signals"]["steer_raw"] for r in cap["rows"]]
        quarter=max(1,len(values)//4)
        drift=abs(statistics.median(values[:quarter])-statistics.median(values[-quarter:]))
        stable=s["peak_to_peak"]<=GUIDED_POLICY["stable_peak_cap"] and drift<=max(4,s["peak_to_peak"]*.30)
        g.update(phase="holding",progress=min(1,s["duration_s"]/cap["duration_s"],len(values)/cap["min_samples"]),
                 stable=stable,peak_to_peak=s["peak_to_peak"],
                 message="Mantén la posición hasta que termine la cuenta atrás.")
        if self.clock()-cap["start"]>cap["duration_s"]+5 and len(values)<cap["min_samples"]:
            self.pause_step("samples", "Llegan muy pocas muestras. Comprueba la conexión Wi-Fi y repite este paso.")
            return
        if s["duration_s"]<cap["duration_s"] or len(values)<cap["min_samples"]:
            return
        if not stable:
            self.pause_step("noise", "La señal varía demasiado durante la medición. Si no moviste el volante, revisa el sensor o el cableado. Puedes repetir este paso.")
            return
        accepted=s["median"]
        self.finish("completed")
        if len(self.run["captures"])==len(PLAN):
            try:
                self.guided_review()
                g.update(state="complete",progress=1,message="Las tres rondas son coherentes. Ya puedes usar esta calibración.")
            except ValueError as exc:
                g.update(state="failed",issue="consistency",progress=0,message="Las repeticiones no coinciden lo suficiente. Repite la calibración o revisa el sensor.",detail=str(exc))
            return
        g.update(from_raw=accepted,moved=False,phase="positioning",progress=0,detail="",issue=None,
                 message="Posición guardada. Coloca la siguiente y pulsa Medir cuando estés listo.")

    def guided_review(self):
        captures=self.run["captures"]
        ref=references(captures,"median_of_centers")
        span=min(ref["left_span"],ref["right_span"])
        criteria={"max_peak_to_peak":span*GUIDED_POLICY["noise_fraction"],
                  "max_return_error":span*GUIDED_POLICY["return_fraction"],
                  "min_span":GUIDED_POLICY["minimum_span"],"center_method":"median_of_centers",
                  "origin":"automatic_consistency_pc_v1"}
        self.review(captures,criteria,None)
        repeat=max(max(x["statistics"]["median"] for x in captures if x["kind"]==key)-min(x["statistics"]["median"] for x in captures if x["kind"]==key) for key in ("left","right"))
        if repeat>span*GUIDED_POLICY["repeat_fraction"]:
            raise ValueError("Los extremos no se repiten dentro del criterio de consistencia")
        return criteria

    def use_guided(self, data):
        if not self.guide or self.guide["state"]!="complete":
            raise ValueError("Completa las tres rondas de comprobación")
        if self.problem() or self.run["context"]!=self.latest["context"]:
            raise ValueError("La conexión o la sesión ha cambiado. Vuelve a comprobarla")
        pid=self.save({"name":data.get("name") or "Dirección · "+utc()[:16].replace("T"," ")+" UTC",
                       "reviewed":True,"criteria":self.guided_review(),"angles":data.get("angles")})
        self.activate({"id":pid,"compatible":True})
        self.guide.update(state="saved",profile_id=pid,message="Calibración guardada y activa en el panel.")
        return pid

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
        ref=references(captures,criteria.get("center_method","initial"))
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
                "same_context":bool(self.run and self.latest and self.run["context"]==self.latest["context"]),
                "panel_version":PANEL_VERSION,
                "guide":self.guide,
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
