"""Reproduce geometría C y glifos del firmware en PC; no es captura del LCD.

Entrada: JSONL emitido por dashboard_test.exe. Requiere Pillow.
"""
import argparse
import json
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]

def color565(rgb):
    r,g,b=(rgb>>16)&255,(rgb>>8)&255,rgb&255
    return ((r>>3)*255//31,(g>>2)*255//63,(b>>3)*255//31)

def render(scene, fonts):
    image=Image.new('RGB',(480,272),'black')
    for op in scene['ops']:
        x,y,w,h=op['x'],op['y'],op['w'],op['h']
        color=color565(op['color'])
        if op['font']<0:
            if w and h: ImageDraw.Draw(image).rectangle((x,y,x+w-1,y+h-1),fill=color)
            continue
        font=fonts[op['font']]
        layer=Image.new('RGBA',(w,h))
        cursor=0
        for char in op['text']:
            glyph=font['lookup'].get(ord(char),font['lookup'][63])
            offset,_,gw,gh,top,left,advance,*_=glyph
            if gw and gh:
                mask=Image.frombytes('L',(gw,gh),bytes(font['pixels'][offset:offset+gw*gh]))
                ink=Image.new('RGBA',(gw,gh),color+(255,));ink.putalpha(mask)
                layer.alpha_composite(ink,(cursor+left,font['baseline']-top))
            cursor+=advance
        image.paste(layer,(x,y),layer)
    return image

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('commands',type=Path);ap.add_argument('output',type=Path);a=ap.parse_args()
    a.output.mkdir(parents=True,exist_ok=True)
    fonts=json.loads((ROOT/'assets/dashboard/font_metrics.json').read_text())['fonts']
    for font in fonts: font['lookup']={g[1]:g for g in font['glyphs']}
    scenes=[json.loads(line) for line in a.commands.read_text(encoding='utf-8').splitlines()]
    gallery=Image.new('RGB',(984,7*250+24),'#687881')
    # Recorta solo franja reservada para comparar el área útil; PNG individuales son 480x272.
    for s in scenes:
        img=render(s,fonts)
        name=f"caso{s['scenario']}_tema{s['theme']}_pagina{s['page']}_rueda{s['wheel']}.png"
        img.save(a.output/name)
        if s['scenario']==2 and s['wheel']==0:
            gallery.paste(img.crop((0,48,480,272)),(8+s['theme']*488,24+s['page']*250))
    gallery.save(a.output/'galeria_claro_oscuro.png')
    pair=Image.new('RGB',(984,244),'#687881')
    for theme in (0,1):
        s=next(s for s in scenes if s['scenario']==3 and s['theme']==theme and s['page']==0)
        pair.paste(render(s,fonts).crop((0,48,480,272)),(8+theme*488,10))
    pair.save(a.output/'resumen_claro_oscuro.png')
    print(f'{len(scenes)} renders sintéticos; glifos y geometría del código C, sin LCD real.')

if __name__=='__main__':main()
