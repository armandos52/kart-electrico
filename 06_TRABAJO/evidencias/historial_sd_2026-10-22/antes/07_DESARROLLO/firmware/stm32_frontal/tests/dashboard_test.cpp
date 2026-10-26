// ENS-0022. Dibuja la misma función C que TouchGFX, con escenarios sintéticos.
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <string>
#include <vector>
#include "../Core/Src/kart_ui_font.c"
#include "../Core/Src/kart_ui_font_data.c"
#include "../Core/Src/kart_display.c"
#include "../Core/Src/kart_dashboard.c"
static_assert(sizeof(KartUiSnapshot)==116 && offsetof(KartUiSnapshot,front_left_age)==96 &&
              offsetof(KartUiSnapshot,drive_input)==104,"Comprobar decodificador SWD");
struct Op {int x,y,w,h,font;uint32_t color;std::string text;};
std::vector<Op> ops;
int checks=0,failures=0;
void check(bool ok,const char* label){checks++;if(!ok)failures++;printf("%s %s\n",ok?"PASS":"FAIL",label);}
void rect_out(void*,int x,int y,int w,int h,uint32_t color){ops.push_back({x,y,w,h,-1,color,""});}
void text_out(void*,int x,int y,int w,uint8_t font,uint32_t color,const char* text){ops.push_back({x,y,w,kart_ui_fonts[font].height,font,color,text});}
void render(KartDashboardState st,KartUiSnapshot snap){ops.clear();KartDashboardPainter p={nullptr,rect_out,text_out};KartDashboard_Draw(&st,&snap,&p);}
bool contains(const char* text){for(const auto& op:ops)if(op.text.find(text)!=std::string::npos)return true;return false;}
bool within_screen(){
    bool ok=true;
    for(const auto& op:ops){
        bool fits=op.x>=0 && op.y>=48 && op.w>=0 && op.h>=0 && op.x+op.w<=480 && op.y+op.h<=272;
        if(op.font>=0){
            fits &= KartUIFont_Width((uint8_t)op.font,op.text.c_str())<=op.w;
            const char* s=op.text.c_str();uint16_t cp;
            while((cp=KartUIFont_Next(&s)))fits &= KartUIFont_Glyph((uint8_t)op.font,cp)->unicode==cp;
        }
        if(!fits){printf("  CLIP (%d,%d,%d,%d) font=%d width=%d %s\n",op.x,op.y,op.w,op.h,op.font,op.font<0?0:KartUIFont_Width((uint8_t)op.font,op.text.c_str()),op.text.c_str());ok=false;}
    }return ok;
}
KartUiSnapshot sample(int scenario){
    KartUiSnapshot s={};
    s.can_age=s.status_age=s.bms_soc_age=s.bms_temp_age=s.front_left_age=s.front_right_age=UINT32_MAX;
    s.mode_received=s.mode_applied=s.relay_state=s.backend_gear=255;
    s.accel_adc=400;s.accel_raw=381;s.steer_adc=2177;s.steer_raw=2048;s.input_flags=7;s.brake=1;
    if(scenario==0)return s;
    s.feedback_fresh=s.feedback_usable=1;s.can_age=10;s.stationary=1;
    if(scenario==1){s.drive_input=1;return s;} // Compatible con ESP legado; no estado 0x124.
    s.status_seen=s.status_fresh=1;s.status_age=15;s.backend_flags=28;s.mode_received=1;s.mode_requested=2;s.mode_applied=0;s.relay_state=3;s.backend_gear=0;s.inhibit=7;s.start=1;
    if(scenario==2){s.sd_requested=1;s.sd_state=KART_SD_ERROR;s.sd_error=1;return s;}
    s.gear=s.drive_input=s.backend_gear=1;s.brake=s.stationary=0;s.inhibit=0;s.backend_flags=12;
    s.speed_kph=23;s.rpm_left=355;s.rpm_right=359;s.rpm_front_left=350;s.rpm_front_right=352;
    s.front_left_valid=s.front_right_valid=1;s.front_left_age=20;s.front_right_age=19;
    s.accel_adc=500;s.accel_raw=857;s.yaw_deci_dps=-24;s.sd_requested=1;s.sd_state=KART_SD_RECORDING;s.sd_rows=12345;
    if(scenario==3)return s;
    if(scenario==4){s.input_flags=0;s.accel_adc=s.steer_adc=65535;s.feedback_usable=s.status_fresh=s.front_left_valid=s.front_right_valid=0;s.status_age=350;s.reverse_input=1;s.ui_notice=KART_UI_INPUT_BAD;return s;}
    s.speed_kph=200;s.rpm_left=s.rpm_right=s.rpm_front_left=s.rpm_front_right=2000;s.accel_adc=s.steer_adc=4095;s.accel_raw=s.steer_raw=4095;
    s.sd_rows=s.adc_errors=s.can_rx_errors=s.can_tx_errors=UINT32_MAX;s.can_age=s.status_age=s.bms_soc_age=s.bms_temp_age=s.front_left_age=s.front_right_age=999999999u;
    s.soc_valid=s.temp_valid=1;s.soc_pct=100;s.battery_temp_c=-40;s.yaw_deci_dps=INT16_MIN;
    memset(s.bms355,0xff,8);memset(s.bms356,0xff,8);return s;
}
uint8_t tap(KartDashboardState& st,int x,int y){KartDashboard_Touch(&st,0,x,y);return KartDashboard_Touch(&st,1,x,y);}
void save_json(FILE* file,int scenario,int theme,int page,int wheel){
    fprintf(file,"{\"scenario\":%d,\"theme\":%d,\"page\":%d,\"wheel\":%d,\"ops\":[",scenario,theme,page,wheel);
    for(size_t i=0;i<ops.size();i++){
        const auto& o=ops[i];fprintf(file,"%s{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"font\":%d,\"color\":%lu,\"text\":\"",i?",":"",o.x,o.y,o.w,o.h,o.font,(unsigned long)o.color);
        for(char c:o.text){if(c=='"'||c=='\\')fputc('\\',file);fputc(c,file);}fprintf(file,"\"}");
    }fprintf(file,"]}\n");
}
#include "bms_dashboard_cases.h"
int main(int argc,char** argv){
    KartDashboardState st;KartDashboard_Init(&st);
    check(!st.light && st.page==KART_PAGE_HOME,"Arranque oscuro en resumen");
    check(KartDashboard_Touch(&st,1,200,130)==0,"Liberacion aislada no cambia modo");
    check(tap(st,440,65)==0 && st.light,"Tema cambia sin emitir accion de control");
    bool nav=true;
    for(int p=0;p<KART_PAGE_COUNT;p++)nav &= tap(st,p*68+20,250)==0 && st.page==p;
    check(nav,"Siete paginas navegables sin accion de control");
    st.page=KART_PAGE_HOME;
    KartDashboard_Touch(&st,0,200,130);KartDashboard_Touch(&st,2,200,130);
    check(KartDashboard_Touch(&st,1,200,130)==0,"Arrastrar o cancelar anula el gesto");
    KartDashboard_Touch(&st,0,200,130);
    check(KartDashboard_Touch(&st,1,400,130)==0,"Salir del boton cancela modo");
    check(tap(st,200,130)==KART_ACTION_MODE && KartDashboard_Touch(&st,1,200,130)==0,"Un toque completo produce una sola solicitud de modo");
    KartDashboard_Touch(&st,0,200,130);KartDashboard_Touch(&st,3,201,131);
    check(KartDashboard_Touch(&st,1,201,131)==KART_ACTION_MODE,"B01 oscilacion de un pixel no pierde el toque");
    KartDashboard_Touch(&st,0,200,130);KartDashboard_Touch(&st,3,208,138);
    check(KartDashboard_Touch(&st,1,208,138)==KART_ACTION_MODE,"B01 tolerancia de ocho pixeles en cada eje");
    KartDashboard_Touch(&st,0,200,130);
    check(KartDashboard_Touch(&st,1,220,130)==0,"B01 desplazamiento grande sin muestras intermedias tampoco activa");
    KartDashboard_Touch(&st,0,200,130);KartDashboard_Touch(&st,3,204,130);KartDashboard_Touch(&st,3,209,130);KartDashboard_Touch(&st,3,200,130);
    check(KartDashboard_Touch(&st,1,200,130)==0,"B01 desplazamiento acumulado cancela aunque regrese al origen");
    KartDashboard_Touch(&st,0,143,130);KartDashboard_Touch(&st,3,141,130);KartDashboard_Touch(&st,3,143,130);
    check(KartDashboard_Touch(&st,1,143,130)==0,"B01 salir de la zona cancela incluso dentro de la tolerancia");
    st.page=KART_PAGE_SD;check(tap(st,400,140)==KART_ACTION_RECORD,"Boton SD solicita grabacion o pausa");
    st.page=KART_PAGE_DRIVE;check(tap(st,400,175)==KART_ACTION_MODE,"Todo el boton Marcha corresponde a su area tactil");
    st.page=KART_PAGE_WHEELS;check(tap(st,150,120)==0 && st.wheel==1,"Seleccion de rueda sin efectos en control");
    check(tap(st,400,47)==0 && st.pressed==-1,"Franja superior sin accion");
    check(st.pressed==-1,"Gesto en franja descartado");
    FILE* output=nullptr;if(argc>1)output=fopen(argv[1],"wb");
    check(argc<=1||output,"Salida de evidencia disponible");
    for(int scenario=0;scenario<6;scenario++)for(int theme=0;theme<2;theme++)for(int page=0;page<KART_PAGE_COUNT;page++){
        st.page=(uint8_t)page;st.light=(uint8_t)theme;
        for(int wheel=0;wheel<(page==KART_PAGE_WHEELS?4:1);wheel++){
            st.wheel=(uint8_t)wheel;render(st,sample(scenario));
            char label[100];snprintf(label,sizeof(label),"Geometria y glifos caso=%d tema=%d pagina=%d rueda=%d",scenario,theme,page,wheel);check(within_screen(),label);
            if(output)save_json(output,scenario,theme,page,wheel);
        }
    }
    test_bms_dashboard(output);
    if(output)fclose(output);
    st.page=KART_PAGE_HOME;render(st,sample(1));
    check(contains("falta estado 0x124") && !contains("Tracción permitida"),"Feedback legado no inventa estado aplicado");
    render(st,sample(4));check(contains("Estado ESP caducado") && contains("revisar entradas") && contains("Sin dato válido"),"Causa, aviso y velocidad invalida permanecen diferenciados");
    st.page=KART_PAGE_WHEELS;st.wheel=0;render(st,sample(0));check(contains("-- RPM") && contains("no afirmar parada"),"Hall sin pulsos no se presenta como cero medido");
    st.page=KART_PAGE_SD;render(st,sample(2));check(contains("Error") && contains("Petición: grabar") && contains("Pedir pausa"),"Fallo SD no se confunde con grabacion solicitada");
    auto bad_csv=sample(2);bad_csv.sd_error=KART_SD_CSV_INVALID;render(st,bad_csv);
    check(contains("revisar CSV") && contains("conservar archivo") && within_screen(),"B04 CSV incompleto visible sin ocultar causa ni recortar textos");
    st.page=KART_PAGE_BMS;render(st,sample(0));check(contains("SOC --") && contains("-- °C"),"BMS ausente permanece desconocido");
    st.page=KART_PAGE_HOME;bool reasons_fit=true;
    for(int i=0;i<21;i++)for(int n=0;n<5;n++){auto s=sample(2);s.inhibit=(uint8_t)i;s.ui_notice=(uint8_t)n;render(st,s);reasons_fit &= within_screen();}
    check(reasons_fit,"Todas las causas y avisos caben simultaneamente");
    printf("TOTAL %d; FALLOS %d\n",checks,failures);return failures?1:0;
}
