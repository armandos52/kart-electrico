// Vuelca las filas de «Condiciones de marcha» y del bloque de estado para escenarios sintéticos.
// Usa las funciones reales del frontal con CAN/pines simulados; no son medidas del kart.
// Compilar como frontend_test.cpp y ejecutar; la salida la consume tools/renderizar_condiciones.py.
#define main frontend_test_main
#include "frontend_test.cpp"
#undef main
static void dump(const char* name) {
    char cond[KART_CONDITION_ROWS][41]={};uint8_t tones[KART_CONDITION_ROWS]={};char status[5][41]={};
    Kart_FormatConditions(&ui,cond,tones);Kart_FormatDisplay(&ui,status);
    printf("SCENARIO %s\n",name);
    for(unsigned row=0;row<KART_CONDITION_ROWS;row++)printf("COND %u %u %s\n",row,tones[row],cond[row]);
    for(unsigned row=0;row<5;row++)printf("MAIN %u %s\n",row,status[row]);
}
int main() {
    reset();feedback();pins[2]=1;adc_accel_value=149;Kart_ControlTick();dump("01_reposo_pedal_149");
    // Con D engranada el ESP32 aun puede inhibir: aqui, START sin pulsar (causa 4).
    const uint8_t start_off[8]={1,4,0,255,KART_STATUS_SPEED_VALID|KART_STATUS_YAW_VALID|KART_STATUS_STATIONARY,0,0,0};
    reset();feedback();rx(KART_CAN_STATUS,start_off);pins[2]=1;Kart_ControlTick();dump("02_d_activa_start_apagado");
    const uint8_t legacy[8]={};
    reset();rx(KART_CAN_FEEDBACK,legacy);pins[2]=1;adc_accel_value=149;Kart_ControlTick();dump("03_sin_0x124_pedal_149");
    reset();feedback();tick+=501;pins[2]=1;Kart_ControlTick();dump("04_can_caducado");
    reset();feedback();send_status(false);pins[2]=1;pins[0]=1;Kart_ControlTick();dump("05_sin_freno_en_movimiento");
    reset();feedback();pins[2]=pins[3]=1;Kart_ControlTick();dump("06_selector_invalido");
    return 0;
}
