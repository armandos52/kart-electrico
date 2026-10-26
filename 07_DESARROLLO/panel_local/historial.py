"""Archivo local inmutable y reproducción de muestras. Nunca abre puertos del kart.

Los originales se conservan por SHA-256; SQLite indexa copias derivadas por sesión.
No se usa la hora de importación como hora de ensayo. Ver HISTORIAL.md.
"""
from __future__ import annotations
import csv
import hashlib
import io
import json
import math
from pathlib import Path, PurePosixPath
import re
import secrets
import sqlite3
import threading
import zipfile
from datetime import datetime, timezone

MAX_FILE = 32 * 1024 * 1024
MAX_BATCH = 1024 * 1024 * 1024
MAX_FILES = 256
EVENTS = {1:'START / pulsador',2:'Marcha / selector',4:'Modo',8:'Freno',16:'Entrada / ADC',32:'CAN / comunicación',64:'Bloqueo / estado trasero',128:'ESP32 / diagnóstico',256:'Inicio de captura'}
OLD_COLUMNS = 'Firmware,Tick_ms,AccelADC,SteerADC,AccelCAN,SteerCAN,InputFlags,Brake,Start,GearReq,ModeReq,ModeRecv,ModeApplied,Inhibit,BackendFlags,Vel_kmh,RpmFL,RpmFR,RpmRL,RpmRR,Yaw_deci_dps,CanAge_ms,StatusAge_ms,SOC_valid,SOC_pct,Temp_valid,Temp_C,BMS355Age_ms,BMS356Age_ms,BMS355_hex,BMS356_hex,CanTxErr,CanRxErr,AdcErr'.split(',')
NEW_COLUMNS = OLD_COLUMNS + 'Seq,Elapsed_ms,EventMask,Dropped,DriveInput,ReverseInput,StartInput,PedalFree,Stationary,FeedbackUsable,FrontLeftAge_ms,FrontRightAge_ms,FrontLeftValid,FrontRightValid,RelayState,BackendGear,LogAge_ms,LogWords_hex'.split(',')
LEGACY_COLUMNS = 'Tick,Accel,Steer,Brake,ModeD,ModeR,Vel,DiEsq,DiDir,TrEsq,TrDir,ModoDif,Yaw'.split(',')
INHIBITS = ['none','no_command','can_timeout','invalid_command','start_off','relays','charger','rearm_required','brake','pedal_zero','neutral','direction_change','mode_change','speed_invalid','hall_fault','mpu_unavailable','frontend_invalid','reverse_limit','output_fault','can_fault','configuration']

def stamp(): return datetime.now(timezone.utc).isoformat()
def dump(value): return json.dumps(value,ensure_ascii=False,allow_nan=False,separators=(',',':'))
def number(value, low=0, high=4294967295):
    n=float(value)
    if not math.isfinite(n) or not low<=n<=high: raise ValueError('valor fuera de representación')
    return int(n) if n.is_integer() else n

def field(value,label='',unit='',age=0,ttl=1500,valid=True,node='stm32'):
    return dict(value=value,label=label,unit=unit,age_ms=age,stale_after_ms=ttl,
                quality='fresh' if valid and age<=ttl else 'stale',node=node)

