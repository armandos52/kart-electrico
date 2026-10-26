#include "kart_dashboard.h"
#include "kart_display.h"
#include <stdio.h>
#include <string.h>

typedef struct {uint32_t bg,panel,text,muted,line,active,on_active,warn,on_warn,ok,on_ok,fail,on_fail;} Palette;
static const Palette dark={0x10181e,0x1b2932,0xedf4f5,0xb1c5ce,0x394e59,0xb0e2eb,0x102b35,0x423a25,0xffe1a2,0x143b32,0x9ee6cb,0x47282b,0xffbfc1};
/* Historical light palette unused: fixed dark. */
static const Palette light={0xf5f7f8,0xe6edf0,0x162c36,0x415b66,0xc0d0d7,0x155d72,0xffffff,0xf4e6c0,0x654800,0xd4ece2,0x174c38,0xf5dddd,0x7c252d};
static const char* const pages[]={"Resumen","Entradas","Marcha","Ruedas","CAN","SD","BMS"};
const char* KartDashboard_PageName(uint8_t page){return page<KART_PAGE_COUNT?pages[page]:"Resumen";}
static int inside(int x,int y,int left,int top,int w,int h){return x>=left&&x<left+w&&y>=top&&y<top+h;}
static int hit(const KartDashboardState* st,int x,int y){
    if(x<0||x>=480||y<48||y>=272)return -1;
    if(y>=228){int p=x/68;return 10+(p<KART_PAGE_COUNT?p:KART_PAGE_COUNT-1);}
    if(inside(x,y,392,48,88,58))return 1;
    if(st->page==KART_PAGE_HOME && inside(x,y,142,110,178,42))return 2;
    if(st->page==KART_PAGE_DRIVE && inside(x,y,338,174,132,46))return 2;
    if(st->page==KART_PAGE_WHEELS){
        if(inside(x,y,10,112,64,48))return 20;
        if(inside(x,y,138,112,64,48))return 21;
        if(inside(x,y,10,172,64,48))return 22;
        if(inside(x,y,138,172,64,48))return 23;
    }
    return -1;
}
void KartDashboard_Init(KartDashboardState* st){memset(st,0,sizeof(*st));st->pressed=-1;st->wheel=2;}
uint8_t KartDashboard_Touch(KartDashboardState* st,int event,int x,int y){
    if(event==0){st->pressed=(int16_t)hit(st,x,y);st->press_x=(int16_t)x;st->press_y=(int16_t)y;return KART_ACTION_NONE;}
    // Comparar con el origen, no con el movimiento anterior: el desplazamiento se acumula.
    if(event==3 || event==1){
        if(x<st->press_x-8 || x>st->press_x+8 || y<st->press_y-8 || y>st->press_y+8 || hit(st,x,y)!=st->pressed)st->pressed=-1;
        if(event==3)return KART_ACTION_NONE;
    }
    if(event!=1){st->pressed=-1;return KART_ACTION_NONE;}
    const int action=st->pressed;st->pressed=-1;
    if(action<0||action!=hit(st,x,y))return KART_ACTION_NONE;
    if(action==1)return KART_ACTION_RECORD;
    else if(action==2)return KART_ACTION_MODE;
    else if(action==3)return KART_ACTION_RECORD;
    else if(action>=10&&action<10+KART_PAGE_COUNT)st->page=(uint8_t)(action-10);
    else if(action>=20&&action<24)st->wheel=(uint8_t)(action-20);
    return KART_ACTION_NONE;
}
static void box(const KartDashboardPainter* p,int x,int y,int w,int h,uint32_t color){p->rect(p->context,x,y,w,h,color);}
static void txt(const KartDashboardPainter* p,int x,int y,int w,uint8_t font,uint32_t color,const char* value){p->text(p->context,x,y,w,font,color,value);}
static void right(const KartDashboardPainter* p,int x,int y,int w,uint8_t font,uint32_t color,const char* value){int width=KartUIFont_Width(font,value);txt(p,width<w?x+w-width:x,y,width<w?width:w,font,color,value);}
static void centered(const KartDashboardPainter* p,int x,int y,int w,uint8_t font,uint32_t color,const char* value){int width=KartUIFont_Width(font,value);txt(p,width<w?x+(w-width)/2:x,y,width<w?width:w,font,color,value);}
static void button(const KartDashboardPainter* p,const Palette* c,int x,int y,int w,int h,const char* label,int selected){
    box(p,x,y,w,h,selected?c->active:c->panel);
    centered(p,x,y+(h-kart_ui_fonts[KART_FONT_BODY].height)/2,w,KART_FONT_BODY,selected?c->on_active:c->text,label);
}
static void age_string(char* text,size_t size,uint32_t age){
    if(age==UINT32_MAX)snprintf(text,size,"Nunca");
    else if(age<10000)snprintf(text,size,"%lu ms",(unsigned long)age);
    else if(age<1000000000u)snprintf(text,size,"%lu s",(unsigned long)(age/1000));
    else snprintf(text,size,">999999 s");
}
static const char* sd_name(const KartUiSnapshot* s){return s->sd_state==KART_SD_RECORDING?"Grabando":s->sd_state==KART_SD_ERROR?"Error":s->sd_state==KART_SD_OPENING?"Abriendo":s->sd_state==KART_SD_CLOSING?"Cerrando":s->sd_state==KART_SD_SAVED?"Guardado":"Sin grabar";}
static const char* gear_name(unsigned gear){return gear==1?"D":gear==2?"R":gear==0?"N":"--";}
static int state_ok(const KartUiSnapshot* s){return s->feedback_fresh&&s->status_seen&&s->status_fresh;}
// Interpretacion historica solo para diagnostico: no modifica flags ni control.
// Comprobar el entero CAN ANTES de dividir para no aceptar 100.01 % o 150.1 C.
static int bms_soc_value(const KartUiSnapshot* s,int* value){
    if(s->bms_soc_age>KART_BMS_MAX_AGE_MS)return 0;
    if(s->soc_valid){if(s->soc_pct>100)return 0;*value=s->soc_pct;}
    else {const unsigned raw=((unsigned)s->bms355[5]<<8)|s->bms355[4];if(raw>10000)return 0;*value=(int)(raw/100);}
    return 1;
}
static int bms_temp_value(const KartUiSnapshot* s,int* value){
    if(s->bms_temp_age>KART_BMS_MAX_AGE_MS)return 0;
    if(s->temp_valid){if(s->battery_temp_c<-50||s->battery_temp_c>150)return 0;*value=s->battery_temp_c;}
    else {const int raw=(int16_t)(((uint16_t)s->bms356[5]<<8)|s->bms356[4]);if(raw<-500||raw>1500)return 0;*value=raw/10;}
    return 1;
}
static void reason(const KartUiSnapshot* s,char* main,char* detail){
    static const char* const reasons[]={"Tracción permitida","Sin mando CAN","Mando CAN caducado","Mando inválido","START apagado","Preparando relés","Cargador conectado","Rearme pendiente","Freno activo","Pedal libre","Neutro","Parar para cambiar sentido","Parar en N para cambiar modo","Velocidad inválida","Revisar Hall","MPU no disponible","Entradas frontales inválidas","Límite R activo","Fallo de salida","Fallo CAN","Configuración inválida"};
    if(!s->feedback_fresh)snprintf(main,64,"Sin datos CAN recientes");
    else if(!s->status_seen)snprintf(main,64,"Estado ESP sin confirmar");
    else if(!s->status_fresh)snprintf(main,64,"Estado ESP caducado");
    else snprintf(main,64,"%s",s->inhibit<21?reasons[s->inhibit]:"Estado ESP desconocido");
    if(s->ui_notice==KART_UI_REQUIRE_N)snprintf(detail,64,"Cambio de modo: poner selector en N");
    else if(s->ui_notice==KART_UI_REQUIRE_STOP)snprintf(detail,64,"Cambio de modo: parar y soltar pedal");
    else if(s->ui_notice==KART_UI_CAN_OLD)snprintf(detail,64,"Cambio de modo: falta CAN válido");
    else if(s->ui_notice==KART_UI_INPUT_BAD)snprintf(detail,64,"Cambio de modo: revisar entradas");
    else if(!s->feedback_fresh)snprintf(detail,64,"Velocidad desconocida; revisar CAN");
    else if(!s->status_seen)snprintf(detail,64,"Recibido 0x123; falta estado 0x124");
    else if(!s->status_fresh)snprintf(detail,64,"No se reutiliza el estado anterior");
    else if(s->inhibit==7)snprintf(detail,64,"N + freno + pedal libre; nuevo START");
    else if(s->inhibit==17)snprintf(detail,64,"Actuación informada por el ESP32");
    else snprintf(detail,64,"Petición START %s  /  marcha frontal %s",s->start?"ON":"OFF",gear_name(s->gear));
}
static void draw_home(const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    char value[48];box(p,10,114,122,106,c->panel);
    if(s->feedback_usable)snprintf(value,sizeof(value),"%d",s->speed_kph);else snprintf(value,sizeof(value),"--");
    txt(p,20,118,104,KART_FONT_SPEED,c->text,value);
    snprintf(value,sizeof(value),"km/h  ·  %s",gear_name(s->gear));txt(p,20,159,104,KART_FONT_BODY,c->text,value);
    txt(p,20,188,105,KART_FONT_SMALL,c->muted,s->feedback_usable?"Vel. recibida":"Sin dato válido");
    box(p,142,110,178,42,c->panel);txt(p,150,114,110,KART_FONT_SMALL,c->muted,"Solicitado / tocar");
    right(p,252,125,60,KART_FONT_VALUE,c->text,Kart_ModeName(s->mode_requested));
    txt(p,150,159,90,KART_FONT_SMALL,c->muted,"Recibido");right(p,240,157,72,KART_FONT_BODY,c->text,state_ok(s)?Kart_ModeName(s->mode_received):"--");
    box(p,142,182,178,1,c->line);txt(p,150,194,96,KART_FONT_BODY,c->text,"Aplicado");right(p,240,188,72,KART_FONT_VALUE,c->text,state_ok(s)?Kart_ModeName(s->mode_applied):"--");
    box(p,330,114,1,106,c->line);
    snprintf(value,sizeof(value),"START %s (pet.)",s->start?"ON":"OFF");txt(p,340,116,132,KART_FONT_SMALL,c->text,value);
    snprintf(value,sizeof(value),"Freno %s",s->brake?"SÍ":"NO");txt(p,340,141,132,KART_FONT_BODY,c->text,value);
    snprintf(value,sizeof(value),"SD %s",sd_name(s));txt(p,340,167,132,KART_FONT_BODY,c->text,value);
    int soc=0,temp=0;const int has_soc=bms_soc_value(s,&soc),has_temp=bms_temp_value(s,&temp);
    const int provisional=(has_soc&&!s->soc_valid)||(has_temp&&!s->temp_valid);
    char charge[12],heat[12];
    if(has_soc)snprintf(charge,sizeof(charge),"%d%%",soc);else snprintf(charge,sizeof(charge),"--%%");
    if(has_temp)snprintf(heat,sizeof(heat),"%d°C",temp);else snprintf(heat,sizeof(heat),"--°C");
    snprintf(value,sizeof(value),"Bat%s %s  %s",provisional?"*":"",charge,heat);txt(p,340,190,132,KART_FONT_SMALL,c->text,value);
    txt(p,340,208,132,KART_FONT_SMALL,c->muted,provisional?"*BMS por validar":has_soc&&has_temp?"BMS validado":"BMS sin dato válido");
}
static void draw_inputs(const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    char v[64];
    txt(p,10,111,214,KART_FONT_SMALL,c->muted,"Pedal · ADC físico / mando filtrado");
    if(s->input_flags&KART_INPUT_ACCEL_OK)snprintf(v,sizeof(v),"%u / %u",s->accel_adc,s->accel_raw);else snprintf(v,sizeof(v),"Inválido (ADC %u)",s->accel_adc);
    txt(p,10,129,218,KART_FONT_VALUE,c->text,v);
    box(p,10,158,216,5,c->panel);if(s->input_flags&KART_INPUT_ACCEL_OK)box(p,10,158,(int)((uint32_t)s->accel_raw*216/4095),5,c->active);
    snprintf(v,sizeof(v),"Libre: %s · cero hasta ADC %u",!(s->input_flags&KART_INPUT_ACCEL_OK)?"--":s->pedal_free?"SÍ":"NO",(unsigned)KART_ACCEL_ZERO_ADC);txt(p,10,170,226,KART_FONT_SMALL,c->text,v);
    txt(p,250,111,220,KART_FONT_SMALL,c->muted,"Dirección · ADC físico / normalizado");
    if(s->input_flags&KART_INPUT_STEER_OK)snprintf(v,sizeof(v),"%u / %u",s->steer_adc,s->steer_raw);else snprintf(v,sizeof(v),"Inválida (ADC %u)",s->steer_adc);
    txt(p,250,129,220,KART_FONT_VALUE,c->text,v);
    box(p,250,158,220,5,c->panel);if(s->input_flags&KART_INPUT_STEER_OK)box(p,250,158,(int)((uint32_t)s->steer_raw*220/4095),5,c->active);
    snprintf(v,sizeof(v),"Errores ADC: %lu",(unsigned long)s->adc_errors);txt(p,250,170,220,KART_FONT_SMALL,c->muted,v);
    snprintf(v,sizeof(v),"Freno:%s   START pin:%u / pet:%u   Selector:%s",s->brake?"SÍ":"NO",s->start_input,s->start,s->drive_input&&s->reverse_input?"D+R!":s->drive_input?"D":s->reverse_input?"R":"N");txt(p,10,195,460,KART_FONT_SMALL,c->text,v);
    txt(p,10,211,460,KART_FONT_SMALL,c->muted,"Referencias de desarrollo · calibración física pendiente");
}
static void condition(const KartDashboardPainter* p,const Palette* c,int x,int y,const char* name,int yes,int known,int required){
    box(p,x,y,225,26,known?(yes?c->ok:required?c->fail:c->panel):c->warn);
    const uint32_t color=known?(yes?c->on_ok:required?c->on_fail:c->text):c->on_warn;
    txt(p,x+7,y+5,167,KART_FONT_SMALL,color,name);right(p,x+176,y+5,42,KART_FONT_SMALL,color,!known?"--":yes?"SÍ":"NO");
}
static void draw_drive(const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    char v[80];int engaged=s->gear==1||s->gear==2;
    condition(p,c,10,111,"Entradas válidas",s->input_flags==7,1,1);
    condition(p,c,245,111,"CAN para D/R",s->feedback_usable,1,1);
    condition(p,c,10,142,"Freno pisado",s->brake,1,!engaged);
    condition(p,c,245,142,"Pedal libre",s->pedal_free,s->input_flags&KART_INPUT_ACCEL_OK,!engaged);
    condition(p,c,10,173,"Parado según CAN",s->stationary,s->feedback_usable,!engaged);
    const char* sel=s->drive_input&&s->reverse_input?"D+R!":s->drive_input?"D":s->reverse_input?"R":"N";
    snprintf(v,sizeof(v),"Sel: %s",sel);txt(p,245,175,85,KART_FONT_SMALL,c->text,v);
    button(p,c,338,174,132,46,"Cambiar modo",0);
    txt(p,10,208,322,KART_FONT_SMALL,c->muted,engaged?"Condiciones de entrada; marcha ya seleccionada":"Verificar condiciones antes de entrar en D/R");
}
static void draw_wheels(const KartDashboardState* st,const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    static const char* const names[]={"Delantera izquierda","Delantera derecha","Trasera izquierda","Trasera derecha"};
    static const char* const short_names[]={"DI","DD","TI","TD"};
    char v[64],age[24];
    const int valid[]={s->front_left_valid,s->front_right_valid,s->feedback_usable,s->feedback_usable};
    const int rpm[]={s->rpm_front_left,s->rpm_front_right,s->rpm_left,s->rpm_right};
    box(p,42,134,128,2,c->line);box(p,42,194,128,2,c->line);box(p,82,115,48,102,c->panel);
    centered(p,82,135,48,KART_FONT_SMALL,c->muted,"Frente");centered(p,82,156,48,KART_FONT_VALUE,c->text,gear_name(s->gear));centered(p,82,191,48,KART_FONT_SMALL,c->muted,"RPM");
    unsigned selected=st->wheel<4?st->wheel:2;
    for(unsigned i=0;i<4;i++){
        const int x=i%2?138:10,y=i<2?112:172;box(p,x,y,64,48,selected==i?c->active:c->panel);
        const uint32_t color=selected==i?c->on_active:c->text;
        centered(p,x,y+3,64,KART_FONT_SMALL,color,short_names[i]);
        if(valid[i])snprintf(v,sizeof(v),"%d",rpm[i]);else snprintf(v,sizeof(v),"--");centered(p,x,y+21,64,KART_FONT_BODY,color,v);
    }
    txt(p,220,110,250,KART_FONT_BODY,c->text,names[selected]);
    if(valid[selected])snprintf(v,sizeof(v),"%d RPM",rpm[selected]);else snprintf(v,sizeof(v),"-- RPM");txt(p,220,132,250,KART_FONT_VALUE,c->text,v);
    age_string(age,sizeof(age),selected==0?s->front_left_age:selected==1?s->front_right_age:s->can_age);
    snprintf(v,sizeof(v),"%s · edad %s",selected<2?"Hall frontal":"Feedback CAN",age);txt(p,220,161,250,KART_FONT_SMALL,c->muted,v);
    txt(p,220,178,250,KART_FONT_SMALL,c->muted,valid[selected]?selected<2?"PPR histórico: 4 · validar":"Validez según feedback disponible":"Sin medida válida; no afirmar parada");
    if(state_ok(s)&&(s->backend_flags&KART_STATUS_YAW_VALID))snprintf(v,sizeof(v),"Giro MPU: %s%d.%d °/s",s->yaw_deci_dps<0?"-":"",(s->yaw_deci_dps<0?-(int)s->yaw_deci_dps:s->yaw_deci_dps)/10,(s->yaw_deci_dps<0?-(int)s->yaw_deci_dps:s->yaw_deci_dps)%10);else snprintf(v,sizeof(v),"Giro MPU: --");txt(p,220,201,250,KART_FONT_BODY,c->text,v);
}
static void can_row(const KartDashboardPainter* p,const Palette* c,int y,const char* label,uint32_t age,int fresh){
    char value[48],a[24];age_string(a,sizeof(a),age);
    txt(p,10,y,240,KART_FONT_BODY,c->text,label);
    snprintf(value,sizeof(value),"%s%s%s",age==UINT32_MAX?"":fresh?"OK · ":"Antiguo · ",a,"");right(p,250,y,220,KART_FONT_SMALL,c->text,value);box(p,10,y+21,460,1,c->line);
}
static void draw_can(const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    char v[88];can_row(p,c,111,"0x123 · velocidad / RPM",s->can_age,s->feedback_fresh);
    can_row(p,c,138,"0x124 · estado del ESP",s->status_age,s->status_fresh);
    snprintf(v,sizeof(v),"Errores RX:%lu / TX:%lu",(unsigned long)s->can_rx_errors,(unsigned long)s->can_tx_errors);txt(p,10,168,460,KART_FONT_SMALL,c->text,v);
    static const char* const relays[]={"OFF","Precarga","Solapamiento","Listo"};
    snprintf(v,sizeof(v),"Relé lógico ESP: %s · sentido ESP: %s",state_ok(s)&&s->relay_state<4?relays[s->relay_state]:"--",state_ok(s)?gear_name(s->backend_gear):"--");txt(p,10,187,460,KART_FONT_SMALL,c->text,v);
    txt(p,10,209,460,KART_FONT_SMALL,c->muted,"Firmware ESP sin identificar · 0x124 identifica protocolo");
}
static void draw_sd(const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    char v[96];snprintf(v,sizeof(v),"SD / S%07lu / %s",(unsigned long)s->sd_session,sd_name(s));txt(p,10,111,460,KART_FONT_BODY,c->text,v);
    snprintf(v,sizeof(v),"Filas %lu · perdidas %lu · cola %lu",(unsigned long)s->sd_rows,(unsigned long)s->sd_dropped,(unsigned long)s->sd_queued);txt(p,10,140,460,KART_FONT_SMALL,s->sd_dropped?c->on_warn:c->text,v);
    snprintf(v,sizeof(v),"Tiempo %lu s · error %u · SD max %lu ms",(unsigned long)(s->sd_elapsed_ms/1000),s->sd_error,(unsigned long)s->sd_max_call_ms);txt(p,10,162,460,KART_FONT_SMALL,c->text,v);
    if(s->sd_free_kib==UINT32_MAX)snprintf(v,sizeof(v),"Espacio sin verificar · cierre necesario antes de retirar");
    else snprintf(v,sizeof(v),"Libre %lu KiB · ultimo sync tick %lu",(unsigned long)s->sd_free_kib,(unsigned long)s->sd_last_save);
    txt(p,10,184,460,KART_FONT_SMALL,c->muted,v);
    txt(p,10,208,460,KART_FONT_SMALL,c->muted,s->sd_error==KART_SD_ABSENT?"Sin tarjeta detectada · no se esta guardando":s->sd_state==KART_SD_CLOSING?"Cerrando archivos · espera a Guardado":"Control de grabacion en la esquina superior derecha");
}
static void draw_bms(const KartUiSnapshot* s,const KartDashboardPainter* p,const Palette* c){
    char v[96],age[24],hex[24];
    int soc=0,temp=0;const int has_soc=bms_soc_value(s,&soc),has_temp=bms_temp_value(s,&temp);
    const int provisional=(has_soc&&!s->soc_valid)||(has_temp&&!s->temp_valid);
    if(has_soc)snprintf(v,sizeof(v),"SOC%s %d %%",s->soc_valid?"":"*",soc);else snprintf(v,sizeof(v),"SOC -- %%");txt(p,10,112,225,KART_FONT_VALUE,c->text,v);
    if(has_temp)snprintf(v,sizeof(v),"%d °C%s",temp,s->temp_valid?"":"*");else snprintf(v,sizeof(v),"-- °C");txt(p,250,112,220,KART_FONT_VALUE,c->text,v);
    age_string(age,sizeof(age),s->bms_soc_age);for(unsigned i=0;i<8;i++)snprintf(hex+2*i,3,"%02X",s->bms355[i]);
    snprintf(v,sizeof(v),"0x355  %s  · %s",hex,age);txt(p,10,146,460,KART_FONT_SMALL,c->muted,v);
    age_string(age,sizeof(age),s->bms_temp_age);for(unsigned i=0;i<8;i++)snprintf(hex+2*i,3,"%02X",s->bms356[i]);
    snprintf(v,sizeof(v),"0x356  %s  · %s",hex,age);txt(p,10,167,460,KART_FONT_SMALL,c->muted,v);
    txt(p,10,192,460,KART_FONT_BODY,c->text,provisional?"* Lectura provisional; formato por confirmar":has_soc&&has_temp?"Datos recibidos y validados":"Sin datos recientes o dentro de rango");
    txt(p,10,212,460,KART_FONT_SMALL,c->muted,"Celdas individuales, tensión y corriente: no disponibles");
}
void KartDashboard_Draw(const KartDashboardState* st,const KartUiSnapshot* s,const KartDashboardPainter* p){
    const Palette* c=&dark;
    const uint8_t page=st->page<KART_PAGE_COUNT?st->page:KART_PAGE_HOME;
    char title[64],main[64],detail[64];
    box(p,0,48,480,224,c->bg);
    const char* version=strchr(KART_FRONTEND_VERSION,'-');version=version?version+1:KART_FRONTEND_VERSION;
    snprintf(title,sizeof(title),"KART / %s  ·  %s",KartDashboard_PageName(page),version);txt(p,10,52,376,KART_FONT_SMALL,c->muted,title);
    reason(s,main,detail);const int running=state_ok(s)&&s->inhibit==0;
    box(p,0,72,392,34,running?c->ok:c->warn);
    txt(p,10,73,374,KART_FONT_BODY,running?c->on_ok:c->on_warn,main);
    txt(p,10,90,374,KART_FONT_SMALL,running?c->on_ok:c->on_warn,detail);
    box(p,392,48,88,58,s->sd_state==KART_SD_RECORDING?c->ok:s->sd_error?c->fail:c->panel);
    centered(p,392,55,88,KART_FONT_SMALL,c->text,s->sd_state==KART_SD_RECORDING?"REC":s->sd_error?"SD: error":s->sd_state==KART_SD_SAVED?"Guardado":"SD");
    centered(p,392,78,88,KART_FONT_BODY,c->text,s->sd_state==KART_SD_CLOSING?"Cerrando":s->sd_state==KART_SD_OPENING?"Cancelar":s->sd_error?"Reintentar":s->sd_requested?"Detener":"Grabar");
    switch(page){
    case KART_PAGE_INPUTS:draw_inputs(s,p,c);break;
    case KART_PAGE_DRIVE:draw_drive(s,p,c);break;
    case KART_PAGE_WHEELS:draw_wheels(st,s,p,c);break;
    case KART_PAGE_CAN:draw_can(s,p,c);break;
    case KART_PAGE_SD:draw_sd(s,p,c);break;
    case KART_PAGE_BMS:draw_bms(s,p,c);break;
    default:draw_home(s,p,c);break;
    }
    box(p,0,227,480,1,c->line);
    for(unsigned i=0;i<KART_PAGE_COUNT;i++)button(p,c,(int)i*68,228,i==6?72:68,44,pages[i],page==i);
}
