#include <gui/common/KartDashboard.hpp>
#include <touchgfx/Font.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>
#include <cstring>

namespace {
class DashboardFont : public touchgfx::Font {
public:
    explicit DashboardFont(uint8_t index):Font(kart_ui_fonts[index].height,kart_ui_fonts[index].baseline,0,0,8,1,2,2,'?','.'),index(index) {
        static_assert(sizeof(touchgfx::GlyphNode)==sizeof(KartUiGlyph),"Formato de glifo incompatible");
        for(unsigned i=0;i<kart_ui_fonts[index].count&&i<128;i++)std::memcpy(&glyphs[i],&kart_ui_fonts[index].glyphs[i],sizeof(glyphs[i]));
    }
    const touchgfx::GlyphNode* getGlyph(touchgfx::Unicode::UnicodeChar cp,const uint8_t*& data,uint8_t& bpp) const override {
        if(!cp)return nullptr;
        const KartUiGlyph* glyph=KartUIFont_Glyph(index,cp);
        data=kart_ui_fonts[index].pixels+glyph->offset;bpp=8;
        return &glyphs[glyph-kart_ui_fonts[index].glyphs];
    }
    int8_t getKerning(touchgfx::Unicode::UnicodeChar,const touchgfx::GlyphNode*) const override {return 0;}
private:
    uint8_t index;
    touchgfx::GlyphNode glyphs[128];
};
const touchgfx::Font* fontFor(uint8_t index){
    static DashboardFont small(KART_FONT_SMALL),body(KART_FONT_BODY),value(KART_FONT_VALUE),speed(KART_FONT_SPEED);
    const touchgfx::Font* fonts[]={&small,&body,&value,&speed};
    return fonts[index<KART_UI_FONT_COUNT?index:static_cast<uint8_t>(KART_FONT_BODY)];
}
touchgfx::colortype color(uint32_t rgb){return touchgfx::Color::getColorFromRGB((rgb>>16)&255,(rgb>>8)&255,rgb&255);}
}
KartDashboard::KartDashboard(){
    std::memset(&snapshot,0,sizeof(snapshot));snapshot.mode_applied=snapshot.mode_received=255;
    snapshot.can_age=snapshot.status_age=snapshot.bms_soc_age=snapshot.bms_temp_age=UINT32_MAX;
    snapshot.front_left_age=snapshot.front_right_age=UINT32_MAX;
    KartDashboard_Init(&state);setPosition(0,0,480,272);setTouchable(true);
    for(unsigned i=0;i<KART_UI_FONT_COUNT;i++)fontFor(i);
}
void KartDashboard::update(const KartUiSnapshot& next){
    if(std::memcmp(&snapshot,&next,sizeof(next))==0)return;
    snapshot=next;
    touchgfx::Rect body(0,48,480,180);invalidateRect(body);
}
void KartDashboard::paintRect(void* context,int x,int y,int w,int h,uint32_t rgb){
    if(w<=0||h<=0)return;
    const auto& ctx=*static_cast<PaintContext*>(context);
    touchgfx::Rect r(x,y,w,h);r&=ctx.clip;
    if(!r.isEmpty()){ctx.widget->translateRectToAbsolute(r);touchgfx::HAL::lcd().fillRect(r,color(rgb));}
}
void KartDashboard::paintText(void* context,int x,int y,int w,uint8_t font,uint32_t rgb,const char* text){
    const auto& ctx=*static_cast<PaintContext*>(context);
    const touchgfx::Font* f=fontFor(font);
    touchgfx::Rect rect(x,y,w,f->getHeight()),clipped=rect;clipped&=ctx.clip;if(clipped.isEmpty())return;
    clipped.x-=rect.x;clipped.y-=rect.y;ctx.widget->translateRectToAbsolute(rect);
    touchgfx::Unicode::UnicodeChar buffer[128];touchgfx::Unicode::fromUTF8(reinterpret_cast<const uint8_t*>(text),buffer,128);
    touchgfx::LCD::StringVisuals visuals;visuals.font=f;visuals.color=color(rgb);
    touchgfx::HAL::lcd().drawString(rect,clipped,visuals,buffer);
}
void KartDashboard::draw(const touchgfx::Rect& area) const {
    PaintContext ctx={this,area};const KartDashboardPainter painter={&ctx,paintRect,paintText};
    KartDashboard_Draw(&state,&snapshot,&painter);
}
void KartDashboard::handleClickEvent(const touchgfx::ClickEvent& event){
    const int type=event.getType()==touchgfx::ClickEvent::PRESSED?0:event.getType()==touchgfx::ClickEvent::RELEASED?1:2;
    const uint8_t action=KartDashboard_Touch(&state,type,event.getX(),event.getY());
    if(action==KART_ACTION_MODE)Kart_RequestNextMode();
    else if(action==KART_ACTION_RECORD)Kart_ToggleRecording();
    if(type==1){touchgfx::Rect all(0,48,480,224);invalidateRect(all);}
}
void KartDashboard::handleDragEvent(const touchgfx::DragEvent&){KartDashboard_Touch(&state,2,0,0);}
