#ifndef KART_DISPLAY_H
#define KART_DISPLAY_H
#include "kart_frontend.h"
#ifdef __cplusplus
extern "C" {
#endif
// Página «Condiciones de marcha»: 11 filas de hasta 40 caracteres y un tono por fila.
#define KART_CONDITION_ROWS 11
enum { KART_TONE_TEXT=0,KART_TONE_HEAD=1,KART_TONE_OK=2,KART_TONE_WARN=3,KART_TONE_FAIL=4 };
const char* Kart_ModeName(uint8_t mode);
void Kart_FormatDisplay(const KartUiSnapshot* s,char lines[5][41]);
void Kart_FormatConditions(const KartUiSnapshot* s,char lines[KART_CONDITION_ROWS][41],uint8_t tones[KART_CONDITION_ROWS]);
#ifdef __cplusplus
}
#endif
#endif
