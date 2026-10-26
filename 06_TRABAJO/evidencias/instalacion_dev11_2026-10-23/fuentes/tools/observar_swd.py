"""Observa el frontal F746 por SWD sin detener la CPU (ENS-0019).

Lee los registros LTDC de la capa 1, vuelca el framebuffer que se está
mostrando y lo convierte a PNG. Con --elf decodifica además el estado del
frontal v1.3 (KartUiSnapshot `ui`, CanData `received` y avisos) a partir de las
direcciones del propio ELF. Solo lectura: no hay halt, reset ni escritura.

La imagen es el contenido del framebuffer que alimenta al LCD, no una foto del
panel: no acredita brillo, color real, franja blanca ni táctil.
"""
import argparse
import json
import re
import struct
import subprocess
import sys
import time
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[4]
OPENOCD = RAIZ / "07_DESARROLLO/herramientas/openocd/xpack-openocd-0.12.0-7"
NM = RAIZ / "07_DESARROLLO/herramientas/toolchains/bin/arm-none-eabi-nm.exe"
LTDC_L1 = 0x40016884
SIMBOLOS = ["ui", "received", "notice", "notice_tick", "selected_mode", "selected_gear",
            "initialized", "watchdog_ready", "sd_state", "sd_error", "sd_open", "uwTick"]
UI_CAMPOS = ("tick can_age status_age bms_soc_age bms_temp_age can_tx_errors can_rx_errors adc_errors sd_rows "
             "accel_adc steer_adc accel_raw steer_raw speed_kph rpm_left rpm_right yaw_deci_dps rpm_front_left "
             "rpm_front_right soc_pct battery_temp_c input_flags brake start gear mode_requested mode_received "
             "mode_applied inhibit backend_flags feedback_fresh status_fresh status_seen soc_valid temp_valid "
             "sd_requested sd_state sd_error ui_notice").split()
UI_FORMATO = "<9I4H6hHh18B8s8s2x"  # 96 B, disposición comprobada con `ptype /o KartUiSnapshot`
UI_EXTRA_CAMPOS = ("front_left_age front_right_age drive_input reverse_input start_input pedal_free "
                   "stationary feedback_usable front_left_valid front_right_valid relay_state backend_gear").split()


def decodificar_ui(raw):
    # dev4/dev5: 96 B; dev6-ui añade diagnóstico sin desplazar los campos previos.
    if len(raw) not in (96, 116, 212):
        raise ValueError(f"KartUiSnapshot de {len(raw)} B no reconocido; comprobar ELF y disposición")
    values = struct.unpack(UI_FORMATO, raw[:96])
    result = dict(zip(UI_CAMPOS, values[:len(UI_CAMPOS)]))
    result["bms355"], result["bms356"] = values[-2].hex(), values[-1].hex()
    if len(raw) >= 116:
        result.update(zip(UI_EXTRA_CAMPOS, struct.unpack_from("<2I10B2x", raw, 96)))
    if len(raw) == 212:
        result.update(zip("sd_session sd_dropped sd_elapsed_ms sd_free_kib sd_last_save sd_max_call_ms sd_queued log_age".split(),struct.unpack_from("<8I",raw,116)))
        result["log_words"]=list(struct.unpack_from("<16I",raw,148))
    return result


def simbolos(elf):
    salida = subprocess.run([str(NM), "-S", str(elf)], capture_output=True, text=True, check=True).stdout
    tabla = {}
    for linea in salida.splitlines():
        partes = linea.split()
        if len(partes) == 4 and partes[3] in SIMBOLOS:
            tabla[partes[3]] = (int(partes[0], 16), int(partes[1], 16))
    return tabla


def guion(serie, destino, tabla):
    d = destino.as_posix()
    lineas = [
        "source [find interface/stlink.cfg]", "transport select swd", f"adapter serial {serie}",
        "set QUADSPI 1", "source [find target/stm32f7x.cfg]", "adapter speed 1800",
        "gdb port disabled", "tcl port disabled", "telnet port disabled",
        "stm32f7x.cpu configure -event examine-end {}", "init",
        "proc w {a} { return [format 0x%08X [lindex [read_memory $a 32 1] 0]] }",
        "if {([lindex [read_memory 0xE0042000 32 1] 0] & 0xFFF) != 0x449} {error \"No es STM32F74x\"}",
    ]
    for nombre, off in [("L1CR", 0x00), ("L1WHPCR", 0x04), ("L1WVPCR", 0x08), ("L1PFCR", 0x10),
                        ("L1CFBAR", 0x28), ("L1CFBLR", 0x2C), ("L1CFBLNR", 0x30)]:
        lineas.append(f"echo \"{nombre}=[w {LTDC_L1 + off:#x}]\"")
    lineas += ["echo \"LTDC_GCR=[w 0x40016818]\"",
               "set fb [lindex [read_memory 0x400168AC 32 1] 0]",
               "set pitch [expr {([lindex [read_memory 0x400168B0 32 1] 0] >> 16) & 0x1FFF}]",
               "set lineas [expr {[lindex [read_memory 0x400168B4 32 1] 0] & 0x7FF}]",
               f"dump_image {{{d}/framebuffer.raw}} $fb [expr {{$pitch * $lineas}}]"]
    for nombre, (dir_, tam) in tabla.items():
        lineas.append(f"dump_image {{{d}/sym_{nombre}.bin}} {dir_:#x} {tam}")
    if "uwTick" in tabla:
        lineas += [f"echo \"UWTICK_A=[w {tabla['uwTick'][0]:#x}]\"", "sleep 500",
                   f"echo \"UWTICK_B=[w {tabla['uwTick'][0]:#x}]\""]
    lineas += ["echo \"FIN\"", "shutdown"]
    return "\n".join(lineas) + "\n"


