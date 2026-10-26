#ifndef KART_DASHBOARD_H
#define KART_DASHBOARD_H
#include "kart_frontend.h"
#include "kart_ui_font.h"
#ifdef __cplusplus
extern "C" {
#endif
enum {KART_PAGE_HOME=0,KART_PAGE_INPUTS,KART_PAGE_DRIVE,KART_PAGE_WHEELS,KART_PAGE_CAN,KART_PAGE_SD,KART_PAGE_BMS,KART_PAGE_COUNT};
enum {KART_ACTION_NONE=0,KART_ACTION_MODE=1,KART_ACTION_RECORD=2};
typedef struct {uint8_t page,light,wheel;int16_t pressed,press_x,press_y;} KartDashboardState;
// Coordenadas físicas 480x272. Ninguna acción ni dibujo en y<48.
typedef struct {
    void* context;
    void (*rect)(void*,int,int,int,int,uint32_t);
    void (*text)(void*,int,int,int,uint8_t,uint32_t,const char*);
} KartDashboardPainter;
void KartDashboard_Init(KartDashboardState* state);
uint8_t KartDashboard_Touch(KartDashboardState* state,int event,int x,int y);
// event: 0 presión, 1 liberación, 2 cancelar, 3 movimiento.
// Efectos solo al liberar en la misma zona; tolerancia de 8 px desde la presión inicial.
void KartDashboard_Draw(const KartDashboardState* state,const KartUiSnapshot* snapshot,const KartDashboardPainter* painter);
const char* KartDashboard_PageName(uint8_t page);
#ifdef __cplusplus
}
#endif
#endif
