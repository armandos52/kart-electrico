"""Registra el pedal acelerador del frontal F746 por SWD sin detener la CPU (ENS-0020, PROB-0012).

Lee a ~20 Hz el snapshot `ui` (KartUiSnapshot) que el firmware v1.3 publica cada 5 ms:
`accel_adc` (cuenta física ADC3 canal 0) y `accel_raw` (normalizada 0-4095 con ACCEL_MIN/MAX).
La dirección sale de `arm-none-eabi-nm` y los desplazamientos de `gdb ptype /o KartUiSnapshot`
sobre el mismo ELF grabado. Antes de registrar comprueba IDCODE, tabla de vectores y cadena de
versión en flash. Solo lectura: no hay halt, reset ni escritura.

Guía al operador con fases cronometradas (texto y pitidos) y etiqueta cada muestra con la fase
planificada y si en ella se toca el pedal. Guarda muestras.csv, fases.csv, meta.json,
openocd.log y el resumen estadístico (resumen.json y resumen.md). Con --solo-resumen recalcula
el resumen de una carpeta ya registrada.
"""
import argparse
import csv
import hashlib
import json
import math
import re
import struct
import subprocess
import sys
import threading
import time
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[4]
OPENOCD = RAIZ / "07_DESARROLLO/herramientas/openocd/xpack-openocd-0.12.0-7"
BIN = RAIZ / "07_DESARROLLO/herramientas/toolchains/bin"
CAMPOS = ["tick", "adc_errors", "accel_adc", "steer_adc", "accel_raw", "steer_raw",
          "input_flags", "brake", "start", "gear", "ui_notice"]
# Valores de Core/Src/kart_frontend.c en dev4; solo para comprobar coherencia y el criterio actual.
ACCEL_MIN, ACCEL_MAX, ADC_MARGIN, PEDAL_LIBRE = 124, 1861, 100, 40
GUARDA_S = 1.0  # se excluye el primer segundo de cada fase (reacción del operador y transición)

# (nombre, segundos, toca_pedal, instrucción). toca_pedal: 0 = pie fuera, 1 = apoyado sin pisar,
# 2 = pisado a fondo, 3 = recorrido lento.
FASES = {
    "completo": [("preparacion", 10, 0, "Prepárate. Pie FUERA del pedal")]
    + [("reposo_inicial", 30, 0, "REPOSO: pie FUERA del pedal, no lo toques")]
    + [("apoyado", 15, 1, "Apoya el pie sobre el pedal SIN pisar")]
    + [("reposo_tras_apoyo", 10, 0, "Retira el pie: REPOSO sin tocar")]
    + [f for k in range(1, 6) for f in (
        (f"fondo_{k}", 5, 2, f"PISA A FONDO y mantén ({k}/5)"),
        (f"suelta_{k}", 10, 0, f"SUELTA y retira el pie ({k}/5)"))]
    + [f for k in range(1, 3) for f in (
        (f"lento_{k}", 12, 3, f"Pisa DESPACIO hasta el fondo (~6 s) y suelta DESPACIO (~6 s) ({k}/2)"),
        (f"reposo_lento_{k}", 8, 0, "SUELTA y retira el pie"))]
    + [("reposo_final", 30, 0, "REPOSO final: pie FUERA del pedal")],
    "prueba": [("prueba_reposo", 8, 0, "Prueba técnica: no hace falta tocar nada")],
}
TOCA = {0: "no", 1: "apoyado_sin_pisar", 2: "a_fondo", 3: "recorrido_lento"}


def herramienta(nombre):
    return str(BIN / f"arm-none-eabi-{nombre}.exe")


def simbolo(elf, nombre):
    salida = subprocess.run([herramienta("nm"), "-S", str(elf)], capture_output=True, text=True, check=True).stdout
    for linea in salida.splitlines():
        p = linea.split()
        if len(p) == 4 and p[3] == nombre:
            return int(p[0], 16), int(p[1], 16)
    raise SystemExit(f"Símbolo {nombre} no encontrado en {elf}")


