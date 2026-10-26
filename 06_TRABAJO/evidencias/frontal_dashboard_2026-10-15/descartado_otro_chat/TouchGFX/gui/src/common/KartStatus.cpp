#include <gui/common/KartStatus.hpp>
#include <gui/common/KartFont.h>
#include <gui/common/KartText.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/Color.hpp>
#include <cstring>
KartStatus::KartStatus() : clickCallback(0) {std::memset(lines,0,sizeof(lines));setPosition(0,176,480,96);}
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
        const unsigned char* glyph=kartfont::find(lines[row][col]);
        if(!glyph)continue;
        for(unsigned y=0;y<7;y++)for(unsigned x=0;x<5;x++)if(glyph[y]&(16>>x)) {
            Rect pixel(col*12+1+x*2,row*19+2+y*2,2,2);translateRectToAbsolute(pixel);pixel&=clip;
            if(!pixel.isEmpty())HAL::lcd().fillRect(pixel,row==1?Color::getColorFromRGB(255,210,80):Color::getColorFromRGB(230,240,255));
        }
    }
    // Aviso del toque que abre «Condiciones de marcha»; solo en la instancia con callback.
    if(clickCallback && clickCallback->isValid())kartDrawText(*this,clip,1+28*12,2,"TOCAR: COND",Color::getColorFromRGB(120,130,150));
}
void KartStatus::handleClickEvent(const touchgfx::ClickEvent& event) {
    if(event.getType()==touchgfx::ClickEvent::RELEASED && clickCallback && clickCallback->isValid())clickCallback->execute();
}
