#include "kart_display.h"
#include <stdio.h>
#include <string.h>
static const char* const causes[]={"TRACCION PERMITIDA","SIN MANDO CAN","CAN CADUCADO","MANDO INVALIDO","START APAGADO",
    "PREPARANDO RELES","CARGADOR CONECTADO","REARME: N FRENO PEDAL LIBRE","FRENO","PEDAL LIBRE","NEUTRO",
    "PARAR PARA CAMBIAR SENTIDO","PARAR EN N PARA CAMBIAR MODO","VELOCIDAD INVALIDA","REVISAR HALL","MPU NO DISPONIBLE",
    "ENTRADAS FRONTALES INVALIDAS","LIMITE R ACTIVO","FALLO SALIDA","FALLO CAN","CONFIGURACION INVALIDA"};
const char* Kart_ModeName(uint8_t mode) {return mode==0?"DIF":mode==1?"ESC":mode==2?"BLQ":"---";}
static const char* inhibit_reason(const KartUiSnapshot* s) {
    return !s->feedback_fresh?"SIN DATOS CAN RECIENTES":!s->status_fresh?"ESP SIN ESTADO RECIENTE":s->inhibit<21?causes[s->inhibit]:"ESTADO DESCONOCIDO";
}
void Kart_FormatDisplay(const KartUiSnapshot* s,char lines[5][41]) {
    static const char* notices[]={"","MODO: PONER N","MODO: PARAR Y SOLTAR PEDAL","MODO: SIN CAN RECIENTE","MODO: REVISAR ENTRADAS"};
    snprintf(lines[0],41,"SOL:%s REC:%s APL:%s",Kart_ModeName(s->mode_requested),Kart_ModeName(s->mode_received),Kart_ModeName(s->mode_applied));
    // La causa nunca se sustituye por un aviso de interaccion del desarrollador.
    snprintf(lines[1],41,"%s",inhibit_reason(s));
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

// Mayor texto: «MAS DE 99 S» (11 caracteres).
static void age_text(char out[12],uint32_t age) {
    if(age==UINT32_MAX)snprintf(out,12,"NUNCA");
    else if(age>99999u)snprintf(out,12,"MAS DE 99 S");
    else snprintf(out,12,"%lu MS",(unsigned long)age);
}
static void add_token(char* list,size_t size,const char* token) {
    const size_t used=strlen(list);
    if(used<size)snprintf(list+used,size-used," %s",token);
}
// Condiciones que solo hacen falta para entrar en D/R: con marcha ya engranada se muestran sin alarma.
static uint8_t entry_tone(const KartUiSnapshot* s,uint8_t met) {
    return (uint8_t)(met?KART_TONE_OK:(s->gear==1||s->gear==2)?KART_TONE_TEXT:KART_TONE_FAIL);
}
void Kart_FormatConditions(const KartUiSnapshot* s,char lines[KART_CONDITION_ROWS][41],uint8_t tones[KART_CONDITION_ROWS]) {
    // Mayor lista de faltas: " ENTR CAN FRENO PEDAL PARADO" (28 caracteres).
    char age[12],missing[30];
    const uint8_t cond=s->drive_cond;
    memset(lines,0,(size_t)KART_CONDITION_ROWS*41u);memset(tones,0,KART_CONDITION_ROWS);

    snprintf(lines[0],41,"CONDICIONES DE MARCHA");tones[0]=KART_TONE_HEAD;

    // Resultado para D/R, con el mismo criterio que el control: entradas, CAN y selector, y despues freno, pedal y quietud.
    if(s->gear==1 || s->gear==2) {
        snprintf(lines[1],41,"D/R: %c ACTIVA",s->gear==1?'D':'R');tones[1]=KART_TONE_OK;
    } else if(s->drive_input && s->reverse_input) {
        snprintf(lines[1],41,"D/R: SELECTOR INVALIDO D Y R");tones[1]=KART_TONE_FAIL;
    } else if(!s->drive_input && !s->reverse_input) {
        snprintf(lines[1],41,"D/R: SELECTOR EN N SIN SOLICITUD");tones[1]=KART_TONE_TEXT;
    } else {
        missing[0]=0;
        if(!(cond&KART_DRIVE_COND_INPUTS))add_token(missing,sizeof(missing),"ENTR");
        if(!(cond&KART_DRIVE_COND_CAN))add_token(missing,sizeof(missing),"CAN");
        if(!(cond&KART_DRIVE_COND_BRAKE))add_token(missing,sizeof(missing),"FRENO");
        if(!(cond&KART_DRIVE_COND_PEDAL))add_token(missing,sizeof(missing),"PEDAL");
        if(!(cond&KART_DRIVE_COND_STOPPED))add_token(missing,sizeof(missing),"PARADO");
        if(missing[0]) {snprintf(lines[1],41,"D/R: FALTA%s",missing);tones[1]=KART_TONE_FAIL;}
        else {snprintf(lines[1],41,"D/R: CONDICIONES CUMPLIDAS");tones[1]=KART_TONE_OK;}
    }

    // Pedal: ADC fisico, valor normalizado y umbral usado por el codigo (PROB-0012).
    if(!(s->input_flags&KART_INPUT_ACCEL_OK)) {
        snprintf(lines[2],41,"PEDAL LIBRE: -- ADC INVALIDO");tones[2]=KART_TONE_FAIL;
    } else {
        // Valores validos: ADC hasta 1961 y normalizado hasta 4095; se acota a 4 cifras para no truncar.
        const unsigned adc=s->accel_adc>9999u?9999u:s->accel_adc,norm=s->accel_raw>9999u?9999u:s->accel_raw;
        snprintf(lines[2],41,"PEDAL LIBRE:%s ADC:%u NORM:%u MAX:%u",s->pedal_free?"SI":"NO",adc,norm,(unsigned)KART_PEDAL_FREE_MAX);
        tones[2]=entry_tone(s,s->pedal_free);
    }

    snprintf(lines[3],41,"FRENO PISADO: %s",s->brake?"SI":"NO");tones[3]=entry_tone(s,s->brake);

    if(s->drive_input && s->reverse_input) {
        snprintf(lines[4],41,"SELECTOR: D Y R A LA VEZ INVALIDO");tones[4]=KART_TONE_FAIL;
    } else if(s->drive_input || s->reverse_input) {
        snprintf(lines[4],41,"SELECTOR: %c",s->drive_input?'D':'R');tones[4]=KART_TONE_OK;
    } else {
        snprintf(lines[4],41,"SELECTOR: N SIN SOLICITUD D/R");tones[4]=KART_TONE_TEXT;
    }

    // Datos CAN: feedback 0x123 y estado extendido 0x124 por separado, con edad y limite del codigo.
    age_text(age,s->can_age);
    if(s->can_age==UINT32_MAX) {snprintf(lines[5],41,"CAN 0X123: NUNCA RECIBIDO");tones[5]=KART_TONE_FAIL;}
    else {
        snprintf(lines[5],41,"CAN 0X123: %s %s LIM %u",s->feedback_fresh?"OK":"CADUCADO",age,(unsigned)KART_FEEDBACK_MAX_AGE_MS);
        tones[5]=(uint8_t)(s->feedback_fresh?KART_TONE_OK:KART_TONE_FAIL);
    }

    age_text(age,s->status_age);
    if(!s->status_seen) {snprintf(lines[6],41,"CAN 0X124: NUNCA RECIBIDO");tones[6]=KART_TONE_WARN;}
    else if(!s->status_fresh) {snprintf(lines[6],41,"CAN 0X124: CADUCADO %s LIM %u",age,(unsigned)KART_STATUS_MAX_AGE_MS);tones[6]=KART_TONE_FAIL;}
    else if(!(s->backend_flags&KART_STATUS_SPEED_VALID)) {snprintf(lines[6],41,"CAN 0X124: VELOCIDAD INVALIDA");tones[6]=KART_TONE_FAIL;}
    else {snprintf(lines[6],41,"CAN 0X124: OK %s LIM %u",age,(unsigned)KART_STATUS_MAX_AGE_MS);tones[6]=KART_TONE_OK;}

    // Quietud: 0x124 si existe; sin 0x124 se usa el criterio heredado de RPM traseras en cero.
    if(!s->feedback_fresh) {snprintf(lines[7],41,"PARADO: -- SIN FEEDBACK RECIENTE");tones[7]=KART_TONE_WARN;}
    else if(s->status_seen && !s->status_fresh) {snprintf(lines[7],41,"PARADO: -- SIN ESTADO 0X124 RECIENTE");tones[7]=KART_TONE_WARN;}
    else if(s->status_seen && !(s->backend_flags&KART_STATUS_SPEED_VALID)) {snprintf(lines[7],41,"PARADO: -- VELOCIDAD NO VALIDA");tones[7]=KART_TONE_WARN;}
    else if(s->status_seen) {snprintf(lines[7],41,"PARADO: %s SEGUN 0X124",s->stationary?"SI":"NO");tones[7]=entry_tone(s,s->stationary);}
    else {snprintf(lines[7],41,"PARADO: %s RPM %d/%d",s->stationary?"SI":"NO",s->rpm_left,s->rpm_right);tones[7]=entry_tone(s,s->stationary);}

    // Validez de las entradas que bloquean D/R y el cambio de modo.
    snprintf(lines[8],41,"ENTR PEDAL:%s DIR:%s SEL:%s",s->input_flags&KART_INPUT_ACCEL_OK?"OK":"FALLO",
        s->input_flags&KART_INPUT_STEER_OK?"OK":"FALLO",s->input_flags&KART_INPUT_BUTTONS_OK?"OK":"FALLO");
    tones[8]=(uint8_t)(s->input_flags==7?KART_TONE_OK:KART_TONE_FAIL);

    // Causa que informa el ESP32 del control trasero: es la otra mitad de lo que impide mover los motores.
    snprintf(lines[9],41,"CAUSA ESP: %s",inhibit_reason(s));
    tones[9]=(uint8_t)((s->feedback_fresh && s->status_fresh && s->inhibit==0)?KART_TONE_OK:KART_TONE_WARN);

    // Cambio de modo: mismas condiciones que Kart_RequestNextMode usa en el control.
    const uint8_t mode=s->mode_cond;
    if(mode==KART_MODE_COND_ALL) {snprintf(lines[10],41,"MODO: CAMBIO PERMITIDO");tones[10]=KART_TONE_OK;}
    else {
        missing[0]=0;
        if(!(mode&KART_MODE_COND_INPUTS))add_token(missing,sizeof(missing),"ENTR");
        if(!(mode&KART_MODE_COND_CAN))add_token(missing,sizeof(missing),"CAN");
        if(!(mode&KART_MODE_COND_NEUTRAL))add_token(missing,sizeof(missing),"N");
        if(!(mode&KART_MODE_COND_STOPPED))add_token(missing,sizeof(missing),"PARADO");
        if(!(mode&KART_MODE_COND_PEDAL))add_token(missing,sizeof(missing),"PEDAL");
        snprintf(lines[10],41,"MODO FALTA:%s",missing);tones[10]=KART_TONE_WARN;
    }
}