def desplazamientos(elf):
    r = subprocess.run([herramienta("gdb"), "-batch", "-nx", "-ex", "ptype /o KartUiSnapshot", str(elf)],
                       capture_output=True, text=True)
    texto = r.stdout
    off = {}
    for m in re.finditer(r"/\*\s+(\d+)\s+\|\s+(\d+)\s+\*/\s+(\w+)\s+(\w+)(?:\[\d+\])?;", texto):
        off[m.group(4)] = (int(m.group(1)), int(m.group(2)), m.group(3))
    total = re.search(r"total size \(bytes\):\s+(\d+)", texto)
    faltan = [c for c in CAMPOS if c not in off]
    if faltan or not total:
        raise SystemExit(f"ptype /o no dio los campos {faltan}:\n{texto}{r.stderr}")
    return off, int(total.group(1)), texto


def secciones(elf):
    salida = subprocess.run([herramienta("objdump"), "-h", str(elf)], capture_output=True, text=True, check=True).stdout
    tabla = {}
    for m in re.finditer(r"^\s*\d+\s+(\S+)\s+([0-9a-f]{8})\s+([0-9a-f]{8})\s+[0-9a-f]{8}\s+([0-9a-f]{8})", salida, re.M):
        tabla[m.group(1)] = (int(m.group(3), 16), int(m.group(2), 16), int(m.group(4), 16))  # vma, tamaño, offset
    return tabla


def identidad(elf):
    """Bytes del ELF que deben coincidir con la flash: tabla de vectores y cadena de versión."""
    datos = elf.read_bytes()
    sec = secciones(elf)
    vma, tam, off = sec[".isr_vector"]
    vectores = (vma, datos[off:off + tam])
    vma, tam, off = sec[".rodata"]
    rodata = datos[off:off + tam]
    m = re.search(rb"stm32_frontal-v[0-9][0-9A-Za-z.\-]*", rodata)
    if not m:
        raise SystemExit("Cadena de versión no encontrada en .rodata")
    version = (vma + m.start(), m.group(0))
    return vectores, version


def guion(serie, ui_dir, palabras, periodo_ms, duracion_ms, vectores, version, parada):
    lineas = [
        "# ENS-0020: registro del pedal por SWD. Solo lectura, sin halt/reset/escritura.",
        "source [find interface/stlink.cfg]", "transport select swd", f"adapter serial {serie}",
        "set QUADSPI 1", "source [find target/stm32f7x.cfg]", "adapter speed 1800",
        "gdb port disabled", "tcl port disabled", "telnet port disabled",
        "stm32f7x.cpu configure -event examine-end {}", "init",
        "if {([lindex [read_memory 0xE0042000 32 1] 0] & 0xFFF) != 0x449} {error \"No es STM32F74x\"}",
        f"echo \"VEC [read_memory {vectores[0]:#x} 8 {len(vectores[1])}]\"",
        f"echo \"VER [read_memory {version[0]:#x} 8 {len(version[1])}]\"",
        f"set siguiente [clock milliseconds]; set fin [expr {{$siguiente + {duracion_ms}}}]",
        f"while {{[clock milliseconds] < $fin && ![file exists {{{parada.as_posix()}}}]}} {{",
        f"\tset ms [clock milliseconds]; set m [read_memory {ui_dir:#x} 32 {palabras}]",
        "\techo \"M $ms $m\"",
        f"\tset siguiente [expr {{$siguiente + {periodo_ms}}}]; set espera [expr {{$siguiente - [clock milliseconds]}}]",
        "\tif {$espera > 0} {sleep $espera} else {set siguiente [clock milliseconds]}",
        "}",
        "echo \"FIN\"", "shutdown",
    ]
    return "\n".join(lineas) + "\n"


def pitar(tipo, sonido):
    if not sonido:
        return
    import winsound
    tonos = {"pisar": [(1300, 350), (1300, 350)], "reposo": [(600, 500)], "apoyar": [(900, 250)] * 3,
             "cuenta": [(800, 90)], "fin": [(1000, 150), (800, 150), (600, 400)]}

    def sonar():
        for f, d in tonos[tipo]:
            winsound.Beep(f, d)
            time.sleep(0.08)
    threading.Thread(target=sonar, daemon=True).start()


