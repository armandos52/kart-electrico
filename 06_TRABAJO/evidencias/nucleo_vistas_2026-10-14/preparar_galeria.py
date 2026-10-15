"""Compone capturas originales del ILI9341; requiere Pillow. No dibuja la GUI."""
from pathlib import Path
import hashlib
import json
import zlib
from PIL import Image, ImageDraw, ImageFont

ev = Path(__file__).resolve().parent
captures = ev/'capturas_finales'
records = json.loads((captures/'manifest.json').read_text())
assert [r['scene'] for r in records] == list(range(10))
captions = [
    ('Reposo', 'N, 0 km/h, START apagado; modo aplicado desconocido.'),
    ('Precarga', 'START activo; preparando reles virtuales; APL:---.'),
    ('Marcha DIF', 'D, 24 km/h; grabacion simulada de una fila.'),
    ('Marcha BLQ', 'D, 18 km/h; BLQ solicitado, recibido y aplicado.'),
    ('Marcha ESC', 'D, 22 km/h; ESC solicitado, recibido y aplicado.'),
    ('Limite de reversa', 'R, 20 km/h; aviso de limite de propulsion.'),
    ('Frenado', 'D, 0 km/h; freno activo y motivo FRENO.'),
    ('Perdida de CAN', 'Caducidad inducida: velocidad -- y marcha N.'),
    ('Error de SD', 'FATFS simulado falla al montar: SD:ERROR E:1.'),
    ('Cambio rechazado', 'Solicitud de modo con D: exige N y conserva DIF.'),
]
gallery = Image.new('RGB', (704, 1600), '#eef2f5')
draw = ImageDraw.Draw(gallery)
fonts = Path('C:/Windows/Fonts')
title_font = ImageFont.truetype(str(fonts/'segoeuib.ttf'), 24)
label_font = ImageFont.truetype(str(fonts/'segoeuib.ttf'), 15)
body_font = ImageFont.truetype(str(fonts/'segoeui.ttf'), 13)
draw.text((24,18), 'KART / ENS-0016', font=title_font, fill='#14242e')
draw.text((24,53), '10 capturas leidas del LCD ILI9341 | NUCLEO-F411RE | 14-10-2026', font=body_font, fill='#334b59')
draw.text((24,74), 'Banco v0.3. Datos del kart sinteticos. No son fotografias ni pruebas de marcha.', font=body_font, fill='#334b59')
md = ['# Capturas del banco visual · ENS-0016', '',
      '14-10-2026 · NUCLEO-F411RE + ILI9341 · firmware de banco v0.3.', '',
      '**Origen:** lectura real de la memoria gráfica del LCD (GRAM), 320×240, RGB565. '
      'La interfaz se dibuja en la placa; el ordenador convierte esos píxeles a PNG. '
      'Son capturas digitales, no fotografías del cristal. Todos los datos del kart son sintéticos; '
      'no validan motores, CAN eléctrico, sensores, SD física ni TouchGFX en F746.', '',
      '[Ensayo y límites](../../ensayos/ENS-0016_vistas_frontal_nucleo.md) · '
      '[Galería conjunta PNG](galeria.png) · [Manifiesto de captura](capturas_finales/manifest.json)', '',
      '**Uso:** pulsar y soltar B1 azul avanza una escena; de la 10 vuelve a la 1. '
      'Mantenerlo pulsado no repite. START mostrado pertenece al escenario simulado.', '']
verification = []
for i, ((title, caption), record) in enumerate(zip(captions, records)):
    stem = f'escena_{i+1:02d}'
    raw = (captures/(stem+'.rgb565')).read_bytes()
    png = captures/(stem+'.png')
    assert len(raw) == 153600
    assert f'{zlib.crc32(raw):08X}' == record['crc32']
    assert hashlib.sha256(raw).hexdigest() == record['sha256_rgb565']
    assert hashlib.sha256(png.read_bytes()).hexdigest() == record['sha256_png']
    im = Image.open(png).convert('RGB')
    assert im.size == (320,240)
    # Comprobacion independiente de conversion: cada pixel PNG coincide con RGB565.
    expected = []
    for j in range(0,len(raw),2):
        c = int.from_bytes(raw[j:j+2],'little');r,g,b = c>>11,(c>>5)&63,c&31
        expected.append(((r*255+15)//31,(g*255+31)//63,(b*255+15)//31))
    # Expansion por repeticion de bits puede diferir 1 de escalado aritmetico.
    error = max(abs(a-b) for actual,target in zip(im.get_flattened_data(),expected) for a,b in zip(actual,target))
    assert error <= 1
    x,y = 24+(i%2)*336,110+(i//2)*292
    draw.text((x,y),f'{i+1:02d} / {title}',font=label_font,fill='#14242e')
    gallery.paste(im,(x,y+27))
    # Version ampliada sin interpolacion, para insertar en el informe.
    im.resize((960,720),Image.Resampling.NEAREST).save(captures/(stem+'_3x.png'))
    md.extend([f'## {i+1:02d} · {title}', '', caption+' **Valores sintéticos.**', '',
               f'![{title}](capturas_finales/{stem}.png)', '',
               f'[PNG 960×720 sin interpolación](capturas_finales/{stem}_3x.png) · '
               f'[Datos y CRC](capturas_finales/{stem}.json) · [RGB565 original](capturas_finales/{stem}.rgb565)', ''])
    verification.append(dict(scene=i,bytes=len(raw),crc_ok=True,png_rgb565_max_error=error))
draw.text((24,1570),'Fuente: GRAM real / CRC32 verificado en las 10 transferencias. Escala original en esta galeria.',font=body_font,fill='#334b59')
gallery.save(ev/'galeria.png')
(ev/'GALERIA.md').write_text('\n'.join(md)+'\n',encoding='utf-8')
(ev/'verificacion_capturas.json').write_text(json.dumps(verification,indent=2)+'\n',encoding='utf-8')
print('10 capturas verificadas; galeria, ampliaciones 3x y pies de figura conservados.')
