#include <gui/common/KartStatus.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/Color.hpp>
#include <cstring>
// Alfabeto propio 5x7, filas de bits; no altera fuentes generadas por Designer.
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
KartStatus::KartStatus() {std::memset(lines,0,sizeof(lines));setPosition(0,176,480,96);}
void KartStatus::setLabel(const char* text) {
    std::memset(lines,0,sizeof(lines));std::strncpy(lines[0],text,40);invalidate();
}
void KartStatus::update(const KartUiSnapshot& snapshot) {
    char next[5][41]={};Kart_FormatDisplay(&snapshot,next);
    if(std::memcmp(lines,next,sizeof(lines))!=0){std::memcpy(lines,next,sizeof(lines));invalidate();}
}
void KartStatus::draw(const touchgfx::Rect& area) const {
    using namespace touchgfx;
    Rect clip=area;translateRectToAbsolute(clip);
    HAL::lcd().fillRect(clip,Color::getColorFromRGB(0,0,0));
    for(unsigned row=0;row<5;row++)for(unsigned col=0;col<40 && lines[row][col];col++) {
        const char* found=std::strchr(symbols,lines[row][col]);
        if(!found)continue;
        const unsigned char* glyph=glyphs[found-symbols];
        for(unsigned y=0;y<7;y++)for(unsigned x=0;x<5;x++)if(glyph[y]&(16>>x)) {
            Rect pixel(col*12+1+x*2,row*19+2+y*2,2,2);translateRectToAbsolute(pixel);pixel&=clip;
            if(!pixel.isEmpty())HAL::lcd().fillRect(pixel,row==1?Color::getColorFromRGB(255,210,80):Color::getColorFromRGB(230,240,255));
        }
    }
}