def tipo_pitido(toca):
    return {0: "reposo", 1: "apoyar", 2: "pisar", 3: "pisar"}[toca]


def normalizar(adc):
    if adc < ACCEL_MIN - ADC_MARGIN or adc > ACCEL_MAX + ADC_MARGIN:
        return 0
    b = min(max(adc, ACCEL_MIN), ACCEL_MAX)
    return (b - ACCEL_MIN) * 4095 // (ACCEL_MAX - ACCEL_MIN)


def decodificar(palabras, off):
    crudo = b"".join(struct.pack("<I", int(p, 16)) for p in palabras)
    fmt = {1: "B", 2: "H", 4: "I"}
    fila = {}
    for c in CAMPOS:
        o, tam, _ = off[c]
        fila[c] = struct.unpack_from("<" + fmt[tam], crudo, o)[0]
    return fila


def registrar(a):
    elf = a.elf.resolve()
    ui_dir, ui_tam = simbolo(elf, "ui")
    off, total, ptype = desplazamientos(elf)
    if ui_tam != total or total % 4:
        raise SystemExit(f"Tamaño de ui {ui_tam} distinto de KartUiSnapshot {total}")
    vectores, version = identidad(elf)
    fases = FASES[a.guion]
    duracion_s = sum(f[1] for f in fases)
    a.salida.mkdir(parents=True, exist_ok=True)
    (a.salida / "ptype_KartUiSnapshot.txt").write_text(ptype, encoding="utf-8")
    parada = a.salida / "PARAR"
    parada.unlink(missing_ok=True)
    cfg = a.salida / "registrar.cfg"
    cfg.write_text(guion(a.serie, ui_dir, total // 4, round(1000 / a.hz), (duracion_s + 15) * 1000,
                         vectores, version, parada.resolve()), encoding="ascii")
    meta = {"herramienta": "registrar_pedal_swd.py", "guion": a.guion, "serie_stlink": a.serie,
            "elf": elf.relative_to(RAIZ).as_posix() if elf.is_relative_to(RAIZ) else str(elf),
            "elf_sha256": hashlib.sha256(elf.read_bytes()).hexdigest(),
            "ui": {"direccion": f"{ui_dir:#010x}", "tamano": ui_tam,
                   "desplazamientos": {c: off[c][0] for c in CAMPOS}},
            "version_esperada": version[1].decode(), "hz_objetivo": a.hz, "guarda_s": GUARDA_S,
            "firmware_criterio": {"ACCEL_MIN": ACCEL_MIN, "ACCEL_MAX": ACCEL_MAX, "ADC_MARGIN": ADC_MARGIN,
                                  "pedal_libre_normalizado_max": PEDAL_LIBRE},
            "inicio_local": time.strftime("%Y-%m-%dT%H:%M:%S")}
    print(f"ui en {ui_dir:#010x} ({ui_tam} B); accel_adc@{off['accel_adc'][0]} accel_raw@{off['accel_raw'][0]}")
    proc = subprocess.Popen([str(OPENOCD / "bin/openocd.exe"), "-s", str(OPENOCD / "openocd/scripts"), "-f", str(cfg)],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding="ascii",
                            errors="replace", bufsize=1)
    log, muestras, limites = [], [], []
    fase_i, t0, ultimo_estado, cuenta = -1, None, 0.0, set()
    identidad_ok = None
    try:
        for linea in proc.stdout:
            if not linea.startswith("M "):
                log.append(linea)
                if linea.startswith("VEC ") or linea.startswith("VER "):
                    leido = bytes(int(x, 16) for x in linea.split()[1:])
                    esperado = vectores[1] if linea.startswith("VEC ") else version[1]
                    ok = leido == esperado
                    meta["identidad_" + linea[:3].lower()] = ok
                    identidad_ok = ok if identidad_ok is None else identidad_ok and ok
                    print(f"{linea[:3]}: {'coincide con el ELF' if ok else 'NO COINCIDE'}")
                    if not ok:
                        parada.write_text("identidad")
                if linea.strip() == "FIN":
                    break
                continue
            p = linea.split()
            ms = int(p[1])
            if not identidad_ok:
                continue
            if t0 is None:
                t0 = ms
                limites.append((ms, 0))
                fase_i = 0
                print(f"\n>>> {fases[0][3].upper()} ({fases[0][1]} s)")
                pitar(tipo_pitido(fases[0][2]), a.sonido)
            rel = (ms - t0) / 1000
            fin_actual = sum(f[1] for f in fases[:fase_i + 1])
            while fase_i < len(fases) and rel >= fin_actual:
                fase_i += 1
                cuenta.clear()
                if fase_i == len(fases):
                    break
                limites.append((ms, fase_i))
                f = fases[fase_i]
                print(f"\n>>> {f[3].upper()} ({f[1]} s)")
                pitar(tipo_pitido(f[2]), a.sonido)
                fin_actual += f[1]
            if fase_i >= len(fases):
                parada.write_text("fin")
                break
            restante = fin_actual - rel
            if restante <= 3 and math.ceil(restante) not in cuenta and fase_i + 1 < len(fases):
                if not cuenta:
                    print(f"  ... en 3 s: {fases[fase_i + 1][3]}", flush=True)
                cuenta.add(math.ceil(restante))
                pitar("cuenta", a.sonido)
            fila = decodificar(p[2:], off)
            fase = fases[fase_i]
            inicio_fase = limites[-1][0]
            fila.update({"host_ms": ms, "t_s": round(rel, 3), "fase": fase[0], "toca_pedal": TOCA[fase[2]],
                         "t_en_fase_s": round((ms - inicio_fase) / 1000, 3),
                         "norm_recalculada": normalizar(fila["accel_adc"]),
                         "pedal_libre_dev4": int(fila["accel_raw"] <= PEDAL_LIBRE)})
            fila["coherente"] = int(fila["norm_recalculada"] == fila["accel_raw"])
            fila["tick_nuevo"] = int(not muestras or fila["tick"] != muestras[-1]["tick"])
            muestras.append(fila)
            if rel - ultimo_estado >= 1.0:
                ultimo_estado = rel
                libre = "LIBRE" if fila["pedal_libre_dev4"] else "pisado?"
                print(f"  {fase[0]:<18} quedan {restante:4.0f} s  ADC={fila['accel_adc']:5d}  "
                      f"norm={fila['accel_raw']:5d}  {libre}", flush=True)
    except KeyboardInterrupt:
        parada.write_text("interrumpido")
        meta["interrumpido"] = True
    finally:
        try:
            resto, _ = proc.communicate(timeout=20)
            log.append(resto or "")
        except subprocess.TimeoutExpired:
            proc.kill()
            log.append("\n[openocd terminado por tiempo]\n")
        parada.unlink(missing_ok=True)
    pitar("fin", a.sonido)
    (a.salida / "openocd.log").write_text("".join(log), encoding="utf-8")
    if identidad_ok is False:
        (a.salida / "meta.json").write_text(json.dumps(meta, indent=1, ensure_ascii=False), encoding="utf-8")
        raise SystemExit("La flash no coincide con el ELF indicado: no se registra")
    if not muestras:
        raise SystemExit("Sin muestras: revisar openocd.log")
    columnas = ["host_ms", "t_s", "fase", "toca_pedal", "t_en_fase_s"] + CAMPOS + \
        ["norm_recalculada", "coherente", "pedal_libre_dev4", "tick_nuevo"]
    with open(a.salida / "muestras.csv", "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=columnas)
        w.writeheader()
        w.writerows(muestras)
    with open(a.salida / "fases.csv", "w", newline="", encoding="utf-8") as fh:
        w = csv.writer(fh)
        w.writerow(["fase", "inicio_host_ms", "inicio_t_s", "duracion_plan_s", "toca_pedal", "instruccion"])
        for ms, i in limites:
            f = fases[i]
            w.writerow([f[0], ms, round((ms - t0) / 1000, 3), f[1], TOCA[f[2]], f[3]])
    meta.update({"fin_local": time.strftime("%Y-%m-%dT%H:%M:%S"), "muestras": len(muestras),
                 "duracion_s": round((muestras[-1]["host_ms"] - muestras[0]["host_ms"]) / 1000, 3),
                 "fases_completadas": len(limites), "fases_plan": len(fases)})
    meta["hz_medido"] = round((len(muestras) - 1) / meta["duracion_s"], 2) if meta["duracion_s"] else None
    (a.salida / "meta.json").write_text(json.dumps(meta, indent=1, ensure_ascii=False), encoding="utf-8")
    resumir(a.salida)


def percentil(ordenados, q):
    if len(ordenados) == 1:
        return float(ordenados[0])
    pos = (len(ordenados) - 1) * q / 100
    i = int(pos)
    j = min(i + 1, len(ordenados) - 1)
    return ordenados[i] + (ordenados[j] - ordenados[i]) * (pos - i)


def estadistica(valores):
    v = sorted(valores)
    n = len(v)
    media = sum(v) / n
    de = math.sqrt(sum((x - media) ** 2 for x in v) / (n - 1)) if n > 1 else 0.0
    res = {"n": n, "min": v[0], "max": v[-1], "media": round(media, 2), "desv": round(de, 2)}
    for q in (1, 5, 50, 95, 99):
        res[f"p{q}"] = round(percentil(v, q), 1)
    return res


def resumir(carpeta):
    with open(carpeta / "muestras.csv", encoding="utf-8") as fh:
        filas = list(csv.DictReader(fh))
    for f in filas:
        for c in CAMPOS + ["host_ms", "norm_recalculada", "coherente", "pedal_libre_dev4", "tick_nuevo"]:
            f[c] = int(f[c])
        f["t_en_fase_s"] = float(f["t_en_fase_s"])
    # 0xFFFF = conversión fallida. Fuera de plausibilidad (sin bit ACCEL_OK) se conserva: es dato físico.
    validas = [f for f in filas if f["accel_adc"] != 0xFFFF]
    orden = list(dict.fromkeys(f["fase"] for f in filas))
    por_fase = {}
    for nombre in orden:
        todas = [f for f in validas if f["fase"] == nombre]
        estables = [f for f in todas if f["t_en_fase_s"] >= GUARDA_S]
        if not estables:
            continue
        adc = [f["accel_adc"] for f in estables]
        por_fase[nombre] = {"toca_pedal": estables[0]["toca_pedal"], "adc": estadistica(adc),
                            "norm": estadistica([f["accel_raw"] for f in estables]),
                            "pedal_libre_dev4_pct": round(100 * sum(f["pedal_libre_dev4"] for f in estables) / len(estables), 1),
                            "adc_todas_min": min(f["accel_adc"] for f in todas),
                            "adc_todas_max": max(f["accel_adc"] for f in todas)}
    grupos = {"reposo_pie_fuera": [f for f in validas if f["toca_pedal"] == "no" and f["t_en_fase_s"] >= GUARDA_S
                                   and f["fase"] != "preparacion"],
              "apoyado_sin_pisar": [f for f in validas if f["toca_pedal"] == "apoyado_sin_pisar" and f["t_en_fase_s"] >= GUARDA_S],
              "a_fondo": [f for f in validas if f["toca_pedal"] == "a_fondo" and f["t_en_fase_s"] >= GUARDA_S],
              "recorrido_lento": [f for f in validas if f["toca_pedal"] == "recorrido_lento"]}
    agregados = {k: {"adc": estadistica([f["accel_adc"] for f in v]),
                     "pedal_libre_dev4_pct": round(100 * sum(f["pedal_libre_dev4"] for f in v) / len(v), 1)}
                 for k, v in grupos.items() if v}
    # Ruido: dispersión dentro de cada fase estable y diferencia entre muestras consecutivas.
    ruido = {}
    for k in ("reposo_pie_fuera", "a_fondo"):
        difs, desv = [], []
        for nombre, st in por_fase.items():
            if st["toca_pedal"] != {"reposo_pie_fuera": "no", "a_fondo": "a_fondo"}[k] or nombre == "preparacion":
                continue
            serie = [f["accel_adc"] for f in grupos[k] if f["fase"] == nombre]
            difs += [abs(b - c) for b, c in zip(serie[1:], serie[:-1])]
            desv.append(st["adc"]["desv"])
        if difs:
            ruido[k] = {"desv_intrafase_media": round(sum(desv) / len(desv), 2), "desv_intrafase_max": max(desv),
                        "dif_consecutiva": estadistica(difs)}
    total = len(filas)
    res = {"muestras": total, "validas": len(validas),
           "incoherentes": sum(1 - f["coherente"] for f in filas),
           "sin_accel_ok": sum(1 for f in filas if not f["input_flags"] & 1),
           "ticks_repetidos": sum(1 - f["tick_nuevo"] for f in filas),
           "adc_errors_inicio_fin": [filas[0]["adc_errors"], filas[-1]["adc_errors"]],
           "guarda_s": GUARDA_S, "por_fase": por_fase, "agregados": agregados, "ruido": ruido}
    (carpeta / "resumen.json").write_text(json.dumps(res, indent=1, ensure_ascii=False), encoding="utf-8")
    lineas = ["# Resumen automático del registro del pedal", "",
              f"Muestras {total}, válidas {len(validas)}, sin ACCEL_OK {res['sin_accel_ok']}, "
              f"incoherentes adc/norm {res['incoherentes']}, "
              f"ticks repetidos {res['ticks_repetidos']}, adc_errors {res['adc_errors_inicio_fin'][0]}→{res['adc_errors_inicio_fin'][1]}. "
              f"Se excluye el primer {GUARDA_S:g} s de cada fase, salvo en recorrido lento.", "",
              "| Grupo | n | mín | p1 | p5 | p50 | p95 | p99 | máx | media | desv | libre dev4 % |",
              "|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for k, v in agregados.items():
        s = v["adc"]
        lineas.append(f"| {k} | {s['n']} | {s['min']} | {s['p1']} | {s['p5']} | {s['p50']} | {s['p95']} | "
                      f"{s['p99']} | {s['max']} | {s['media']} | {s['desv']} | {v['pedal_libre_dev4_pct']} |")
    lineas += ["", "| Fase | toca | n | mín | p50 | máx | desv | libre dev4 % |", "|---|---|---|---|---|---|---|---|"]
    for nombre, st in por_fase.items():
        s = st["adc"]
        lineas.append(f"| {nombre} | {st['toca_pedal']} | {s['n']} | {s['min']} | {s['p50']} | {s['max']} | "
                      f"{s['desv']} | {st['pedal_libre_dev4_pct']} |")
    for k, r in ruido.items():
        d = r["dif_consecutiva"]
        lineas.append(f"\nRuido {k}: desv. intrafase media {r['desv_intrafase_media']} (máx {r['desv_intrafase_max']}); "
                      f"|Δ| consecutiva p50 {d['p50']}, p95 {d['p95']}, máx {d['max']} cuentas.")
    (carpeta / "resumen.md").write_text("\n".join(lineas) + "\n", encoding="utf-8")
    print("\n".join(lineas))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--serie", help="número de serie del ST-LINK")
    ap.add_argument("--elf", type=Path, help="ELF grabado en la F746")
    ap.add_argument("--salida", required=True, type=Path)
    ap.add_argument("--guion", choices=sorted(FASES), default="completo")
    ap.add_argument("--hz", type=float, default=20)
    ap.add_argument("--sin-sonido", dest="sonido", action="store_false")
    ap.add_argument("--solo-resumen", action="store_true", help="recalcular el resumen de una carpeta registrada")
    a = ap.parse_args()
    if a.solo_resumen:
        resumir(a.salida)
    elif not (a.serie and a.elf):
        ap.error("--serie y --elf son obligatorios para registrar")
    else:
        registrar(a)


if __name__ == "__main__":
    sys.exit(main())