def csv_signals(row, schema):
    """Semántica versionada: bruto, mando y validez son campos distintos."""
    n=lambda key,lo=0,hi=4294967295:number(row[key],lo,hi)
    flags=n('InputFlags',0,7); ca=n('CanAge_ms'); sa=n('StatusAge_ms'); backend=n('BackendFlags',0,127)
    gear=int(n('GearReq',0,2));mode=int(n('ModeReq',0,2))
    signals={}
    def add(key,value,label,unit='',age=0,ttl=1500,valid=True,node='stm32'):
        signals[key]=field(value,label,unit,age,ttl,valid,node)
    add('accelerator_adc_physical',n('AccelADC',0,65535),'ADC físico pedal','cuentas',valid=bool(flags&1))
    add('steer_adc_physical',n('SteerADC',0,65535),'ADC físico dirección','cuentas',valid=bool(flags&2))
    add('accelerator_raw',n('AccelCAN',0,4095),'Mando pedal procesado','cuentas',valid=bool(flags&1))
    add('accelerator_pct',n('AccelCAN',0,4095)*100/4095,'Pedal procesado','%',valid=bool(flags&1))
    add('steer_raw',n('SteerCAN',0,4095),'Dirección normalizada','cuentas',valid=bool(flags&2))
    add('gear','NDR'[gear],'Marcha frontal');add('mode',['DIF','ESC','BLQ'][mode],'Modo solicitado')
    add('start_requested',n('Start',0,1),'START frontal');add('brake',n('Brake',0,1),'Freno leído')
    add('speed_kph',n('Vel_kmh',0,200),'Velocidad estimada','km/h',ca,500)
    for col,key,label,valid in [('RpmRL','rpm_rl','RPM trasera I',sa>300 or bool(backend&4)),('RpmRR','rpm_rr','RPM trasera D',sa>300 or bool(backend&4)),('Yaw_deci_dps','yaw_dps','Guiñada',sa<=300 and bool(backend&8))]:
        add(key,n(col,-32768,32767)/(10 if key=='yaw_dps' else 1),label,'°/s' if key=='yaw_dps' else 'rpm',ca,500,valid,'esp32')
    for col,key,label,side in [('RpmFL','rpm_fl','RPM delantera I','Left'),('RpmFR','rpm_fr','RPM delantera D','Right')]:
        age=n('Front'+side+'Age_ms') if schema==3 else 0
        add(key,n(col,-32768,32767),label,'rpm',age,500,bool(n('Front'+side+'Valid',0,1)) if schema==3 else False)
    for col,key,label in [('CanTxErr','can_tx_errors','Errores TX frontal'),('CanRxErr','frontend_can_rx_errors','Errores RX frontal'),('AdcErr','adc_errors','Errores ADC'),('CanAge_ms','feedback_age_ms','Edad feedback CAN'),('StatusAge_ms','status_age_ms','Edad estado trasero')]:
        add(key,n(col),label,'ms' if col.endswith('_ms') else '')
    add('mode_applied',n('ModeApplied',0,255),'Modo aplicado (255 desconocido)',age=sa,ttl=300,valid=n('ModeApplied')!=255,node='esp32')
    inhibit=int(n('Inhibit',0,255))
    if inhibit<len(INHIBITS):add('drive_inhibit',INHIBITS[inhibit],'Motivo trasero',age=sa,ttl=300,node='esp32')
    for col,key,label,agecol,rawcol,scale,low,high in [('SOC_pct','soc_pct','SOC BMS','BMS355Age_ms','BMS355_hex',100,0,10000),('Temp_C','battery_temp_c','Temperatura BMS','BMS356Age_ms','BMS356_hex',10,-500,1500)]:
        raw=row[rawcol]
        if not re.fullmatch('[0-9a-fA-F]{16}',raw):raise ValueError('trama BMS inválida')
        value=int.from_bytes(bytes.fromhex(raw)[4:6],'little',signed=key=='battery_temp_c')
        valid=bool(n('SOC_valid' if key=='soc_pct' else 'Temp_valid',0,1))
        add(key,n(col,-100,200) if valid else value/scale,label+('' if valid else ' · provisional'), '%' if key=='soc_pct' else '°C',n(agecol),2000,valid or low<=value<=high,'bms')
        add(rawcol,raw,'Bytes '+rawcol,age=n(agecol),ttl=2000,node='bms')
    if schema==3:
        for col,key,label in [('DriveInput','selector_d','Selector D'),('ReverseInput','selector_r','Selector R'),('StartInput','start_input','Pulsador START'),('PedalFree','pedal_free','Permiso pedal libre'),('Stationary','stationary','Quietud calculada'),('FeedbackUsable','feedback_usable','Feedback utilizable')]:
            add(key,n(col,0,1),label)
        add('sd_dropped',n('Dropped'),'Muestras descartadas SD')
        raw=row['LogWords_hex']
        if not re.fullmatch('[0-9a-fA-F]{128}',raw):raise ValueError('bloque diagnóstico incompleto')
        w=[int(raw[i:i+8],16) for i in range(0,128,8)];age=n('LogAge_ms')
        if age!=4294967295:
            def rear(key,val,label,unit=''):add(key,val,label,unit,age,1000,True,'esp32')
            rear('backend_boot',f'{w[0]:08x}','Arranque ESP32');rear('backend_build',f'{w[1]:08x}','Build ESP32 (FNV32)')
            rear('pwm_left_raw',w[3]&255,'PWM ordenado I','/255');rear('pwm_right_raw',(w[3]>>8)&255,'PWM ordenado D','/255')
            rear('relay1',(w[3]>>16)&1,'Relé 1 ordenado');rear('relay2',(w[3]>>17)&1,'Relé 2 ordenado')
            rear('relay_seq',(w[3]>>24)&255,'Secuencia relés');rear('charger_connected',(w[3]>>18)&1,'Cargador detectado')
            for idx,key,label,unit in [(5,'loop_max_us','Periodo máximo loop','µs'),(6,'can_rx_count','Órdenes CAN recibidas',''),(7,'backend_can_tx_errors','Errores TX ESP32',''),(8,'can_rx_invalid','Errores RX ESP32',''),(9,'mpu_errors','Errores MPU',''),(10,'telemetry_dropped','Pérdidas telemetría',''),(11,'telemetry_us_max','Coste telemetría','µs'),(12,'can_init_attempts','Intentos inicio CAN',''),(15,'can_rx_age_ms','Edad de orden CAN','ms')]:rear(key,w[idx],label,unit)
            rear('reset_reason_code',w[13]&255,'Motivo de reset ESP32 (código)');rear('can_error_flags',(w[13]>>8)&255,'Flags MCP2515')
            rear('wifi_clients',(w[13]>>16)&255,'Clientes Wi-Fi');rear('wifi_ap',(w[13]>>24)&1,'AP Wi-Fi')
            rear('mpu_ok',(w[4]>>24)&1,'Validez MPU según versión');rear('speed_valid',(w[4]>>25)&1,'Validez Hall según versión')
            rear('backend_profile',(w[4]>>28)&15,'Perfil trasero (1 legado, 3 v1.3)')
            if (w[4]>>28)&15==3:rear('reverse_limit_kph',(w[14]>>16)/100,'Límite R configurado','km/h')
    return signals

