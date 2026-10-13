from pathlib import Path
import csv, hashlib, shutil, json, zipfile
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[2]
source = next(Path('C:/Users/nandi/Downloads').glob('Cronograma*.docx'))
dest = ROOT/'02_ORIGINALES/propuesta_2026-09-30'/source.name
dest.parent.mkdir(parents=True, exist_ok=True)
digest = hashlib.sha256(source.read_bytes()).hexdigest()
if dest.exists() and hashlib.sha256(dest.read_bytes()).hexdigest() != digest:
    raise RuntimeError('La copia existente difiere; no se sobrescribe')
if not dest.exists():
    shutil.copy2(source, dest)
assert hashlib.sha256(dest.read_bytes()).hexdigest() == digest
ns = {'w': 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'}
with zipfile.ZipFile(dest) as z:
    body = ET.fromstring(z.read('word/document.xml')).find('w:body', ns)
lines = []
for el in body:
    if el.tag.endswith('}tbl'):
        lines.append('TABLA')
        for row in el.findall('w:tr', ns):
            lines.append(' | '.join(' '.join(t.text or '' for t in cell.findall('.//w:t', ns)) for cell in row.findall('w:tc', ns)))
    elif el.tag.endswith('}p'):
        lines.append(''.join(t.text or '' for t in el.findall('.//w:t', ns)))
out = ROOT/'08_INDICE_GENERADO/texto'/digest
out.mkdir(parents=True, exist_ok=True)
(out/'texto.txt').write_text('\n'.join(lines), encoding='utf-8')
catalog = ROOT/'03_CATALOGO/documentos.csv'
with catalog.open(encoding='utf-8-sig', newline='') as f:
    reader = csv.DictReader(f)
    fields = reader.fieldnames
    rows = list(reader)
record = dict.fromkeys(fields, '')
record.update(id='DOC-0043', ruta_original=dest.relative_to(ROOT).as_posix(), titulo=source.stem,
    fecha_documento='2026-09-30', sistema='integracion_del_kart', configuracion='Propuesta de etapa 2026/27; montaje de partida por verificar',
    estado='pendiente_verificar', resumen='Cronograma personal: diferencial electrónico inteligente y estimación de autonomía hasta febrero de 2027. Propuesta, no aprobación ni resultado técnico.',
    referencia_precisa='Calendarização; tabla Tarefas; objetivos hasta febrero de 2027; fecha y participantes',
    sha256=digest, fecha_indexacion='2026-09-30', version_extractor='OOXML-1', formato='.docx',
    estado_extraccion='extraido_no_revisado', estado_lectura='parcial', ruta_texto=(out/'texto.txt').relative_to(ROOT).as_posix(),
    copias='1', lote='P1', tipo='cronograma_propuesta', cobertura_lectura='Texto y tablas completos extraídos de OOXML y analizados; disposición visual, firmas y paginación no verificadas')
existing = next((r for r in rows if r['id']=='DOC-0043'), None)
if existing and existing['sha256'] != digest:
    raise RuntimeError('DOC-0043 ya corresponde a otra fuente')
if not existing:
    rows.append(record)
    with catalog.open('w', encoding='utf-8-sig', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)
link = '../../'+dest.relative_to(ROOT).as_posix().replace(' ', '%20')
(ROOT/'03_CATALOGO/fichas/DOC-0043.md').write_text(f'''# DOC-0043 · Cronograma de Smart Electric Kart V1.0

Propuesta del usuario incorporada el 30-09-2026 como inicio de su investigación y proyecto. Separada de la entrega histórica de 42 documentos.

- Original: [Word conservado]({link}). Copiado desde Descargas; archivo de origen conservado.
- SHA-256 verificado en origen y copia: `{digest}`.
- Fecha declarada: 30 de septiembre de 2026.
- Participantes escritos: Armando Aimacaña e Irvin Chasi; orientadores Luís Neves y Paulo Ventura. Revisar la grafía del nombre en el Word; no se infiere aprobación ni firma.
- Configuración: propuesta 2026/27; configuración física de partida pendiente de verificar.
- Objetivos declarados hasta febrero de 2027: implementación del diferencial electrónico inteligente y estimación de la autonomía real.
- Cobertura: extracción y revisión del texto y de todas las filas de las tablas; sin renderizado ni comprobación visual de disposición, firmas o paginación.
- Referencias: tabla «Tarefas» y bloque «Objetivos a cumprir até fevereiro de 2027». No se asignan páginas sin verificar.
- [Análisis y organización recomendada](../../06_TRABAJO/INICIO_INVESTIGACION_2026-27.md).

La extracción está en `08_INDICE_GENERADO/texto/{digest}/texto.txt`. Si cambia el Word, marcar esta ficha y el análisis como pendientes de revisión antes de actualizar sus conclusiones.
''', encoding='utf-8')
print(json.dumps({'original': str(dest), 'sha256': digest, 'documentos_catalogados': len(rows)}, ensure_ascii=False))
