import json, statistics, sys
d = json.load(open(sys.argv[1], encoding="utf-8"))
usb, tcp = d["usb"], d["tcp"]
sesiones = {f["session"] for _, f in usb} | {f["session"] for *_, f in tcp}
summary = {"sessions": sorted(sesiones), "firmwares": sorted({f.get("fw", "sin-version") for _, f in usb}),
           "usb_frames": len(usb), "tcp_frames": len(tcp), "invalid_json": d.get("invalidas", {}),
           "usb_sequence_gaps": sum(b[1]["seq"] - a[1]["seq"] - 1 for a,b in zip(usb,usb[1:])),
           "phases": {}, "control_states": sorted({f["signals"].get("control_state", "no-publicado") for _,f in usb}),
           "all_control_not_ready": all(f["signals"].get("control_ready")==0 for _,f in usb),
           "all_relays_off": all(f["signals"].get("relay1")==0 and f["signals"].get("relay2")==0 for _,f in usb),
           "no_motor_measurements_published": all("rpm_rl" not in f["signals"] and "pwm_left_raw" not in f["signals"] for _,f in usb),
           "telemetry_us_max": max(f["signals"].get("telemetry_us_max",0) for _,f in usb),
           "sample_tick_deltas_ms":sorted({b[1]["tick_ms"]-a[1]["tick_ms"] for a,b in zip(usb,usb[1:])}),
           "telemetry_dropped_max":max(f["signals"].get("telemetry_dropped",0) for _,f in usb)}
print("sesiones vistas (1 = sin reinicios):", sesiones)
seq_usb = {f["seq"]: t for t, f in usb}
print(f"USB: {len(usb)} tramas, seq {usb[0][1]['seq']}..{usb[-1][1]['seq']}, saltos {sum(b[1]['seq']-a[1]['seq']-1 for a, b in zip(usb, usb[1:]))}")
for fase in sorted({x[0] for x in tcp}):
    fr = [(t, f) for fa, t, f in tcp if fa == fase]
    seqs = [f["seq"] for _, f in fr]
    summary["phases"][fase] = {"frames":len(fr), "sequence_gaps":sum(b-a-1 for a,b in zip(seqs,seqs[1:]))}
    lat = [t - seq_usb[f["seq"]] for t, f in fr if f["seq"] in seq_usb]
    print(f"TCP {fase}: {len(fr)} tramas, seq {seqs[0]}..{seqs[-1]}, saltos {sum(b-a-1 for a, b in zip(seqs, seqs[1:]))}, "
          f"llegada TCP - USB: mediana {statistics.median(lat)*1000:.0f} ms, máx {max(lat)*1000:.0f} ms")
def resumen(nombre, cond):
    sel = [f["signals"] for t, f in usb if cond(t)]
    if not sel:
        return
    lm = [s["loop_max_us"] for s in sel if s.get("loop_max_us")]
    tiempos = f"mediana {statistics.median(lm):6.0f} máx {max(lm):6d}" if lm else "no disponible (control aún no inicializado)"
    print(f"{nombre:28s} n={len(sel):3d} loop_max_us {tiempos} | "
          f"tele_us máx {max(s['telemetry_us_max'] for s in sel):4d} | clientes {sorted({s['wifi_clients'] for s in sel})} | "
          f"descartadas {sel[-1]['telemetry_dropped']}")
ev = {e: t for t, e in d["eventos"]}
tA = next(t for t, e in d["eventos"] if e.startswith("A_30s: TCP conectado"))
tCorte = next(t for t, e in d["eventos"] if "cortado" in e)
resumen("antes de conectar Wi-Fi", lambda t: t < tA)
resumen("con cliente TCP (fase A)", lambda t: tA + 1 < t < tA + 30)
resumen("portátil sin Wi-Fi (corte)", lambda t: tCorte < t < tCorte + 10)
resumen("todo el ensayo", lambda t: True)
print("JSON inválidos:", summary["invalid_json"], "| estados:", summary["control_states"])
if len(sys.argv)>2:
    with open(sys.argv[2],"w",encoding="utf-8") as output:
        json.dump(summary,output,indent=2,ensure_ascii=False)
