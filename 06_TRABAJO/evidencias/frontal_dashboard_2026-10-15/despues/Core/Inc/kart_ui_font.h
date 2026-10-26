#ifndef KART_UI_FONT_H
#define KART_UI_FONT_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
enum {KART_FONT_SMALL=0,KART_FONT_BODY=1,KART_FONT_VALUE=2,KART_FONT_SPEED=3,KART_UI_FONT_COUNT=4};
#pragma pack(push,2)
typedef struct {
    uint32_t offset; uint16_t unicode;
    uint8_t width,height,top; int8_t left;
    uint8_t advance,kerning_pos,kerning_size,flags;
} KartUiGlyph;
#pragma pack(pop)
typedef struct {const KartUiGlyph* glyphs;const uint8_t* pixels;uint16_t count,height,baseline;} KartUiFont;
extern const KartUiFont kart_ui_fonts[KART_UI_FONT_COUNT];
const KartUiGlyph* KartUIFont_Glyph(uint8_t font,uint16_t codepoint);
uint16_t KartUIFont_Next(const char** text);
int KartUIFont_Width(uint8_t font,const char* text);
#ifdef __cplusplus
}
#endif
#endif
