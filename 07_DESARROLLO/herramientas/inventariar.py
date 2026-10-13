"""Inventario y extracción incremental. No ejecuta contenido de los originales."""
from pathlib import Path, PurePosixPath
import argparse, collections, csv, hashlib, json, os, re, shutil, stat, subprocess, zipfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / '02_ORIGINALES' / 'entrega_2026-09-28'
CACHE = ROOT / '08_INDICE_GENERADO'
CAT = ROOT / '03_CATALOGO'
VERSION = '1'
POPPLER = Path(r'C:\Users\nandi\.cache\codex-runtimes\codex-primary-runtime\dependencies\native\poppler\Library\bin')

def sha(p):
    h = hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda: f.read(1024*1024), b''): h.update(b)
    return h.hexdigest()

def write_json(p, data):
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding='utf-8')

def csv_write(p, rows, fields):
    with p.open('w', newline='', encoding='utf-8-sig') as f:
        w = csv.DictWriter(f, fieldnames=fields, extrasaction='ignore'); w.writeheader(); w.writerows(rows)

def baseline():
    paths = [ROOT/'Relatórios de anos anteriores', ROOT/'Smart Eletric Kart Cross-2223406-2223403.zip', ROOT/'Loop paragraph.loop']
    rows = []
    for p in paths:
        for f in sorted(p.rglob('*')) if p.is_dir() else [p]:
            if f.is_file(): rows.append({'ruta_recibida':f.relative_to(ROOT).as_posix(), 'bytes':f.stat().st_size, 'sha256':sha(f)})
    write_json(CACHE/'recepcion_original.json', rows)
    print(json.dumps({'files':len(rows), 'bytes':sum(r['bytes'] for r in rows)}))

def verify():
    rows = json.loads((CACHE/'recepcion_original.json').read_text(encoding='utf-8'))
    bad = [r['ruta_recibida'] for r in rows if not (BASE/r['ruta_recibida']).is_file() or sha(BASE/r['ruta_recibida']) != r['sha256']]
    if bad: raise RuntimeError(str(bad))
    print('Recepción verificada por SHA-256:', len(rows))

def safe_member(name):
    p = PurePosixPath(name.replace('\\','/'))
    if p.is_absolute() or '..' in p.parts or any(':' in s for s in p.parts): raise ValueError('Ruta ZIP no segura: '+name)
    return p