def a_png(raw, pfcr, pitch, lineas, ancho, png):
    from PIL import Image
    if pfcr != 2:
        raise SystemExit(f"Formato LTDC {pfcr} no soportado (se espera RGB565=2)")
    img = Image.new("RGB", (ancho, lineas))
    px = img.load()
    for y in range(lineas):
        fila = raw[y * pitch:y * pitch + ancho * 2]
        for x, (v,) in enumerate(struct.iter_unpack("<H", fila)):
            px[x, y] = (((v >> 11) & 31) * 255 // 31, ((v >> 5) & 63) * 255 // 63, (v & 31) * 255 // 31)
    img.save(png)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--serie", required=True, help="número de serie del ST-LINK")
    ap.add_argument("--salida", required=True, type=Path)
    ap.add_argument("--elf", type=Path, help="ELF v1.3 grabado, para decodificar el estado")
    a = ap.parse_args()
    a.salida.mkdir(parents=True, exist_ok=True)
    tabla = simbolos(a.elf) if a.elf else {}
    cfg = a.salida / "observar.cfg"
    cfg.write_text(guion(a.serie, a.salida.resolve(), tabla), encoding="ascii")
    inicio = time.strftime("%Y-%m-%dT%H:%M:%S")
    r = subprocess.run([str(OPENOCD / "bin/openocd.exe"), "-s", str(OPENOCD / "openocd/scripts"), "-f", str(cfg)],
                       capture_output=True, text=True)
    (a.salida / "openocd.log").write_text(r.stdout + r.stderr, encoding="utf-8")
    valores = dict(re.findall(r"^(\w+)=(\S+)$", r.stdout + r.stderr, re.M))
    if "FIN" not in r.stdout + r.stderr:
        print(r.stdout + r.stderr)
        raise SystemExit("OpenOCD no terminó la observación")
    reg = {k: int(v, 16) for k, v in valores.items() if v.startswith("0x")}
    pitch = (reg["L1CFBLR"] >> 16) & 0x1FFF
    lineas = reg["L1CFBLNR"] & 0x7FF
    ancho = ((reg["L1CFBLR"] & 0x1FFF) - 3) // 2
    a_png((a.salida / "framebuffer.raw").read_bytes(), reg["L1PFCR"] & 7, pitch, lineas, ancho,
          a.salida / "pantalla.png")
    res = {"inicio_local": inicio, "registros": {k: f"{v:#010x}" for k, v in reg.items()},
           "framebuffer": {"direccion": f"{reg['L1CFBAR']:#010x}", "ancho": ancho, "lineas": lineas, "pitch": pitch}}
    if tabla:
        datos = {n: (a.salida / f"sym_{n}.bin").read_bytes() for n in tabla}
        if "ui" in datos:
            res["ui"] = decodificar_ui(datos["ui"])
        if "received" in datos:
            rx = struct.unpack("<5I4B8s8s8s4h", datos["received"])
            res["received"] = dict(zip("feedback_tick status_tick soc_tick temp_tick rx_errors feedback_seen "
                                       "status_seen soc_seen temp_seen".split(), rx[:9]))
            res["received"]["status"] = rx[9].hex()
        for n in ["notice", "selected_mode", "selected_gear", "initialized", "watchdog_ready",
                  "sd_state", "sd_error", "sd_open"]:
            if n in datos:
                res[n] = datos[n][0]
        if "notice_tick" in datos:
            res["notice_tick"] = struct.unpack("<I", datos["notice_tick"])[0]
        if "UWTICK_A" in reg:
            res["uwTick_500ms"] = [reg["UWTICK_A"], reg["UWTICK_B"], reg["UWTICK_B"] - reg["UWTICK_A"]]
    (a.salida / "observacion.json").write_text(json.dumps(res, indent=1, ensure_ascii=False), encoding="utf-8")
    print(json.dumps(res, indent=1, ensure_ascii=False))


if __name__ == "__main__":
    sys.exit(main())
