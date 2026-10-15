#if !BENCH_DIAGNOSTIC_ONLY
// Reutiliza las pruebas y funciones reales sin alterar el firmware F746/ESP32.
// Esta unidad NO incluye Arduino/HAL real: CAN, ADC, SD y reloj son simulados.
#include <cstdio>
#include <string>
#include "bench_logic.h"
#include "visual_scenes.h"
#include "navigation.h"
#define main benchOriginalTests
#define SystemCoreClock benchTestCoreClock
#define printf benchPrintf
#include "../../stm32_frontal/tests/frontend_test.cpp"
#undef printf
#undef SystemCoreClock
#undef main

static kart::Controller liveBackend;
int benchRunTests() {
    checks=failures=0;
    return benchOriginalTests();
}
void benchBegin() {
    reset();
    liveBackend=kart::Controller();
}
BenchState benchTick(uint32_t now, bool pressed) {
    tick=now;
    pins[1]=pressed?1:0;
    // Banco neutral: freno activo, pedal libre, sin movimiento. Feedback sintetico.
    pins[0]=pins[2]=pins[3]=0;
    feedback();
    Kart_ControlTick();
    kart::Sensors sensors;
    sensors.speed_valid=true;
    sensors.yaw_valid=sensors.pulse_left_recent=sensors.pulse_right_recent=true;
    liveBackend.receive(sent_command,8,now);
    const auto output=liveBackend.step(sensors,now);
    return {now,uint8_t(pressed),uint8_t(start_sent()),uint8_t(output.relay),
            uint8_t(output.selected_mode),uint8_t(output.inhibit),checks,failures};
}

