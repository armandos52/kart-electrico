#include "kart_ui_font.h"
const KartUiGlyph* KartUIFont_Glyph(uint8_t font,uint16_t cp) {
    if(font>=KART_UI_FONT_COUNT)font=KART_FONT_BODY;
    const KartUiFont* f=&kart_ui_fonts[font];
    for(unsigned i=0;i<f->count;i++)if(f->glyphs[i].unicode==cp)return &f->glyphs[i];
    return &f->glyphs['?'-32];
}
uint16_t KartUIFont_Next(const char** text) {
    const unsigned char* s=(const unsigned char*)*text;
    uint16_t cp=*s;
    if(!cp)return 0;
    if(cp>=0xc2 && cp<=0xdf && s[1]>=0x80 && s[1]<=0xbf){cp=(uint16_t)(((cp&31)<<6)|(s[1]&63));*text+=2;}
    else if(cp>=0xe0 && cp<=0xef && s[1]>=0x80 && s[1]<=0xbf && s[2]>=0x80 && s[2]<=0xbf){cp=(uint16_t)(((cp&15)<<12)|((s[1]&63)<<6)|(s[2]&63));*text+=3;}
    else {(*text)++;if(cp>=128)cp='?';}
    return cp;
}
int KartUIFont_Width(uint8_t font,const char* text) {
    int width=0;uint16_t cp;
    while((cp=KartUIFont_Next(&text))!=0)width+=KartUIFont_Glyph(font,cp)->advance;
    return width;
}