def unzip_all():
    log_path = CACHE/'descompresion.json'
    logs = json.loads(log_path.read_text(encoding='utf-8')) if log_path.exists() else []
    completed = {x['zip']: x for x in logs}
    seen = set()
    for depth in range(8):
        todo = [p for p in BASE.rglob('*.zip') if str(p) not in seen]
        if not todo: break
        for p in todo:
            seen.add(str(p)); rel = p.relative_to(ROOT).as_posix(); digest = sha(p)
            if rel in completed and completed[rel]['sha256'] == digest: continue
            with zipfile.ZipFile(p) as z:
                members = [i for i in z.infolist() if not i.is_dir()]
                if sum(i.file_size for i in members) > min(shutil.disk_usage(BASE).free//2, 20*1024**3): raise ValueError('ZIP excede límite de extracción')
                paths = [safe_member(i.filename) for i in members]
                if len({str(n).casefold() for n in paths}) != len(paths): raise ValueError('Colisión de nombres en ZIP')
                for i in members:
                    if stat.S_ISLNK(i.external_attr >> 16): raise ValueError('Enlace simbólico ZIP no admitido')
                target = p.parent / p.stem
                # Los ZIP ya extraídos por el usuario conservan su carpeta contenedora.
                # Si hay divergencias, crear otra extracción completa sin sobrescribirlas.
                contents = []
                for i,n in zip(members,paths):
                    with z.open(i) as f:
                        h=hashlib.sha256()
                        for b in iter(lambda:f.read(1024*1024),b''): h.update(b)
                    contents.append((i,n,h.hexdigest()))
                conflict = any((target/str(n)).exists() and sha(target/str(n)) != h for i,n,h in contents)
                if conflict: target = p.parent/(p.stem+'_extraido_'+digest[:8])
                extracted=0; reused=0
                for i,n,h in contents:
                    dest = target/str(n)
                    if not dest.resolve().is_relative_to(target.resolve()): raise ValueError('Destino fuera del directorio')
                    if dest.exists():
                        if sha(dest)!=h: raise ValueError('Conflicto de contenido: '+str(dest))
                        reused+=1; continue
                    dest.parent.mkdir(parents=True,exist_ok=True)
                    with z.open(i) as src, dest.open('xb') as out: shutil.copyfileobj(src,out)
                    if sha(dest)!=h: raise ValueError('Fallo de verificación: '+str(dest))
                    extracted+=1
                record={'zip':rel,'sha256':digest,'destino':target.relative_to(ROOT).as_posix(),'miembros':len(members),'extraidos':extracted,'reutilizados':reused,'conflicto_previo':conflict,'estado':'verificado_sha256'}
                logs=[r for r in logs if r['zip']!=rel]+[record]
                write_json(log_path,logs)
                print(json.dumps(record,ensure_ascii=False),flush=True)
    remaining=[str(p) for p in BASE.rglob('*') if p.is_file() and p.suffix.lower() in {'.rar','.7z','.gz','.tar'}]
    print('Otros comprimidos:',json.dumps(remaining,ensure_ascii=False))

def classify(p):
    s=p.as_posix().lower(); ext=p.suffix.lower()
    if '__macosx' in s or p.name.startswith('._') or p.name=='.DS_Store': return 'metadatos_macos'
    if ext in {'.zip','.rar','.7z'}: return 'comprimido'
    if ext=='.csv': return 'datos_de_prueba'
    if ext in {'.pdf','.docx','.doc','.xlsx','.xls','.pptx','.loop'}: return 'documento'
    if '/libraries/' in s or '/drivers/' in s or '/middleware' in s or '/middlewar' in s: return 'dependencia_software'
    if ext in {'.o','.d','.su','.cyclo','.elf','.hex','.bin','.a','.lib','.dll','.exe','.cache','.temp','.out','.map','.list','.mk'}: return 'compilacion_o_binario'
    if ext in {'.ino','.c','.cpp','.h','.hpp','.ioc','.s','.pde','.py'}: return 'codigo_o_configuracion'
    if ext in {'.sldprt','.sldasm','.slddrw','.pdsprj','.sch','.brd','.pcbdoc','.prjpcb','.pcb','.dsn','.lay6','.lay','.lpp','.lps','.step','.stp'}: return 'diseno'
    if ext in {'.jpg','.jpeg','.png','.bmp','.svg','.mp4','.mov'}: return 'imagen_o_video'
    return 'otro'

def inventory():
    oldfile=CAT/'inventario_archivos.csv'
    old=list(csv.DictReader(oldfile.open(encoding='utf-8-sig'))) if oldfile.exists() else []
    oldmap={r['ruta_original']:r for r in old}
    next_id=max([int(r['id'][4:]) for r in old]+[0])+1
    files=[]
    for p in sorted(BASE.rglob('*')):
        if not p.is_file(): continue
        rel=p.relative_to(ROOT).as_posix(); s=p.stat(); prev=oldmap.get(rel)
        # Hash cache uses file size and mtime; first scan hashes every byte.
        digest=prev['sha256'] if prev and prev['bytes']==str(s.st_size) and prev['mtime_ns']==str(s.st_mtime_ns) else sha(p)
        ident=prev['id'] if prev else f'ARC-{next_id:05d}'
        if not prev: next_id+=1
        files.append({'id':ident,'ruta_original':rel,'extension':p.suffix.lower(),'bytes':s.st_size,'mtime_ns':s.st_mtime_ns,'sha256':digest,'categoria':classify(p),'duplicado_de':''})
    groups={}
    for r in files:
        if r['sha256'] in groups: r['duplicado_de']=groups[r['sha256']]
        else: groups[r['sha256']]=r['id']
    csv_write(oldfile,files,list(files[0]))
    duplicates=[r for r in files if r['duplicado_de']]
    csv_write(CAT/'duplicados_exactos.csv',duplicates,list(files[0]))
    stats={'archivos':len(files),'bytes':sum(r['bytes'] for r in files),'contenidos_unicos':len(groups),'copias_duplicadas':len(duplicates),'bytes_duplicados':sum(r['bytes'] for r in duplicates),'categorias':dict(collections.Counter(r['categoria'] for r in files)),'formatos':dict(collections.Counter(r['extension'] for r in files))}
    write_json(CACHE/'inventario_resumen.json',stats); print(json.dumps(stats,ensure_ascii=False))

def extract(p,h):
    out=CACHE/'texto'/h
    meta=out/'meta.json'
    if meta.exists():
        data=json.loads(meta.read_text(encoding='utf-8'))
        stale = (p.suffix.lower()=='.pptx' and data['estado']=='formato_pendiente') or (p.suffix.lower()=='.docx' and 'imagenes' not in data)
        if data.get('version_extractor')==VERSION and data['estado']!='error' and not stale: return data
    out.mkdir(parents=True,exist_ok=True)
    result={'version_extractor':VERSION,'sha256':h,'estado':'extraido_no_revisado','paginas':None,'paginas_poco_texto':[]}
    try:
        if p.suffix.lower()=='.pdf':
            exe=POPPLER/'pdftotext.exe'
            if exe.exists():
                r=subprocess.run([str(exe),'-layout','-enc','UTF-8',str(p),str(out/'texto_bruto.txt')],capture_output=True,timeout=120)
                if r.returncode: raise RuntimeError(r.stderr.decode('utf-8',errors='replace')[:300])
                text=(out/'texto_bruto.txt').read_text(encoding='utf-8')
                pages=text.split('\f')
                if pages and not pages[-1].strip(): pages.pop()
            else:
                from pypdf import PdfReader
                pages=[pg.extract_text() or '' for pg in PdfReader(p).pages]
            result['paginas']=len(pages)
            result['paginas_poco_texto']=[i for i,t in enumerate(pages,1) if len(t.strip())<80]
            write_json(out/'paginas.json',[{'pagina_pdf':i,'texto':t} for i,t in enumerate(pages,1)])
            text='\n\n'.join(f'===== PAGINA PDF {i} =====\n{t}' for i,t in enumerate(pages,1))
        elif p.suffix.lower()=='.docx':
            from docx import Document
            doc=Document(p); parts=[]
            ns='{http://schemas.openxmlformats.org/wordprocessingml/2006/main}'
            for i,el in enumerate(doc.element.body,1):
                if el.tag==ns+'p': parts.append(f'[BLOQUE {i}] '+''.join(n.text or '' for n in el.iter(ns+'t')))
                elif el.tag==ns+'tbl':
                    parts.append(f'[TABLA BLOQUE {i}]')
                    for row in el.findall(ns+'tr'):
                        parts.append(' | '.join(' '.join(n.text or '' for n in cell.iter(ns+'t')) for cell in row.findall(ns+'tc')))
            text='\n'.join(parts)
            result['nota']='Bloques y tablas del cuerpo; no equivale a paginación ni revisión visual; imágenes, comentarios y objetos requieren revisión.'
            plain=''.join(n.text or '' for n in doc.element.body.iter(ns+'t'))
            with zipfile.ZipFile(p) as z:
                media=[n for n in z.namelist() if n.startswith('word/media/') and not n.endswith('/')]
                result['imagenes']=len(media)
                if len(plain.strip())<200 and media:
                    result['estado']='texto_insuficiente_revisar_imagenes'
                    (out/'imagenes').mkdir(exist_ok=True)
                    for name in media: (out/'imagenes'/Path(name).name).write_bytes(z.read(name))
        elif p.suffix.lower()=='.pptx':
            # Texto de las diapositivas en el orden de presentación, sin modificar el archivo.
            import posixpath
            with zipfile.ZipFile(p) as z:
                ns={'p':'http://schemas.openxmlformats.org/presentationml/2006/main','a':'http://schemas.openxmlformats.org/drawingml/2006/main'}
                relns='{http://schemas.openxmlformats.org/officeDocument/2006/relationships}id'
                rels={x.attrib['Id']:x.attrib['Target'] for x in ET.fromstring(z.read('ppt/_rels/presentation.xml.rels'))}
                slides=ET.fromstring(z.read('ppt/presentation.xml')).findall('p:sldIdLst/p:sldId',ns)
                parts=[]
                for i,s in enumerate(slides,1):
                    target=rels[s.attrib[relns]]
                    path=target.lstrip('/') if target.startswith('/') else posixpath.normpath('ppt/'+target)
                    xml=ET.fromstring(z.read(path))
                    parts.append(f'===== DIAPOSITIVA {i} =====\n'+'\n'.join(n.text or '' for n in xml.findall('.//a:t',ns)))
            text='\n\n'.join(parts); result['diapositivas']=len(slides)
            result['nota']='Texto de diapositivas; imágenes, gráficos, notas del presentador y diseño no revisados.'
        elif p.suffix.lower()=='.xlsx':
            # Inventario OOXML de hojas; no ejecuta ni recalcula fórmulas.
            with zipfile.ZipFile(p) as z:
                ns={'s':'http://schemas.openxmlformats.org/spreadsheetml/2006/main'}
                sheets=ET.fromstring(z.read('xl/workbook.xml')).findall('s:sheets/s:sheet',ns)
                result['hojas']=[s.attrib['name'] for s in sheets]
            text='Hojas del libro (contenido y fórmulas pendientes de análisis):\n'+'\n'.join(result['hojas'])
            result['estado']='inventariado_sin_analisis'
        else:
            result['estado']='formato_pendiente'; text='Formato pendiente de revisión con su aplicación de origen.'
        (out/'texto.txt').write_text(text,encoding='utf-8')
        result['caracteres']=len(text)
    except Exception as e:
        result.update(estado='error',error=str(e)[:500])
    write_json(meta,result)
    return result

def documents():
    files=list(csv.DictReader((CAT/'inventario_archivos.csv').open(encoding='utf-8-sig')))
    oldfile=CAT/'documentos.csv'
    old=list(csv.DictReader(oldfile.open(encoding='utf-8-sig'))) if oldfile.exists() else []
    oldmap={r['sha256']:r for r in old}
    n=max([int(r['id'][4:]) for r in old if r['id'].startswith('DOC-')]+[0])+1
    unique={}
    for r in files:
        if r['categoria']=='documento': unique.setdefault(r['sha256'],[]).append(r)
    docs=[]
    for h, copies in unique.items():
        r=copies[0]; p=ROOT/r['ruta_original']; prev=oldmap.get(h,{})
        ident=prev.get('id',f'DOC-{n:04d}')
        if not prev: n+=1
        info=extract(p,h)
        row={'id':ident,'ruta_original':r['ruta_original'],'titulo':p.stem,'fecha_documento':prev.get('fecha_documento','desconocido'),'sistema':prev.get('sistema','por_clasificar'),'configuracion':prev.get('configuracion','por_verificar'),'estado':prev.get('estado','pendiente_verificar'),'resumen':prev.get('resumen',''),'referencia_precisa':prev.get('referencia_precisa',''),'sha256':h,'fecha_indexacion':'2026-09-28','version_extractor':VERSION,'sustituido_por':prev.get('sustituido_por',''),'duplicado_de':'','formato':p.suffix.lower(),'paginas':info.get('paginas') or '', 'paginas_poco_texto':','.join(map(str,info.get('paginas_poco_texto',[]))),'estado_extraccion':info['estado'],'estado_lectura':prev.get('estado_lectura','pendiente'),'ruta_texto':(CACHE/'texto'/h/'texto.txt').relative_to(ROOT).as_posix(),'copias':len(copies),'rutas_alternativas':' | '.join(x['ruta_original'] for x in copies[1:]),'hojas':' | '.join(info.get('hojas',[]))}
        # Conservar campos de curación añadidos después de la extracción inicial.
        for key,value in prev.items():
            if key not in row: row[key]=value
        docs.append(row)
        print(ident,p.name,info['estado'],info.get('paginas'),flush=True)
    fields=list(dict.fromkeys(key for row in docs for key in row))
    csv_write(oldfile,docs,fields)
    write_json(CACHE/'documentos_resumen.json',{'documentos_unicos':len(docs),'formatos':dict(collections.Counter(r['formato'] for r in docs)),'paginas_pdf':sum(int(r['paginas']) for r in docs if r['paginas']),'estados':dict(collections.Counter(r['estado_extraccion'] for r in docs))})

if __name__=='__main__':
    a=argparse.ArgumentParser(); a.add_argument('accion',choices=['baseline','verify','unzip','inventory','documents']); args=a.parse_args()
    CACHE.mkdir(exist_ok=True); CAT.mkdir(exist_ok=True)
    {'baseline':baseline,'verify':verify,'unzip':unzip_all,'inventory':inventory,'documents':documents}[args.accion]()
