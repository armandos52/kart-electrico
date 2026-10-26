#ifndef KART_FONT_H
#define KART_FONT_H
#include <cstring>
// Alfabeto propio 5x7, filas de bits; no altera fuentes generadas por Designer.
// Sin dependencias de TouchGFX para poder comprobarlo en el PC.
namespace kartfont {
static const char symbols[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-.%/ ";
static const unsigned char glyphs[][7]={
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,25,21,19,19,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
 {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},
 {14,17,17,14,17,17,14},{14,17,17,15,1,1,14},
 {0,4,4,0,4,4,0},{0,0,0,31,0,0,0},{0,0,0,0,0,6,6},{24,25,2,4,8,19,3},{1,2,2,4,8,8,16},{0,0,0,0,0,0,0}
};
static const unsigned glyphCount=sizeof(glyphs)/sizeof(glyphs[0]);
// Devuelve el glifo del caracter o nullptr si no esta en el alfabeto (el texto lo omite).
inline const unsigned char* find(char c) {
    if(!c)return nullptr;
    const char* found=std::strchr(symbols,c);
    return found?glyphs[found-symbols]:nullptr;
}
// Recorre los tramos horizontales de puntos encendidos de un glifo: f(fila,columna,longitud).
// Pinta los mismos puntos que el bucle punto a punto con menos llamadas de relleno.
template <typename F>
inline void forEachRun(const unsigned char* glyph,F f) {
    for(unsigned y=0;y<7;y++) {
        unsigned x=0;
        while(x<5) {
            if(glyph[y]&(16>>x)) {
                unsigned n=1;
                while(x+n<5 && (glyph[y]&(16>>(x+n))))n++;
                f(y,x,n);x+=n;
            } else x++;
        }
    }
}
}
#endif
