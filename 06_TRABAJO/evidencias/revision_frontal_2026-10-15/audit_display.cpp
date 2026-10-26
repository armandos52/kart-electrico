// ENS-0017: comprobaciones de aceptación adicionales; no modifica firmware.
// Reutiliza las funciones reales y los periféricos simulados del arnés existente.
#define main existing_frontend_tests_main
#include "../../../07_DESARROLLO/firmware/stm32_frontal/tests/frontend_test.cpp"
#undef main

static void dump_display(const char* scenario) {
    char lines[5][41]={};Kart_FormatDisplay(&ui,lines);
    printf("SCENARIO %s\n",scenario);
    for(unsigned i=0;i<5;i++)printf("LINE %u %s\n",i,lines[i]);
}
int main() {
    char lines[5][41]={};
    reset();feedback();Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    printf("SOURCE_VERSION %s\nDISPLAY_VERSION %s\n",KART_FRONTEND_VERSION,lines[4]);
    check(strstr(lines[4],"V1.3-DEV3")!=nullptr,
          "F01: version visible coincide con frontend dev3");

    reset();const uint8_t legacy[8]={};rx(KART_CAN_FEEDBACK,legacy);Kart_ControlTick();
    dump_display("ESP_LEGADO_SIN_0x124");Kart_FormatDisplay(&ui,lines);
    check(!ui.status_seen && strstr(lines[2],"R:20KM/H")==nullptr,
          "F02: no atribuir limite R20 a controlador legado sin estado extendido");

    reset();feedback();pins[2]=1;Kart_RequestNextMode();Kart_ControlTick();
    const uint8_t charger[8]={1,6,0,255,4|16|8,0,0,0};rx(KART_CAN_STATUS,charger);
    tick+=100;Kart_ControlTick();dump_display("MODO_RECHAZADO_Y_CARGADOR");
    Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[1],"CARGADOR CONECTADO")!=nullptr,
          "F03: causa de inhibicion prioritaria visible durante aviso de modo");
    tick+=3001;rx(KART_CAN_FEEDBACK,legacy);rx(KART_CAN_STATUS,charger);Kart_ControlTick();
    Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[1],"CARGADOR CONECTADO")!=nullptr,
          "F03 control: la causa aparece despues de caducar aviso temporal");
    printf("AUDIT TOTAL %d; FALLOS %d\n",checks,failures);
    return failures?1:0;
}
