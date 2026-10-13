"""Buscador local FTS5 por documento y página, sin API ni embeddings."""
import argparse, csv, hashlib, json, re, sqlite3
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
DB=ROOT/'08_INDICE_GENERADO'/'busqueda.sqlite'

def connect():
    c=sqlite3.connect(DB)
    c.execute('CREATE TABLE IF NOT EXISTS versiones (id TEXT PRIMARY KEY, firma TEXT)')
    c.execute("CREATE VIRTUAL TABLE IF NOT EXISTS fragmentos USING fts5(id UNINDEXED, titulo, localizacion UNINDEXED, texto, tokenize='unicode61 remove_diacritics 2')")
    return c

def index(c):
    rows=list(csv.DictReader((ROOT/'03_CATALOGO/documentos.csv').open(encoding='utf-8-sig')))
    active={r['id'] for r in rows if r['estado_extraccion']=='extraido_no_revisado'}
    for (oldid,) in list(c.execute('SELECT id FROM versiones')):
        if oldid not in active:
            c.execute('DELETE FROM fragmentos WHERE id=?',(oldid,))
            c.execute('DELETE FROM versiones WHERE id=?',(oldid,))
    changed=0
    for r in rows:
        if r['estado_extraccion']!='extraido_no_revisado': continue
        txt=ROOT/r['ruta_texto']; signature=hashlib.sha256(txt.read_bytes()).hexdigest()
        old=c.execute('SELECT firma FROM versiones WHERE id=?',(r['id'],)).fetchone()
        if old and old[0]==signature: continue
        c.execute('DELETE FROM fragmentos WHERE id=?',(r['id'],))
        pages=txt.parent/'paginas.json'
        if r['formato']=='.pdf':
            parts=[(f"Página PDF {p['pagina_pdf']}",p['texto']) for p in json.loads(pages.read_text(encoding='utf-8'))]
        elif r['formato']=='.pptx':
            chunks=re.split(r'===== DIAPOSITIVA (\d+) =====\n',txt.read_text(encoding='utf-8'))
            parts=[('Diapositiva '+chunks[i],chunks[i+1]) for i in range(1,len(chunks),2)]
        else:
            text=txt.read_text(encoding='utf-8')
            parts=[(f'Caracteres {i+1}-{min(i+5000,len(text))}',text[i:i+5000]) for i in range(0,len(text),4500)]
        c.executemany('INSERT INTO fragmentos(id,titulo,localizacion,texto) VALUES(?,?,?,?)',[(r['id'],r['titulo'],loc,t) for loc,t in parts])
        c.execute('INSERT OR REPLACE INTO versiones VALUES (?,?)',(r['id'],signature)); changed+=1
    c.commit()
    print(json.dumps({'documentos_actualizados':changed,'fragmentos':c.execute('SELECT COUNT(*) FROM fragmentos').fetchone()[0]}))

if __name__=='__main__':
    import sys; sys.stdout.reconfigure(encoding='utf-8')  # la consola de Windows (cp1252) falla con Ω, ₄, etc.
    p=argparse.ArgumentParser(); p.add_argument('--indexar',action='store_true'); p.add_argument('--consulta'); p.add_argument('--documento'); p.add_argument('--pagina',type=int); p.add_argument('--limite',type=int,default=6); a=p.parse_args()
    c=connect()
    if a.indexar: index(c)
    elif a.pagina and a.documento:
        for loc,t in c.execute('SELECT localizacion,texto FROM fragmentos WHERE id=? AND localizacion=?',(a.documento,f'Página PDF {a.pagina}')): print(loc+'\n'+t)
    elif a.consulta:
        sql="SELECT id,titulo,localizacion,snippet(fragmentos,3,'[',']',' … ',48) FROM fragmentos WHERE fragmentos MATCH ?"
        params=[a.consulta]
        if a.documento: sql+=' AND id=?'; params.append(a.documento)
        sql+=' ORDER BY bm25(fragmentos) LIMIT ?'; params.append(max(1,min(a.limite,30)))
        for row in c.execute(sql,params): print(json.dumps(dict(zip(['id','titulo','localizacion','extracto'],row)),ensure_ascii=False))
    else: p.error('Usar --indexar, --consulta o --documento con --pagina')
