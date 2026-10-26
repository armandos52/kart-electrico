#include "kart_display.h"
#include <stdio.h>
#include <string.h>
const char* Kart_ModeName(uint8_t mode) {return mode==0?"DIF":mode==1?"ESC":mode==2?"BLQ":"---";}
void Kart_FormatDisplay(const KartUiSnapshot* s,char lines[5][41]) {
    static const char* causes[]={"TRACCION PERMITIDA","SIN MANDO CAN","CAN CADUCADO","MANDO INVALIDO","START APAGADO",
        "PREPARANDO RELES","CARGADOR CONECTADO","REARME: N FRENO PEDAL LIBRE","FRENO","PEDAL LIBRE","NEUTRO",
        "PARAR PARA CAMBIAR SENTIDO","PARAR EN N PARA CAMBIAR MODO","VELOCIDAD INVALIDA","REVISAR HALL","MPU NO DISPONIBLE",
        "ENTRADAS FRONTALES INVALIDAS","LIMITE R ACTIVO","FALLO SALIDA","FALLO CAN","CONFIGURACION INVALIDA"};
    static const char* notices[]={"","MODO: PONER N","MODO: PARAR Y SOLTAR PEDAL","MODO: SIN CAN RECIENTE","MODO: REVISAR ENTRADAS"};
    snprintf(lines[0],41,"SOL:%s REC:%s APL:%s",Kart_ModeName(s->mode_requested),Kart_ModeName(s->mode_received),Kart_ModeName(s->mode_applied));
    const char* reason=!s->feedback_fresh?"SIN DATOS CAN RECIENTES":!s->status_fresh?"ESP SIN ESTADO RECIENTE":s->inhibit<21?causes[s->inhibit]:"ESTADO DESCONOCIDO";
    // La causa nunca se sustituye por un aviso de interaccion del desarrollador.
    snprintf(lines[1],41,"%s",reason);
    // 0x124 valida version/formato al recibir, pero no comunica un umbral numerico.
    // INACT significa que el ESP no anuncia actuacion, no que falte la proteccion.
    const char* reverse=!s->status_seen || !s->status_fresh || !s->feedback_fresh?"--":
        (s->backend_flags&KART_STATUS_REVERSE_LIMIT)?"LIM":"INACT";
    snprintf(lines[2],41,"CAN:%s MPU:%s FRENO:%s R:%s",s->feedback_fresh?"OK":"--",s->status_fresh?(s->backend_flags&KART_STATUS_YAW_VALID?"OK":"NO"):"--",s->brake?"SI":"NO",reverse);
    const char* sd=s->sd_state==KART_SD_RECORDING?"GRABA":s->sd_state==KART_SD_ERROR?"ERROR":s->sd_state==KART_SD_OPENING?"ABRE":"PAUSA";
    if(s->soc_valid && s->temp_valid)snprintf(lines[3],41,"BMS:%u%% %dC SD:%s E:%u",s->soc_pct,s->battery_temp_c,sd,s->sd_error);
    else snprintf(lines[3],41,"BMS:-- SD:%s E:%u",sd,s->sd_error);
    // Una sola version para pantalla, CSV y nombre de ELF/HEX. El widget usa mayusculas.
    const char* release=strchr(KART_FRONTEND_VERSION,'-');
    release=release?release+1:KART_FRONTEND_VERSION;
    char version[sizeof(KART_FRONTEND_VERSION)];
    unsigned i=0;
    do {
        const char c=release[i];version[i]=(c>='a' && c<='z')?(char)(c-'a'+'A'):c;
    } while(release[i++]);
    if(s->ui_notice>0 && s->ui_notice<5)
        snprintf(lines[4],41,"STM %s %s",version,notices[s->ui_notice]);
    else snprintf(lines[4],41,"STM %s SD FILAS:%lu",version,(unsigned long)s->sd_rows);
}
