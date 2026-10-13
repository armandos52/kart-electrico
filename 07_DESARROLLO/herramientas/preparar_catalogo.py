"""Genera navegación y fichas de recepción a partir del catálogo. No resume con IA."""
import csv,json
from pathlib import Path
from urllib.parse import quote
from inventariar import ROOT,CAT,CACHE,csv_write

def link(label,path): return f'[{label}]({quote(path,safe="/#")})'

def main():
    rows=list(csv.DictReader((CAT/'documentos.csv').open(encoding='utf-8-sig')))
    notes={
    1:('pendiente','contenedor_loop','por_identificar','Archivo Microsoft Loop binario. Conservado; contenido pendiente de abrir o exportar desde su aplicación.'),
    2:('L5','informe','integracion_del_kart','Informe 2018/19: diferencial y baterías/BMS. Orientación basada en portada y resumen; lectura completa pendiente.'),
    3:('L5','informe','integracion_del_kart','Informe titulado 24/25: migración a STM32 y reorganización eléctrica. La portada dice julio de 2024; fecha por aclarar.'),
    4:('L2','datasheet','cableado_y_protecciones','Ficha BC546/547/548; identificación inicial del componente, sin validar parámetros.'),
    5:('L2','datasheet','cableado_y_protecciones','Contactor DC CZW200A; falta relacionar variante y montaje instalado.'),
    6:('L2','datasheet','cableado_y_protecciones','HEF4013B, biestable doble tipo D; identificación inicial.'),
    7:('L3','datasheet','software_y_telemetria','Catálogo Honeywell con sensores Hall GT1; portada de catálogo y página 2.'),
    8:('L3','manual','motor_y_controlador','Manual Kelly KEB revisión 3.4, febrero de 2023. Comparar con DOC-0031 antes de reutilizar una síntesis.'),
    9:('L2','datasheet','cableado_y_protecciones','Relés de PCB serie RZ; conservar referencia del archivo y verificar variante.'),
    10:('L2','datasheet','cableado_y_protecciones','Convertidores DC/DC TEN 3N; identificación inicial.'),
    11:('L2','datasheet','cableado_y_protecciones','Optoacopladores TLP521; material de varias páginas, sin validación de parámetros.'),
    12:('L4','manual','software_y_telemetria','STM32 UM1907 Rev 3, marzo de 2016, 49 páginas. Diferente revisión de DOC-0034.'),
    13:('L3','hoja_calculo','motor_y_controlador','Libro de valores de controladores. Hojas inventariadas; datos y fórmulas pendientes.'),
    14:('L3','hoja_calculo','software_y_telemetria','Libro asociado al ADC. Hojas inventariadas; datos y fórmulas pendientes.'),
    15:('L3','capturas_configuracion','motor_y_controlador','Word con cinco imágenes y sin texto sustantivo. Primera captura revisada: programa de configuración Kelly; faltan las restantes.'),
    16:('L3','plano','chasis_frenos_y_transmision','Plano Part1 de la carpeta CaixaBaterias; requiere revisión visual.'),
    17:('L3','plano','chasis_frenos_y_transmision','Plano Part2 de la carpeta CaixaBaterias; requiere revisión visual.'),
    18:('L3','plano','chasis_frenos_y_transmision','Plano Part3 de la carpeta CaixaBaterias; requiere revisión visual.'),
    19:('L3','plano_pcb','cableado_y_protecciones','Capa inferior de PCB del sensor de corriente; clasificación por ubicación, pendiente de lectura visual.'),
    20:('L3','plano_pcb','cableado_y_protecciones','Capa superior de PCB del sensor de corriente; clasificación por ubicación, pendiente de lectura visual.'),
    21:('L5','informe_editable','integracion_del_kart','Informe Word del paquete 2023/24. Comparar con DOC-0022 y DOC-0027; equivalencia aún no comprobada.'),
    22:('L5','informe','integracion_del_kart','Informe 2023/24 dentro del ZIP: portada fechada 12 de febrero de 2024 y resumen diferente de DOC-0027. Conservar ambas versiones.'),
    23:('L5','informe','integracion_del_kart','Vehículo eléctrico II, diferencial eléctrico, curso 2008/09. Portada verificada visualmente; extracción de texto irregular.'),
    24:('L5','informe','integracion_del_kart','SEKC, septiembre de 2011. Resumen sobre conversión a propulsión eléctrica e integración de componentes.'),
    25:('L5','informe','integracion_del_kart','Smart Electric KartCross, control de potencia, agosto de 2014. Portada y resumen revisados.'),
    26:('L5','informe','integracion_del_kart','Kart-Cross con tracción eléctrica, curso 2015/16. Diferencial electrónico con Arduino; revisión inicial del resumen y comienzo de conclusiones.'),
    27:('L5','informe','integracion_del_kart','Informe 2023/24 entregado por separado: portada del 15 de julio de 2024. Resumen y conclusiones revisados; comparar con DOC-0022.'),
    28:('L1','presentacion','integracion_del_kart','Presentación asociada al paquete más reciente. Texto de 17 diapositivas extraído; gráficos, vídeos y notas sin revisar.'),
    29:('L2','datasheet','cableado_y_protecciones','La portada identifica módulos HLK-10D48xxB, aunque el nombre del archivo menciona VRB4805LD. Verificar la referencia antes de usar valores.'),
    30:('L2','datasheet','bateria_y_bms','Ficha de batería Thunder Sky Winston. La extracción mezcla idiomas y columnas; revisar visualmente antes de transcribir valores.'),
    31:('L3','manual','motor_y_controlador','Manual Kelly KEB revisión 3.4, febrero de 2023; binario distinto de DOC-0008. Comparación técnica pendiente.'),
    32:('L4','manual','software_y_telemetria','Guía de la placa Keyestudio ESP32 KS0413; revisión de identificación.'),
    33:('L2','datasheet','cableado_y_protecciones','Ficha de relé RZ, archivo RZ03-1A4-D005. Verificar la variante usada.'),
    34:('L4','manual','software_y_telemetria','STM32 UM1907 Rev 7, mayo de 2026, 47 páginas. No sustituir automáticamente la Rev 3 del paquete anterior.'),
    35:('L2','manual','bateria_y_bms','Manual DALY Smart Active Balance BMS en inglés pese al nombre de archivo chino. Modelo exacto instalado pendiente.'),
    36:('L3','hoja_calculo','cableado_y_protecciones','Libro Fichas del paquete reciente. Hoja inventariada; contenido pendiente de análisis.'),
    37:('L3','plano_pcb','cableado_y_protecciones','PCB frontal, frente; requiere revisión visual.'),
    38:('L3','plano_pcb','cableado_y_protecciones','PCB frontal, reverso; requiere revisión visual.'),
    39:('L3','plano_pcb','cableado_y_protecciones','PCB trasera, frente; requiere revisión visual.'),
    40:('L3','plano_pcb','cableado_y_protecciones','PCB trasera, reverso; requiere revisión visual.'),
    41:('L1','informe_editable','integracion_del_kart','Word del informe de Bernardo y Ricardo. Texto extraído; equivalencia con el PDF DOC-0042 pendiente de cotejo.'),
    42:('L1','informe','integracion_del_kart','Informe de julio de 2026, curso 2025/26. Arquitectura STM32 + ESP32 con CAN. Síntesis inicial y pendientes documentados; no equivale a verificar el vehículo.')}
    coverage={2:'PDF 1,5; comienzo de 60',3:'PDF 1,4,47; verificación visual de 1',22:'PDF 1,3',23:'PDF 1 con verificación visual',24:'PDF 1,5',25:'PDF 1,7',26:'PDF 1,3; comienzo de 47',27:'PDF 1,3,67',42:'PDF 1,5,7,8,96,97,98; verificación visual de 5,97,98',15:'Imagen 1 de 5; texto y número de imágenes comprobados'}
    for n in [4,5,6,8,9,10,12,29,30,31,32,33,34,35]: coverage[n]='Identificación inicial en la primera página; lectura técnica pendiente'
    coverage[7]='Identificación inicial en páginas PDF 1-2'; coverage[11]='Identificación inicial en páginas PDF 1-2'
    dates={2:'2019-09',3:'2024-07 (portada; curso 24/25, contradicción pendiente)',22:'2024-02-12 (portada)',23:'2009-09-09',24:'2011-09',25:'2014-08',27:'2024-07-15 (portada)',42:'2026-07'}
    folder=CAT/'fichas'; folder.mkdir(exist_ok=True)
    index=['# Índice documental','', '42 documentos originales identificados. Las fichas enlazan a los archivos locales y registran el alcance de lectura; la extracción automática no implica análisis completo.','', '[Plan de lectura por lotes](../06_TRABAJO/PLAN_DE_LECTURA.md) · [Balance de recepción](RECEPCION_2026-09-28.md)','']
    for r in rows:
        n=int(r['id'][4:]); lote,tipo,sistema,summary=notes[n]
        r.update(lote=lote,tipo=tipo,sistema=sistema,resumen=summary)
        r['cobertura_lectura']=coverage.get(n,'Pendiente; únicamente inventario y/o extracción automática')
        r['estado_lectura']='parcial' if n in coverage else 'pendiente'
        if n in dates:r['fecha_documento']=dates[n]
        if n==42:r['configuracion']='Descrita en informe 2025/26; montaje actual por confirmar'
        if n==3:r['configuracion']='Curso 24/25 según título; fecha contradictoria'
        path=f"fichas/{r['id']}.md"
        meta=json.loads((ROOT/r['ruta_texto']).with_name('meta.json').read_text(encoding='utf-8'))
        body=[f"# {r['id']} · {r['titulo']}",'',summary,'',f"- Tipo: {tipo}. Lote: {lote}. Sistema: {sistema}.",f"- Fecha: {r['fecha_documento']}.",f"- Configuración: {r['configuracion']}.",f"- Formato: {r['formato']}. Páginas PDF: {r['paginas'] or 'no aplica'}.",f"- Extracción: {r['estado_extraccion']}.",f"- Lectura: {r['estado_lectura']}. Cobertura: {r['cobertura_lectura']}.",f"- Original: {link('abrir archivo','../../'+r['ruta_original'])}.",f"- SHA-256: `{r['sha256']}`."]
        if r['paginas_poco_texto']:body.append(f"- Páginas con poco texto: {r['paginas_poco_texto']}. Puede tratarse de páginas vacías, imágenes o planos; no significa automáticamente que necesiten OCR.")
        if r['hojas']:body.append('- Hojas: '+r['hojas']+'. Datos, fórmulas y gráficos pendientes de análisis.')
        if meta.get('diapositivas'):body.append(f"- Diapositivas: {meta['diapositivas']}. Texto extraído; diseño y objetos pendientes.")
        if meta.get('imagenes'):body.append(f"- Recursos gráficos en Word: {meta['imagenes']}. La extracción de texto no interpreta las imágenes.")
        body+=['','## Consulta eficiente','',f"Buscar por `{r['id']}` con el buscador local. El texto derivado está en `08_INDICE_GENERADO/texto/{r['sha256']}/`. Verificar cifras y tablas contra el original.",'','Ficha de recepción generada desde el catálogo; las síntesis técnicas se mantienen en `04_CONOCIMIENTO`.','']
        (folder/(r['id']+'.md')).write_text('\n'.join(body),encoding='utf-8')
    csv_write(CAT/'documentos.csv',rows,list(rows[0]))
    for lote,title in [('L1','Informe y presentación de 2025/26'),('L2','Batería, BMS y alimentación'),('L3','Controladores, sensores, planos y mediciones'),('L4','Placas y software'),('L5','Evolución histórica'),('pendiente','Formato pendiente')]:
        index += ['## '+lote+' · '+title,'','| Documento | Tipo | Páginas PDF | Lectura |','|---|---|---:|---|']
        for r in rows:
            if r['lote']==lote:index.append('| '+link(r['id']+' · '+r['titulo'],f"fichas/{r['id']}.md")+f" | {r['tipo']} | {r['paginas'] or '—'} | {r['estado_lectura']} |")
        index.append('')
    (CAT/'INDICE_DOCUMENTOS.md').write_text('\n'.join(index),encoding='utf-8')
    print('42 fichas e índice por lotes generados.')

if __name__=='__main__':main()
