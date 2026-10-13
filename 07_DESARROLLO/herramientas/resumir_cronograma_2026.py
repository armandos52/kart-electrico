from pathlib import Path
from zipfile import ZipFile
from lxml import etree as E
import hashlib

root=Path(__file__).resolve().parents[2]
src=next((root/'02_ORIGINALES/propuesta_2026-09-30').glob('*.docx'))
out=root/'07_DESARROLLO/propuesta_2026-27/Cronograma_Smart_Electric_Kart_proposta_resumida.docx'
ns={'w':'http://schemas.openxmlformats.org/wordprocessingml/2006/main'}
with ZipFile(src) as z:
    x=E.fromstring(z.read('word/document.xml'))
    tables=x.findall('w:body/w:tbl',ns)
    def put(cell,text):
        nodes=cell.findall('.//w:t',ns)
        if nodes:
            nodes[0].text=text
            for n in nodes[1:]: n.text=''
        else:
            p=cell.find('w:p',ns)
            r=p.find('w:r',ns)
            if r is None: r=E.SubElement(p,'{'+ns['w']+'}r')
            E.SubElement(r,'{'+ns['w']+'}t').text=text
    tasks=[
        ('Estudo e diagnóstico da configuração do Kart',[1]),
        ('Definição da melhoria, sensores e plano de ensaios',[1,2]),
        ('Aperfeiçoamento e validação do diferencial eletrónico',[2,3,4,5]),
        ('Aperfeiçoamento da interface com o utilizador',[7,8]),
        ('Ensaios de referência, comparação e validação final',[2,3,4,5,9,10]),
        ('Documentação e registo de resultados',[1,2,3,4,5,7,8,9,10,11]),
        ('Medição do consumo e estimação da autonomia',[4,5])]
    rows=tables[2].findall('w:tr',ns)
    for row,(label,months) in zip(rows[1:],tasks):
        cells=row.findall('w:tc',ns)
        put(cells[0],label)
        for j,c in enumerate(cells[1:],1): put(c,'X' if j in months else '')
    objectives=tables[3].findall('w:tr',ns)
    put(objectives[1].find('w:tc',ns),'Aperfeiçoar o diferencial eletrónico e comparar os resultados com a configuração inicial.')
    put(objectives[2].find('w:tc',ns),'Estimar a autonomia real com medições de consumo e condições de ensaio definidas.')
    for p in x.findall('w:body/w:p',ns):
        text=''.join(p.itertext())
        if 'Data:' in text:
            put(p,'Data: 1 de outubro de 2026')
    with ZipFile(out,'w') as w:
        for info in z.infolist():
            w.writestr(info,E.tostring(x,xml_declaration=True,encoding='UTF-8',standalone=True) if info.filename=='word/document.xml' else z.read(info.filename))
with ZipFile(src) as a,ZipFile(out) as b:
    changed=[n for n in a.namelist() if a.read(n)!=b.read(n)]
assert changed==['word/document.xml'],changed
print(out)
print('Solo cambió word/document.xml; logotipo y formato conservados.')
