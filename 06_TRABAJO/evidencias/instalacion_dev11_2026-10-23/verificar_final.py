from pathlib import Path
import csv,json,hashlib
import preparar
E=Path(__file__).resolve().parent
result={}
for base,data in preparar.old.ihex(E/'dev12/binarios/stm32_frontal-v1.3-dev12-log.hex').items():
    p=E/'dev12/relectura'/f'region_{base:08x}.bin'
    assert p.read_bytes()==data,hex(base)
    result[hex(base)]={'bytes':len(data),'diferencias':0,'sha256':preparar.old.sha(p)}
preparar.save(E/'dev12/relectura/verificacion.json',result)
rows=list(csv.DictReader((E/'dev12/continuidad/muestras.csv').open(encoding='utf-8')))
result={'tipo':'medicion SWD, no lector microSD','muestras':len(rows),'duracion_host_s':float(rows[-1]['t_s']),
        'sesiones':sorted({r['sd_session'] for r in rows}),
        'estados':{s:sum(r['sd_state']==s for r in rows) for s in sorted({r['sd_state'] for r in rows})},
        'maximos':{k:max(int(r[k]) for r in rows) for k in ['sd_dropped','sd_error','sd_max_call_ms','can_tx_errors','can_rx_errors','adc_errors','sd_queued']},
        'cierre':{k:rows[-1][k] for k in ['sd_session','sd_rows','sd_state','sd_elapsed_ms','sd_last_save','sd_queued']}}
preparar.save(E/'dev12/resumen_continuidad.json',result)
files={p.relative_to(E).as_posix():{'bytes':p.stat().st_size,'sha256':preparar.old.sha(p)} for p in E.rglob('*') if p.is_file() and p.name!='manifest.json' and '__pycache__' not in p.parts and 'testtmp' not in p.parts}
(E/'manifest.json').write_text(json.dumps(files,indent=2),encoding='utf-8')
