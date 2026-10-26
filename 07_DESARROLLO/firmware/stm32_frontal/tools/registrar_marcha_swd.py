"""Registro F746 de solo lectura: identidad, snapshot completo y condiciones D/R.

--pedal exige START OFF y selector/marcha N durante toda la captura; si cambian,
se cancela la adquisición (no puede detener el kart). La etiqueta describe la
instrucción del operador, no demuestra por sí misma la posición física del pie.
"""
import argparse
import csv
import datetime
import hashlib
import json
import struct
import subprocess
from pathlib import Path

import observar_swd as observer
import registrar_pedal_swd as pedal


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--serie', required=True)
    ap.add_argument('--elf', type=Path, required=True)
    ap.add_argument('--salida', type=Path, required=True)
    ap.add_argument('--etiqueta', required=True)
    ap.add_argument('--segundos', type=int, default=30)
    ap.add_argument('--pedal', action='store_true')
    a = ap.parse_args()
    if not 1 <= a.segundos <= 60:
        ap.error('duración permitida: 1 a 60 s')
    a.salida.mkdir(parents=True, exist_ok=False)
    address, size = pedal.simbolo(a.elf, 'ui')
    if size not in (116,212):
        raise SystemExit('Se exige snapshot diagnóstico de 116 o 212 B')
    vectors, version = pedal.identidad(a.elf)
    stop = a.salida.resolve()/'PARAR'
    cfg = a.salida/'registrar.cfg'
    cfg.write_text(pedal.guion(a.serie, address, size//4, 50, a.segundos*1000,
                               vectors, version, stop), encoding='ascii')
    meta = {'inicio_utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
            'etiqueta_operador': a.etiqueta, 'solo_lectura': True,
            'elf_sha256': hashlib.sha256(a.elf.read_bytes()).hexdigest(),
            'version_esperada': version[1].decode(), 'serie_stlink': a.serie,
            'exige_off_neutro': a.pedal, 'duracion_plan_s': a.segundos}
    rows, identity, first_ms, last_report = [], {}, None, -5000
    proc = subprocess.Popen([str(observer.OPENOCD/'bin/openocd.exe'), '-s',
                             str(observer.OPENOCD/'openocd/scripts'), '-f', str(cfg)],
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            text=True, encoding='utf-8', errors='replace')
    try:
        with (a.salida/'openocd.log').open('w', encoding='utf-8') as log:
            for line in proc.stdout:
                log.write(line)
                if line.startswith(('VEC ', 'VER ')):
                    tag = line[:3]
                    expected = vectors[1] if tag == 'VEC' else version[1]
                    identity[tag] = bytes(int(x, 16) for x in line.split()[1:]) == expected
                    if not identity[tag]:
                        stop.write_text('identidad no coincide')
                        meta['abortado'] = 'Identidad no coincide'
                if not line.startswith('M ') or identity != {'VEC': True, 'VER': True}:
                    continue
                parts = line.split()
                ms = int(parts[1])
                if first_ms is None: first_ms = ms
                sample = observer.decodificar_ui(b''.join(struct.pack('<I', int(x, 16)) for x in parts[2:]))
                sample.update(host_ms=ms, t_s=(ms-first_ms)/1000, etiqueta=a.etiqueta)
                rows.append(sample)
                if a.pedal and (sample['start'] or sample['gear'] or sample['drive_input'] or sample['reverse_input']):
                    meta['abortado'] = 'Se exige START OFF y selector/marcha N; no tocar pedal'
                    stop.write_text(meta['abortado'])
                if ms-first_ms-last_report >= 5000:
                    last_report = ms-first_ms
                    print(f"t={sample['t_s']:.1f}s ADC={sample['accel_adc']} libre={sample['pedal_free']} "
                          f"freno={sample['brake']} START={sample['start']} selector={sample['drive_input']}/{sample['reverse_input']} "
                          f"marcha={sample['gear']} CAN={sample['feedback_usable']}", flush=True)
            proc.wait(timeout=5)
    finally:
        if proc.poll() is None:
            stop.write_text('fin del registrador')
            try: proc.wait(timeout=5)
            except subprocess.TimeoutExpired: proc.kill(); proc.wait()
    if rows:
        with (a.salida/'muestras.csv').open('w', newline='', encoding='utf-8') as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
    summary = {}
    for name in ['accel_adc', 'accel_raw', 'steer_adc', 'can_age', 'can_tx_errors', 'can_rx_errors', 'adc_errors'] + (['sd_rows','sd_dropped','sd_max_call_ms','sd_queued','sd_session','sd_state'] if size==212 else []):
        if rows: summary[name] = pedal.estadistica([r[name] for r in rows])
    for name in ['pedal_free', 'brake', 'start', 'gear', 'drive_input', 'reverse_input', 'feedback_usable', 'stationary', 'mode_requested', 'input_flags']:
        summary[name] = {str(v): sum(r[name] == v for r in rows) for v in sorted(set(r[name] for r in rows))}
    meta.update(identidad=identity, muestras=len(rows), salida_openocd=proc.returncode,
                fin_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(), resumen=summary)
    (a.salida/'meta.json').write_text(json.dumps(meta, indent=2, ensure_ascii=False), encoding='utf-8')
    print(json.dumps(meta, ensure_ascii=False, indent=2))
    return int(bool(meta.get('abortado')) or not rows or proc.returncode != 0)


if __name__ == '__main__':
    raise SystemExit(main())
