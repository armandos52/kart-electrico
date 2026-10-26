// ENS-0026: perifericos simulados, muestras ADC reales reproducidas en frontend dev8.
#define main regression_main
#include "fuentes_dev8/stm32_frontal/tests/frontend_test.cpp"
#undef main
int main() {
    for(unsigned adc : {178u,190u,206u,229u}) {
        reset();feedback();adc_accel_value=(uint16_t)adc;pins[2]=1;
        Kart_ControlTick();
        check(ui.brake && ui.stationary && ui.feedback_usable && !ui.pedal_free && ui.gear==0,
              "ADC observado: D bloqueada incluso con freno y CAN correctos");
        pins[2]=0;Kart_RequestNextMode();Kart_ControlTick();
        check(ui.mode_requested==0 && ui.ui_notice==KART_UI_REQUIRE_STOP,
              "Mismo ADC: cambio DIF/ESC/BLQ bloqueado en N");
        printf("ADC=%u normalizado=%u marcha=%u modo=%u aviso=%u\n",adc,ui.accel_raw,ui.gear,ui.mode_requested,ui.ui_notice);
    }
    reset();feedback();pins[2]=1;adc_accel_value=141;Kart_ControlTick();
    check(ui.gear==1,"Control positivo: D entra con ADC 141 y freno");
    reset();feedback();pins[2]=1;adc_accel_value=142;Kart_ControlTick();
    check(ui.gear==0,"Limite: ADC 142 bloquea D");
    reset();feedback();pins[2]=1;pins[0]=1;Kart_ControlTick();
    check(ui.gear==0,"Sin freno D sigue bloqueada aunque pedal sea libre");
    printf("TOTAL %d; FALLOS %d\n",checks,failures);
    return failures?1:0;
}
