#include "kart_display.h"
#include <stdio.h>
const char* Kart_ModeName(uint8_t mode) {return mode==0?"DIF":mode==1?"ESC":mode==2?"BLQ":"---";}
void Kart_FormatDisplay(const KartUiSnapshot* s,char lines[5][41]) {
    static const char* causes[]={"TRACCION PERMITIDA","SIN MANDO CAN","CAN CADUCADO","MANDO INVALIDO","START APAGADO",
        "PREPARANDO RELES","CARGADOR CONECTADO","REARME: N FRENO PEDAL LIBRE","FRENO","PEDAL LIBRE","NEUTRO",
        "PARAR PARA CAMBIAR SENTIDO","PARAR EN N PARA CAMBIAR MODO","VELOCIDAD INVALIDA","REVISAR HALL","MPU NO DISPONIBLE",
        "ENTRADAS FRONTALES INVALIDAS","LIMITE R 20 KM/H","FALLO SALIDA","FALLO CAN","CONFIGURACION INVALIDA"};
    static const char* notices[]={"","MODO: PONER N","MODO: PARAR Y SOLTAR PEDAL","MODO: SIN CAN RECIENTE","MODO: REVISAR ENTRADAS"};
    snprintf(lines[0],41,"SOL:%s REC:%s APL:%s",Kart_ModeName(s->mode_requested),Kart_ModeName(s->mode_received),Kart_ModeName(s->mode_applied));
    const char* reason=!s->feedback_fresh?"SIN DATOS CAN RECIENTES":!s->status_fresh?"ESP SIN ESTADO RECIENTE":s->inhibit<21?causes[s->inhibit]:"ESTADO DESCONOCIDO";
    if(s->ui_notice>0 && s->ui_notice<5)reason=notices[s->ui_notice];
    snprintf(lines[1],41,"%s",reason);
    snprintf(lines[2],41,"CAN:%s MPU:%s FRENO:%s R:20KM/H",s->feedback_fresh?"OK":"--",s->status_fresh?(s->backend_flags&KART_STATUS_YAW_VALID?"OK":"NO"):"--",s->brake?"SI":"NO");
    const char* sd=s->sd_state==KART_SD_RECORDING?"GRABA":s->sd_state==KART_SD_ERROR?"ERROR":s->sd_state==KART_SD_OPENING?"ABRE":"PAUSA";
    if(s->soc_valid && s->temp_valid)snprintf(lines[3],41,"BMS:%u%% %dC SD:%s E:%u",s->soc_pct,s->battery_temp_c,sd,s->sd_error);
    else snprintf(lines[3],41,"BMS:-- SD:%s E:%u",sd,s->sd_error);
    snprintf(lines[4],41,"STM V1.3-DEV2  SD FILAS:%lu",(unsigned long)s->sd_rows);
}