class History:
    def __init__(self,root,definitions=None):
        self.root=Path(root);self.definitions=definitions or {};self.lock=threading.RLock()
        for folder in ('blobs','batches','sessions'): (self.root/folder).mkdir(parents=True,exist_ok=True)
    def _id(self,value):
        if not isinstance(value,str) or not re.fullmatch('[a-f0-9]{32,64}',value):raise ValueError('Identificador inválido')
        return value
    def start(self):
        batch=secrets.token_hex(16);(self.root/'batches'/f'{batch}.json').write_text(dump({'files':{},'size':0}),encoding='utf-8');return {'batch':batch}
    def upload(self,batch,name,data):
        batch=self._id(batch);path=PurePosixPath(name.replace('\\','/'))
        if path.is_absolute() or '..' in path.parts or len(name)>240 or not path.name or ':' in name:raise ValueError('Nombre de archivo inválido')
        if not 0<len(data)<=MAX_FILE:raise ValueError('Archivo vacío o mayor de 32 MiB; conserva el original')
        with self.lock:
            manifest=self.root/'batches'/f'{batch}.json';m=json.loads(manifest.read_text(encoding='utf-8'))
            if len(m['files'])>=MAX_FILES or m['size']+len(data)>MAX_BATCH:raise ValueError('Límite de lote: 256 archivos / 1 GiB')
            if str(path) in m['files']:raise ValueError('Nombre repetido dentro del lote')
            digest=hashlib.sha256(data).hexdigest();blob=self.root/'blobs'/digest
            if not blob.exists():
                with blob.open('xb') as f:f.write(data)
            m['files'][str(path)]={'sha256':digest,'size':len(data)};m['size']+=len(data)
            manifest.write_text(dump(m),encoding='utf-8')
        return {'sha256':digest,'size':len(data)}
    def _db(self,sid):return sqlite3.connect(self.root/'sessions'/self._id(sid)/'data.sqlite')
    def list(self):
        rows=[]
        for p in (self.root/'sessions').glob('*/manifest.json'):
            try:rows.append(json.loads(p.read_text(encoding='utf-8')))
            except (OSError,ValueError):continue
        return sorted(rows,key=lambda r:r['imported_utc'],reverse=True)
    def finish(self,batch):
        with self.lock:
            m=json.loads((self.root/'batches'/f'{self._id(batch)}.json').read_text(encoding='utf-8'))
            groups={}
            for name,info in m['files'].items():groups.setdefault(str(PurePosixPath(name).parent),{})[name]=info
            results=[]
            for group,files in groups.items():
                if not any(PurePosixPath(n).suffix.lower() in ('.csv','.jsonl') for n in files):continue
                # JSONL sessions and legacy CSV are independent, modern SD directory is a bundle.
                modern=any(PurePosixPath(n).name.upper()=='META.TXT' for n in files)
                bundles=[files] if modern else [{n:i} for n,i in files.items() if PurePosixPath(n).suffix.lower() in ('.csv','.jsonl') and not re.fullmatch('E[0-9]{7}\.CSV',PurePosixPath(n).name.upper())]
                for bundle in bundles:results.append(self._import(bundle))
            if not results:raise ValueError('No hay archivos de registro reconocibles en el lote')
            return {'sessions':results}
    def _import(self,files):
        signature=dump(sorted((PurePosixPath(n).name,i['sha256']) for n,i in files.items()))
        sid=hashlib.sha256(signature.encode()).hexdigest();directory=self.root/'sessions'/sid
        if (directory/'manifest.json').exists():return {'id':sid,'duplicate':True}
        directory.mkdir(exist_ok=True)
        # A failed, unpublished derivation can be replaced; original blobs never are.
        db=sqlite3.connect(directory/'data.sqlite');db.executescript('DROP TABLE IF EXISTS frames;DROP TABLE IF EXISTS events;CREATE TABLE frames(id INTEGER PRIMARY KEY,t REAL NOT NULL,payload TEXT NOT NULL,raw TEXT NOT NULL);CREATE INDEX frame_time ON frames(t,id);CREATE TABLE events(id INTEGER PRIMARY KEY,t REAL,text TEXT);')
        result={'id':sid,'title':PurePosixPath(next(iter(files))).parent.name or PurePosixPath(next(iter(files))).stem,'imported_utc':stamp(),'test_date':None,'date_source':'sin confirmar','notes':'','objective':'','conditions':'','files':files,'frames':0,'duration_ms':0,'warnings':[],'warning_count':0,'closed':False,'versions':[],'schema':None}
        def warn(text):
            result['warning_count']+=1
            if len(result['warnings'])<100:result['warnings'].append(text)
        versions=set();seq=None;previous_tick=None;offset=0;first=None;last=-1;json_state={};node_sessions={};base_received=None
        def event(t,text):db.execute('INSERT INTO events(t,text) VALUES(?,?)',(max(t,0),text))
        def frame(t,signals,raw):
            nonlocal last
            if t<last:raise ValueError('tiempo retrocede')
            if last>=0 and t-last>1500:event(last,'Hueco de adquisición: %.3f s'%((t-last)/1000))
            # Preserve acquisition time for each field, no future sample when seeking back.
            for s in signals.values():s.setdefault('sample_t',t-s.get('age_ms',0))
            db.execute('INSERT INTO frames(t,payload,raw) VALUES(?,?,?)',(t,dump(signals),dump(raw)))
            result['frames']+=1;result['duration_ms']=max(result['duration_ms'],t);last=t
            if result['frames']%1000==0:db.commit()
        try:
            for name,info in sorted(files.items()):
                bn=PurePosixPath(name).name.upper();blob=self.root/'blobs'/info['sha256']
                if bn in ('META.TXT','END.TXT') or re.fullmatch('E[0-9]{7}\.CSV',bn):continue
                if not bn.endswith(('.CSV','.JSONL')):continue
                with blob.open(encoding='utf-8-sig',errors='strict',newline='') as f:
                    if bn.endswith('.JSONL'):
                        result['schema']='jsonl-v1'
                        for line_no,line in enumerate(f,1):
                            try:
                                if len(line)>65536:raise ValueError('línea demasiado larga')
                                record=json.loads(line)
                                if record.get('kind')!='telemetry':
                                    if record.get('text'):event(max(last,0),str(record['text'])[:1000])
                                    continue
                                data=record['frame'];received=datetime.fromisoformat(record['received_utc'])
                                if received.tzinfo is None:raise ValueError('fecha UTC sin zona')
                                ms=received.timestamp()*1000
                                if base_received is None:base_received=ms;result['test_date']=received.isoformat();result['date_source']='recepción PC; reloj de adquisición independiente'
                                t=ms-base_received;node=data['node'];session=data['session']
                                if data.get('v')!=1 or data.get('type')!='telemetry' or node not in ('esp32','stm32','bms'):raise ValueError('protocolo JSONL no reconocido')
                                if node_sessions.get(node)!=session:
                                    json_state={k:v for k,v in json_state.items() if v['node']!=node};node_sessions[node]=session;event(t,'Arranque '+node+' · '+str(session))
                                if data.get('fw'):versions.add(str(data['fw']))
                                for key,value in data['signals'].items():
                                    definition=self.definitions.get(key)
                                    if definition:
                                        label,unit,low,high,ttl=definition
                                        if isinstance(low,set):
                                            if value not in low:raise ValueError('señal enumerada inválida: '+key)
                                        else:value=number(value,low,high)
                                    else:
                                        label,unit,ttl=key,'',1500
                                        if not isinstance(value,(str,int,float,bool)) or isinstance(value,(int,float)) and not math.isfinite(value):raise ValueError('valor JSON inválido')
                                    s=field(value,label,unit,ttl=ttl,node=node);s['sample_t']=t;json_state[key]=s
                                current={k:{**v,'age_ms':max(0,t-v['sample_t'])} for k,v in json_state.items()}
                                frame(t,current,data)
                            except (ValueError,KeyError,TypeError,OverflowError) as exc:warn(f'{name}:{line_no}: {exc}');event(max(last,0),'Fila rechazada · '+str(line_no))
                    else:
                        header=f.readline();schema=3 if 'schema=3,' in header else 2 if 'schema=2,' in header else 0
                        if header.startswith('#'):
                            if schema not in (2,3):raise ValueError('Esquema CSV desconocido: '+name)
                            columns=f.readline()
                        else:columns=header
                        delimiter=';' if ';' in columns else ',';names=next(csv.reader([columns],delimiter=delimiter))
                        if names!=(NEW_COLUMNS if schema==3 else OLD_COLUMNS if schema==2 else LEGACY_COLUMNS):raise ValueError('Cabecera CSV no reconocida: '+name)
                        result['schema']=schema or 'legacy';reader=csv.DictReader(f,fieldnames=names,delimiter=delimiter)
                        for line_no,row in enumerate(reader,3 if schema else 2):
                            try:
                                if None in row or any(v is None for v in row.values()):raise ValueError('fila parcial / número de columnas')
                                tick=number(row['Tick_ms'] if schema else row['Tick']);mask=0
                                if schema==3:
                                    t=number(row['Elapsed_ms'],0,2**53-1);current_seq=number(row['Seq'])
                                    if seq is not None and current_seq<=seq:raise ValueError('secuencia repetida o fuera de orden')
                                    if seq is not None and current_seq!=seq+1:event(t,'Pérdida de muestras: '+str(current_seq-seq-1))
                                    seq=current_seq;mask=int(number(row['EventMask'],0,511))
                                else:
                                    if previous_tick is not None and tick<previous_tick:
                                        if previous_tick>0xf0000000 and tick<0x10000000:offset+=2**32
                                        else:offset=(last if last>=0 else 0)+(first or 0)-tick+1;event(max(last,0),'Reinicio o concatenación: separación histórica incierta')
                                    if first is None:first=tick
                                    t=offset+tick-first;previous_tick=tick
                                if schema:
                                    signals=csv_signals(row,schema);versions.add(row['Firmware'])
                                else:
                                    # Exact historical header: units of raw yaw/ADC not assumed.
                                    signals={key:field(number(row[col],-32768,65535),label,unit) for col,key,label,unit in [('DiEsq','rpm_fl','RPM delantera I','rpm'),('DiDir','rpm_fr','RPM delantera D','rpm'),('TrEsq','rpm_rl','RPM trasera I','rpm'),('TrDir','rpm_rr','RPM trasera D','rpm'),('Vel','speed_kph','Velocidad histórica','km/h')]}
                                    for col in ('Accel','Steer','Yaw'):signals['legacy_'+col]=field(number(row[col],-1e6,1e6),col+' histórico · unidad sin confirmar')
                                frame(t,signals,row)
                                if mask:event(t,' · '.join(label for bit,label in EVENTS.items() if mask&bit))
                            except (ValueError,KeyError,TypeError,OverflowError) as exc:warn(f'{name}:{line_no}: {exc}');event(max(last,0),'Fila rechazada · '+str(line_no))
            if not result['frames']:raise ValueError('No hay muestras válidas; originales conservados en blobs')
            if result['schema']==3:
                ending=next((i for n,i in files.items() if PurePosixPath(n).name.upper()=='END.TXT'),None)
                if ending:
                    values=dict(line.split('=',1) for line in (self.root/'blobs'/ending['sha256']).read_text(encoding='utf-8').splitlines() if '=' in line)
                    parts=sum(bool(re.fullmatch('D[0-9]{7}\.CSV',PurePosixPath(n).name.upper())) for n in files)
                    result['closed']=values.get('closed')=='1' and int(values.get('rows',-1))==result['frames'] and int(values.get('parts',-1))==parts and not result['warning_count']
                    result['dropped']=int(values.get('dropped',0))
                if not result['closed']:warn('Sesión incompleta: cierre ausente, partes faltantes o filas rechazadas')
            else:warn('Registro anterior sin cierre verificable ni separación garantizada de todas las pruebas')
            result['versions']=sorted(versions);db.commit()
            (directory/'manifest.json').write_text(dump(result),encoding='utf-8')
            return {'id':sid,'duplicate':False,'frames':result['frames'],'warnings':result['warning_count']}
        finally:db.close()
    def state(self,sid,at):
        at=number(at,0,2**53-1)
        with self._db(sid) as db:
            row=db.execute('SELECT id,t,payload,raw FROM frames WHERE t<=? ORDER BY t DESC,id DESC LIMIT 1',(at,)).fetchone()
            if not row:return {'at_ms':at,'signals':{},'raw':{},'frame_id':0}
            ident,t,payload,raw=row;signals=json.loads(payload)
            for key,s in signals.items():
                s['age_ms']=max(0,at-s['sample_t']);s['sample_id']=f'{sid}:{s["sample_t"]}:{key}'
                if s['age_ms']>s['stale_after_ms']:s['quality']='stale'
            if signals.get('backend_profile',{}).get('value')==3 or any('v1.3' in v for v in self.manifest(sid)['versions']):
                for flag,keys in [('mpu_ok',('yaw_dps',)),('speed_valid',('rpm_rl','rpm_rr'))]:
                    if flag in signals and (signals[flag]['quality']!='fresh' or not signals[flag]['value']):
                        for key in keys:
                            if key in signals:signals[key]['quality']='stale'
            return {'at_ms':at,'sample_ms':t,'signals':signals,'raw':json.loads(raw),'frame_id':ident}
    def window(self,sid,start,end):
        start=number(start,0,2**53-1);end=number(end,start,min(start+60000,2**53-1))
        with self._db(sid) as db:
            rows=db.execute('SELECT id,t,payload FROM frames WHERE t>=? AND t<=? ORDER BY t,id LIMIT 20001',(start,end)).fetchall()
        if len(rows)>20000:raise ValueError('Ventana demasiado densa; reduce el intervalo')
        return {'frames':[{'id':i,'t':t,'signals':json.loads(p)} for i,t,p in rows]}
    def events(self,sid,start=0):
        with self._db(sid) as db:rows=db.execute('SELECT id,t,text FROM events WHERE id>? ORDER BY id LIMIT 500',(int(number(start)),)).fetchall()
        return {'events':[dict(id=i,t=t,text=text) for i,t,text in rows],'next':rows[-1][0] if len(rows)==500 else None}
    def manifest(self,sid):return json.loads((self.root/'sessions'/self._id(sid)/'manifest.json').read_text(encoding='utf-8'))
    def annotate(self,sid,data):
        with self.lock:
            m=self.manifest(sid)
            for key in ('title','notes','objective','conditions','test_date'):
                if key in data:
                    if not isinstance(data[key],str) or len(data[key])>4000:raise ValueError('Anotación inválida')
                    m[key]=data[key]
                    if key=='test_date':m['date_source']='anotación manual; no sincronizada'
            (self.root/'sessions'/sid/'manifest.json').write_text(dump(m),encoding='utf-8');return m
    def export(self,sid):
        m=self.manifest(sid);out=io.BytesIO()
        with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
            z.writestr('manifest.json',dump(m))
            for name,info in m['files'].items():z.write(self.root/'blobs'/info['sha256'],'originales/'+name)
        return out.getvalue()
