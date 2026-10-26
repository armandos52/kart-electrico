#include <gui/common/KartConditions.hpp>
#include <gui/common/KartText.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <touchgfx/Color.hpp>
#include <cstring>
// 11 filas de 20 px en 224 px útiles: 220 px, sin tocar la franja superior dañada (PROB-0004).
static const int rowPitch=20;
static const int rowTextOffset=3;
static const int hintColumn=27;
KartConditions::KartConditions() : closeCallback(0) {
    std::memset(lines,0,sizeof(lines));std::memset(tones,0,sizeof(tones));
    setPosition(0,48,480,224);
}
void KartConditions::update(const KartUiSnapshot& snapshot) {
    char next[KART_CONDITION_ROWS][41];uint8_t nextTones[KART_CONDITION_ROWS];
    Kart_FormatConditions(&snapshot,next,nextTones);
    // Solo se vuelve a pintar la franja de cada fila que cambia.
    for(unsigned row=0;row<KART_CONDITION_ROWS;row++) {
        if(std::memcmp(lines[row],next[row],41)==0 && tones[row]==nextTones[row])continue;
        std::memcpy(lines[row],next[row],41);tones[row]=nextTones[row];
        touchgfx::Rect band(0,static_cast<int16_t>(row*rowPitch),480,rowPitch);
        invalidateRect(band);
    }
}
static touchgfx::colortype toneColor(uint8_t tone) {
    using touchgfx::Color;
    switch(tone) {
        case KART_TONE_HEAD: return Color::getColorFromRGB(120,180,255);
        case KART_TONE_OK: return Color::getColorFromRGB(110,230,140);
        case KART_TONE_WARN: return Color::getColorFromRGB(255,210,80);
        case KART_TONE_FAIL: return Color::getColorFromRGB(255,110,100);
        default: return Color::getColorFromRGB(230,240,255);
    }
}
void KartConditions::draw(const touchgfx::Rect& area) const {
    using namespace touchgfx;
    Rect clip=area;translateRectToAbsolute(clip);
    HAL::lcd().fillRect(clip,Color::getColorFromRGB(0,0,0));
    for(unsigned row=0;row<KART_CONDITION_ROWS;row++) {
        Rect band(0,static_cast<int16_t>(row*rowPitch),480,rowPitch);band&=area;
        if(band.isEmpty() || !lines[row][0])continue;
        kartDrawText(*this,clip,1,static_cast<int>(row)*rowPitch+rowTextOffset,lines[row],toneColor(tones[row]));
    }
    // Aviso fijo de cierre en la fila de título.
    Rect title(0,0,480,rowPitch);title&=area;
    if(!title.isEmpty())kartDrawText(*this,clip,1+hintColumn*12,rowTextOffset,"TOCAR: VOLVER",Color::getColorFromRGB(120,130,150));
}
void KartConditions::handleClickEvent(const touchgfx::ClickEvent& event) {
    if(event.getType()==touchgfx::ClickEvent::RELEASED && closeCallback && closeCallback->isValid())closeCallback->execute();
}
