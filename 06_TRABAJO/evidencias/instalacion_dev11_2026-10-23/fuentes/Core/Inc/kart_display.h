#ifndef KART_DISPLAY_H
#define KART_DISPLAY_H
#include "kart_frontend.h"
#ifdef __cplusplus
extern "C" {
#endif
const char* Kart_ModeName(uint8_t mode);
void Kart_FormatDisplay(const KartUiSnapshot* s,char lines[5][41]);
#ifdef __cplusplus
}
#endif
#endif