// Entradas sinteticas al decodificador CAN real y a HAL/FATFS simulados.
// No genera movimiento, no simula dinamica ni acredita salida real del ESP32.
static void sceneFeedback(int speed,int left,int right,int yaw,uint8_t mode,
                          uint8_t applied,uint8_t cause,bool stationary) {
    uint8_t bytes[8]={};
    kart_put_u16be(bytes,speed);kart_put_u16be(bytes+2,left);
    kart_put_u16be(bytes+4,right);kart_put_u16be(bytes+6,yaw);rx(KART_CAN_FEEDBACK,bytes);
    const uint8_t flags=KART_STATUS_SPEED_VALID|KART_STATUS_YAW_VALID|
        KART_STATUS_FRONTEND_HEALTH|(stationary?KART_STATUS_STATIONARY:0)|
        (cause==4 || cause==5?0:KART_STATUS_READY)|(cause==17?KART_STATUS_REVERSE_LIMIT:0);
    const uint8_t status[8]={1,cause,mode,applied,flags,uint8_t(cause==4?0:cause==5?1:3),0,mode};
    rx(KART_CAN_STATUS,status);
}
bool benchSpeedVisible(const KartUiSnapshot& s) {
    // Mismo criterio que Screen1View::updateDisplayData del frontal F746.
    return s.feedback_fresh && (!s.status_seen ||
        (s.status_fresh && (s.backend_flags&KART_STATUS_SPEED_VALID)));
}
BenchScene benchScene(unsigned index) {
    static const char* titles[]={"REPOSO","PRECARGA","MARCHA DIF","MARCHA BLQ","MARCHA ESC",
        "LIMITE REVERSA","FRENADO","PERDIDA CAN","ERROR SD","CAMBIO RECHAZADO"};
    index%=BENCH_SCENE_COUNT;reset();
    const uint8_t mode=index==3?2:index==4?1:0;
    const uint8_t cause=index==0?4:index==1?5:index==5?17:index==6?8:0;
    const uint8_t applied=index<2?255:mode;
    sceneFeedback(0,0,0,0,mode,applied,cause,true);Kart_ControlTick();
    for(unsigned n=0;n<mode;n++){Kart_RequestNextMode();Kart_ControlTick();}
    if(index!=0) {
        arm_button();press_start();button(0,0);button(0,40);
        // Helpers de regresion inyectan feedback neutro: reponer la escena.
        sceneFeedback(0,0,0,0,mode,applied,cause,true);Kart_ControlTick();
    }
    if(index>=2) {
        pins[index==5?3:2]=1;Kart_ControlTick(); // D/R antes de mover, freno y pedal libre.
        pins[0]=index==6?0:1;adc_accel_value=index==6?124:800;
        tick+=100;
        const int speed=index==3?18:index==4?22:index==5?20:index==6?0:24;
        sceneFeedback(speed,index==3?250:index==6?0:320,
            index==3?250:index==6?0:338,index==4?35:0,mode,applied,cause,index==6);
        Kart_ControlTick();
    }
    if(index==2 || index==3 || index==4 || index==8) {
        if(index==8)mount_result=1;
        Kart_ToggleRecording();Kart_SdTick();Kart_ControlTick();
    }
    if(index==7) {tick+=501;Kart_ControlTick();}
    if(index==9) {Kart_RequestNextMode();Kart_ControlTick();}
    BenchScene scene={};scene.title=titles[index];Kart_CopySnapshot(&scene.snapshot);
    Kart_FormatDisplay(&scene.snapshot,scene.lines);
    // Conservar version del nucleo frontal y avisos de desarrollo del formateador.
    return scene;
}
int benchVisualTests() {
    const int previous=checks;
    for(unsigned i=0;i<BENCH_SCENE_COUNT;i++) {
        const auto scene=benchScene(i);const auto& s=scene.snapshot;
        check(!s.soc_valid && !s.temp_valid && strstr(scene.lines[3],"BMS:--"),"Escena conserva BMS desconocido");
        check(benchSpeedVisible(s)==(i!=7),"Escena aplica validez de velocidad del frontal");
        check(s.gear==(i<2 || i==7?0:i==5?2:1),"Marcha sale de logica frontal con interbloqueo");
        check(s.mode_requested==(i==3?2:i==4?1:0),"DIF BLQ ESC solicitados por API real en N");
        if(i==1)check(s.start && s.inhibit==5 && s.mode_applied==255,"Precarga no presenta modo aplicado");
        if(i==2)check(s.sd_state==KART_SD_RECORDING && s.sd_rows==1,"Grabacion simulada abre y registra una fila");
        if(i==5)check(s.speed_kph==20 && s.inhibit==17 && strstr(scene.lines[1],"LIMITE R ACTIVO") && strstr(scene.lines[2],"R:LIM"),"Reversa muestra actuacion sintetica sin inventar umbral CAN");
        if(i==6)check(s.brake && s.inhibit==8,"Frenado conserva indicador y motivo");
        if(i==7)check(!s.feedback_fresh && !s.status_fresh && s.mode_received==255 && strstr(scene.lines[1],"SIN DATOS"),"CAN caducado oculta datos antiguos");
        if(i==8)check(s.sd_state==KART_SD_ERROR && s.sd_error==1,"Fallo FATFS simulado llega a pantalla");
        if(i==9)check(s.ui_notice==KART_UI_REQUIRE_N && s.mode_requested==0 && strstr(scene.lines[4],"MODO: PONER N"),"Cambio en D rechazado visible sin alterar DIF ni ocultar motivo");
    }
    auto s=benchScene(2).snapshot;s.backend_flags&=~KART_STATUS_SPEED_VALID;
    check(!benchSpeedVisible(s),"CAN fresco sin SPEED_VALID oculta velocidad");
    s=benchScene(2).snapshot;s.status_fresh=0;
    check(!benchSpeedVisible(s),"Estado viejo con feedback fresco oculta velocidad");
    BenchNavigation nav;nav.reset(0);
    check(!nav.sample(true,1000) && !nav.sample(false,1001) && !nav.sample(false,1041),"B1 sostenido al arrancar exige soltar sin avanzar");
    bool noise=false;for(unsigned n=0;n<100;n++){noise|=nav.sample(true,1050+n*10);noise|=nav.sample(false,1055+n*10);}
    check(!noise,"B1 cien rebotes cortos no avanzan");
    nav.sample(false,2100);nav.sample(true,2200);nav.sample(true,2240);
    check(!nav.sample(true,9000),"B1 mantenido no repite");
    nav.sample(false,9001);check(nav.sample(false,9041) && !nav.sample(false,9100),"B1 una liberacion estable avanza una vez");
    nav.reset(0xfffffff0u);nav.sample(false,0xfffffff0u);nav.sample(false,24);
    nav.sample(true,30);nav.sample(true,70);nav.sample(false,80);
    check(nav.sample(false,120),"B1 cruza desbordamiento de milisegundos");
    benchPrintf("VISUAL_CHECKS %d; TOTAL %d; FALLOS %d\n",checks-previous,checks,failures);
    return failures;
}
#endif
