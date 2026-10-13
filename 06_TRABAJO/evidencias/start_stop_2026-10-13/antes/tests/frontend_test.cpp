#include <cstdio>
#include <string>
#include "stubs/main.h"
#include "stubs/fatfs.h"
ADC_HandleTypeDef hadc3;CAN_HandleTypeDef hcan1={CAN1};FakeDwt fakeDwt={};FakeDebug fakeDebug={};
uint32_t SystemCoreClock=216000000,tick=1000,adc_channel=0;
uint16_t adc_accel_value=124,adc_steer_value=2177;
int adc_ok=1,pins[4]={0,0,0,0},mailboxes=3,tx_ok=1,tx_count=0,rx_pending=0,wd_refresh=0;
uint8_t sent_command[8]={},sent_health[8]={},rx_bytes[8]={};CAN_RxHeaderTypeDef rx_header={};
int mount_result=0,open_result=0,write_result=0,sync_result=0,close_result=0,short_write=0;
int mounts=0,opens=0,closes=0,syncs=0;std::string file_content;
#include "../Core/Src/kart_frontend.c"
#include "../Core/Src/kart_display.c"
int checks=0,failures=0;
void check(bool ok,const char* message){checks++;if(!ok)failures++;printf("%s %s\n",ok?"PASS":"FAIL",message);}
void reset(){
    memset((void*)&received,0,sizeof(received));memset(&ui,0,sizeof(ui));
    selected_mode=selected_gear=mode_click=record_request=notice=sd_open=sd_state=sd_error=0;
    sd_last_try=sd_last_log=sd_last_sync=sd_rows=sample_sequence=adc_errors=tx_errors=0;
    hall_l_seen=hall_r_seen=0;tick=1000;adc_ok=tx_ok=1;adc_accel_value=124;adc_steer_value=2177;
    memset(pins,0,sizeof(pins));mailboxes=3;mount_result=open_result=write_result=sync_result=close_result=short_write=0;
    mounts=opens=closes=syncs=0;file_content.clear();Kart_Init();
}
void rx(unsigned id,const uint8_t* b,unsigned len=8){rx_header={id,0,0,len};memcpy(rx_bytes,b,8);rx_pending=1;HAL_CAN_RxFifo0MsgPendingCallback(&hcan1);}
void feedback(){const uint8_t b[8]={0,0,0,0,0,0,0,0};rx(0x123,b);const uint8_t s[8]={1,10,0,255,4|16|8,3,0,0};rx(0x124,s);}
int main(){
    reset();Kart_ControlTick();check(ui.gear==0 && sent_command[4]==1 && sent_command[0]==0,"Sin feedback transmite N freno pedal cero");
    feedback();Kart_ControlTick();check(ui.input_flags==7 && ui.feedback_fresh && ui.status_fresh,"ADC validos y CAN fresco");
    check(sent_health[0]==1 && sent_health[1]==7 && kart_u16be(sent_health+2)==124,"Ext 0x101 conserva ADC fisico");
    const uint16_t seq=kart_u16be(sent_health+6);Kart_ControlTick();check(kart_u16be(sent_health+6)==uint16_t(seq+1),"Secuencia avanza por muestra");
    pins[2]=1;Kart_ControlTick();check(sent_command[5]==1 && ui.gear==1,"D exige freno quietud pedal libre");
    pins[0]=1;adc_accel_value=1861;Kart_ControlTick();check(kart_u16be(sent_command)==4095,"Pedal superior normalizado");
    received.status[4]=4;received.left=100;received.right=100;pins[2]=0;pins[3]=1;Kart_ControlTick();
    check(ui.gear==0 && !sent_command[5] && !sent_command[6],"D a R con movimiento pasa N, no conserva D");
    reset();feedback();adc_ok=0;Kart_ControlTick();check(ui.input_flags==4 && ui.accel_adc==65535 && sent_command[4]==1,"Fallo conversion ADC inhibe y publica centinela");
    reset();feedback();adc_accel_value=0;Kart_ControlTick();check(!(ui.input_flags&1),"ADC fuera de plausibilidad se rechaza");
    reset();feedback();pins[2]=pins[3]=1;Kart_ControlTick();check(!(ui.input_flags&4) && ui.gear==0,"D R simultaneos invalidos");
    reset();feedback();Kart_RequestNextMode();Kart_ControlTick();check(ui.mode_requested==1,"Modo avanza parado en N");
    pins[2]=1;Kart_RequestNextMode();Kart_ControlTick();check(ui.mode_requested==1 && ui.ui_notice==KART_UI_REQUIRE_N,"Modo con D se rechaza visible");
    reset();feedback();received.status[4]=4;received.left=1;Kart_RequestNextMode();Kart_ControlTick();
    check(ui.mode_requested==0 && ui.ui_notice==KART_UI_REQUIRE_STOP,"0 km/h entero no acredita quietud");
    reset();feedback();tick+=301;received.feedback_tick=tick;Kart_ControlTick();check(ui.feedback_fresh && !ui.status_fresh && ui.gear==0,"Estado extendido caducado no vuelve a legado");
    reset();feedback();tick+=501;Kart_ControlTick();check(!ui.feedback_fresh,"Feedback caduca");
    const uint8_t b[8]={};const uint32_t old=received.rx_errors;rx(0x123,b,7);check(received.rx_errors==old+1,"DLC incorrecto contado");
    reset();feedback();mailboxes=0;Kart_ControlTick();check(ui.can_tx_errors>0,"Buzones llenos abortan y cuentan");
    tx_ok=0;Kart_ControlTick();check(ui.can_tx_errors>=3,"Errores TX visibles");
    reset();Kart_ToggleRecording();Kart_SdTick();check(sd_state==KART_SD_RECORDING && sd_open && opens==1,"SD abre al solicitar grabacion");
    check(file_content.find(KART_FRONTEND_VERSION)!=std::string::npos && file_content.find("AccelADC")!=std::string::npos,"CSV versionado con cabeceras reales");
    check(sd_rows==1,"Primera fila registrada");
    Kart_ToggleRecording();Kart_SdTick();check(sd_state==KART_SD_STOPPED && !sd_open && closes==1,"Pausa sincroniza y cierra");
    Kart_ToggleRecording();tick+=200;Kart_SdTick();check(sd_state==KART_SD_RECORDING && opens==2 && sd_rows==2,"Reanudar vuelve a montar y abrir");
    reset();mount_result=1;Kart_ToggleRecording();Kart_SdTick();check(sd_state==KART_SD_ERROR && !sd_open,"Fallo montaje visible");
    Kart_SdTick();check(mounts==1,"Fallo SD no reintenta en bucle rapido");
    tick+=1000;mount_result=0;Kart_SdTick();check(sd_state==KART_SD_RECORDING,"SD puede recuperar al reinsertar");
    reset();short_write=1;Kart_ToggleRecording();Kart_SdTick();check(sd_state==KART_SD_ERROR && !sd_open && sd_rows==0,"Escritura parcial es error y no fila valida");
    reset();Kart_ToggleRecording();Kart_SdTick();sync_result=1;tick+=1001;Kart_SdTick();check(sd_state==KART_SD_ERROR && !sd_open,"Error sync cierra y se publica");
    reset();Kart_ToggleRecording();Kart_SdTick();close_result=1;Kart_ToggleRecording();Kart_SdTick();check(sd_state==KART_SD_ERROR,"Error cierre no se oculta como pausa");
    reset();fakeDwt.CYCCNT=0xffff0000;Kart_HallEdge(HALL_DI_ESQ_Pin);tick+=100;fakeDwt.CYCCNT+=21600000;Kart_HallEdge(HALL_DI_ESQ_Pin);feedback();Kart_ControlTick();
    check(ui.rpm_front_left==150,"Hall usa periodo real a traves de wrap DWT");
    tick+=501;Kart_ControlTick();check(ui.rpm_front_left==0,"Hall no revive por wrap DWT sin pulsos");
    KartUiSnapshot s={};s.mode_received=s.mode_applied=255;char lines[5][41]={};Kart_FormatDisplay(&s,lines);
    check(strstr(lines[0],"REC:---") && strstr(lines[1],"SIN DATOS") && strstr(lines[3],"BMS:--"),"Pantalla no inventa modo ni bateria ni CAN");
    s.feedback_fresh=s.status_fresh=1;s.inhibit=17;s.sd_state=KART_SD_ERROR;s.sd_error=1;Kart_FormatDisplay(&s,lines);
    check(strstr(lines[1],"20 KM/H") && strstr(lines[3],"ERROR E:1"),"Limite R y fallo SD visibles");
    for(unsigned i=0;i<=20;i++){s.inhibit=(uint8_t)i;Kart_FormatDisplay(&s,lines);check(strlen(lines[1])<40,"Motivo cabe en pantalla");}
    printf("TOTAL %d; FALLOS %d\n",checks,failures);return failures?1:0;
}
