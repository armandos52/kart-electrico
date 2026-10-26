"""Genera glifos AA de DejaVu Sans para el dashboard; no usa Designer ni hardware.

Requiere Pillow. Se conserva TTF/licencia junto al generador para reproducibilidad.
Los píxeles son alpha8, filas contiguas; métricas enteras y sin kerning.
"""
from pathlib import Path
import hashlib
import json
import struct
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / 'assets/dashboard'
SIZES = (12, 13, 20, 32)
CHARS = sorted(set(range(32, 127)) | {161, 176, 183, 191, 193, 201, 205, 209, 211, 218, 220,
                                    225, 233, 237, 241, 243, 250, 252})

def license_text(data):
    count = struct.unpack_from('>H', data, 4)[0]
    for i in range(count):
        tag, _, offset, _ = struct.unpack_from('>4sIII', data, 12 + i*16)
        if tag == b'name':
            _, n, strings = struct.unpack_from('>HHH', data, offset)
            result = {}
            for j in range(n):
                platform, _, _, ident, length, pos = struct.unpack_from('>6H', data, offset+6+j*12)
                if platform == 3 and ident in (0, 13, 14):
                    result[ident] = data[offset+strings+pos:offset+strings+pos+length].decode('utf-16-be')
            return '\n\n'.join(result[k] for k in sorted(result)) + '\n'
    raise ValueError('No hay tabla de atribución en TTF')

def main():
    path = ASSETS / 'DejaVuSans.ttf'
    data = path.read_bytes()
    assert len(CHARS) <= 128
    (ASSETS / 'LICENSE-DejaVu.txt').write_text(license_text(data), encoding='utf-8')
    out = ['/* Generado por tools/generar_fuente_dashboard.py; DejaVu Sans, licencia en assets/dashboard. */',
           '#include "kart_ui_font.h"']
    meta = {'font_sha256': hashlib.sha256(data).hexdigest(), 'fonts': []}
    for size in SIZES:
        font = ImageFont.truetype(str(path), size)
        ascent, descent = font.getmetrics()
        nodes, pixels = [], []
        for cp in CHARS:
            c = chr(cp)
            left, top, right, bottom = font.getbbox(c, anchor='ls')
            w, h = right-left, bottom-top
            if c == ' ': w = h = 0
            advance = int(font.getlength(c)+0.5)
            offset = len(pixels)
            if w and h:
                img = Image.new('L', (w,h))
                ImageDraw.Draw(img).text((-left,-top), c, font=font, fill=255, anchor='ls')
                pixels.extend(img.tobytes())
            glyph_top = -top if h else 0
            flags = ((glyph_top & 0x300) >> 3)
            nodes.append((offset,cp,w,h,glyph_top & 255,left,advance,0,0,flags))
        out += [f'static const KartUiGlyph glyphs_{size}[] = {{']
        out += ['  {'+','.join(map(str,node))+'},' for node in nodes]
        out += ['};', f'static const uint8_t pixels_{size}[] = {{']
        out += ['  '+','.join(map(str,pixels[i:i+24]))+',' for i in range(0,len(pixels),24)]
        out += ['};']
        meta['fonts'].append({'size':size,'height':ascent+descent,'baseline':ascent,'glyphs':nodes,'pixels':pixels})
    out += ['const KartUiFont kart_ui_fonts[KART_UI_FONT_COUNT] = {']
    for f in meta['fonts']:
        size=f['size']
        out += [f'  {{glyphs_{size},pixels_{size},{len(CHARS)},{f["height"]},{f["baseline"]}}},']
    out += ['};','']
    (ROOT/'Core/Src/kart_ui_font_data.c').write_text('\n'.join(out),encoding='ascii')
    (ASSETS/'font_metrics.json').write_text(json.dumps(meta,separators=(',',':')),encoding='utf-8')
    print('Fuentes generadas: '+str(sum(len(f['pixels']) for f in meta['fonts']))+' bytes de alpha8')

if __name__ == '__main__': main()
