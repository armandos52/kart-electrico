"""Dibuja en PNG la salida de tests/volcar_condiciones.cpp con la misma fuente 5x7 del frontal.

Reproduce la geometría de KartConditions (filas de 20 px, celdas de 12 px, puntos de 2x2) y de KartStatus
(filas de 19 px). Es una simulación en PC de lo que formatea el código, NO una captura del LCD de la F746.
Sin dependencias: PNG escrito con zlib.
"""
from pathlib import Path
import argparse, re, struct, sys, zlib

ROOT = Path(__file__).resolve().parents[1]
FONT = ROOT / 'TouchGFX/gui/include/gui/common/KartFont.h'
TONES = {0: (230, 240, 255), 1: (120, 180, 255), 2: (110, 230, 140), 3: (255, 210, 80), 4: (255, 110, 100)}
HINT = (120, 130, 150)
AMBER = (255, 210, 80)
WHITE = (230, 240, 255)


def load_font():
    text = FONT.read_text(encoding='utf-8')
    symbols = re.search(r'symbols\[\]="([^"]*)"', text).group(1)
    body = text[text.index('glyphs[][7]={'):]
    body = body[:body.index('};')]
    rows = [tuple(int(n) for n in g.split(',')) for g in re.findall(r'\{(\d+(?:,\d+){6})\}', body)]
    if len(rows) != len(symbols):
        raise SystemExit(f'Tabla de glifos incoherente: {len(rows)} glifos, {len(symbols)} símbolos')
    return {c: r for c, r in zip(symbols, rows)}


def write_png(path, width, height, pixels):
    raw = b''.join(b'\x00' + bytes(pixels[y * width * 3:(y + 1) * width * 3]) for y in range(height))

    def chunk(kind, data):
        body = kind + data
        return struct.pack('>I', len(data)) + body + struct.pack('>I', zlib.crc32(body) & 0xffffffff)

    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')
    Path(path).write_bytes(png)


class Canvas:
    def __init__(self, width, height):
        self.w, self.h = width, height
        self.px = bytearray(width * height * 3)

    def rect(self, x, y, w, h, color):
        for yy in range(max(y, 0), min(y + h, self.h)):
            for xx in range(max(x, 0), min(x + w, self.w)):
                i = (yy * self.w + xx) * 3
                self.px[i:i + 3] = bytes(color)

    def text(self, font, x, y, text, color):
        for col, ch in enumerate(text):
            glyph = font.get(ch)
            if not glyph:
                continue
            for gy, bits in enumerate(glyph):
                for gx in range(5):
                    if bits & (16 >> gx):
                        self.rect(x + col * 12 + gx * 2, y + gy * 2, 2, 2, color)


def parse(dump):
    scenarios, current = {}, None
    for line in dump.splitlines():
        if line.startswith('SCENARIO '):
            current = scenarios.setdefault(line.split(' ', 1)[1].strip(), {'cond': {}, 'main': {}})
        elif current is not None and line.startswith('COND '):
            _, row, tone, *rest = line.split(' ', 3)
            current['cond'][int(row)] = (int(tone), rest[0] if rest else '')
        elif current is not None and line.startswith('MAIN '):
            _, row, *rest = line.split(' ', 2)
            current['main'][int(row)] = rest[0] if rest else ''
    return scenarios


def render_conditions(font, data):
    canvas = Canvas(480, 224)
    for row, (tone, text) in sorted(data['cond'].items()):
        canvas.text(font, 1, row * 20 + 3, text, TONES.get(tone, WHITE))
    canvas.text(font, 1 + 27 * 12, 3, 'TOCAR: VOLVER', HINT)
    return canvas


def render_status(font, data):
    canvas = Canvas(480, 96)
    for row, text in sorted(data['main'].items()):
        canvas.text(font, 1, row * 19 + 2, text, AMBER if row == 1 else WHITE)
    canvas.text(font, 1 + 28 * 12, 2, 'TOCAR: COND', HINT)
    return canvas


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('dump', type=Path)
    parser.add_argument('out', type=Path)
    args = parser.parse_args()
    font = load_font()
    scenarios = parse(args.dump.read_text(encoding='utf-8-sig'))
    args.out.mkdir(parents=True, exist_ok=True)
    for name, data in scenarios.items():
        for kind, canvas in (('condiciones', render_conditions(font, data)), ('estado', render_status(font, data))):
            write_png(args.out / f'{name}_{kind}.png', canvas.w, canvas.h, canvas.px)
    print(f'{len(scenarios)} escenarios, {2 * len(scenarios)} imágenes en {args.out}')


if __name__ == '__main__':
    sys.exit(main())
