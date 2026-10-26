#include <gui/common/KartText.hpp>
#include <gui/common/KartFont.h>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>
void kartDrawText(const touchgfx::Drawable& owner,const touchgfx::Rect& clip,int x,int y,const char* text,touchgfx::colortype color) {
    using namespace touchgfx;
    for(unsigned col=0;text[col];col++) {
        const unsigned char* glyph=kartfont::find(text[col]);
        if(!glyph)continue;
        kartfont::forEachRun(glyph,[&](unsigned gy,unsigned gx,unsigned length) {
            Rect run(x+col*12+gx*2,y+gy*2,length*2,2);
            owner.translateRectToAbsolute(run);run&=clip;
            if(!run.isEmpty())HAL::lcd().fillRect(run,color);
        });
    }
}
