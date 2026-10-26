// ENS-0026: regresión de reposo real, límites y protecciones; periféricos simulados.
static void test_pedal_zero(){
    bool all_rest=true;
    for(unsigned adc=156;adc<=302;adc++) {
        reset();feedback();adc_accel_value=(uint16_t)adc;pins[2]=1;Kart_ControlTick();
        all_rest &= ui.input_flags==7 && ui.pedal_free && ui.gear==1 && ui.accel_raw==0 && kart_u16be(sent_command)==0;
    }
    check(all_rest,"Pedal: intervalo de reposo medido permite D con mando exactamente cero");
    reset();feedback();adc_accel_value=302;Kart_RequestNextMode();Kart_ControlTick();
    check(ui.mode_requested==1 && ui.ui_notice==KART_UI_OK,"Pedal: reposo medido permite cambiar modo en N");
    reset();feedback();adc_accel_value=302;pins[3]=1;Kart_ControlTick();
    check(ui.gear==2 && kart_u16be(sent_command)==0,"Pedal: R entra con freno, quietud y mando cero");
    reset();feedback();adc_accel_value=320;pins[2]=1;Kart_ControlTick();
    check(ui.gear==1 && ui.pedal_free && ui.accel_raw==0,"Pedal: limite ADC 320 pertenece a zona cero");
    reset();feedback();adc_accel_value=321;pins[2]=1;Kart_ControlTick();
    check(ui.gear==0 && !ui.pedal_free && ui.accel_raw>0,"Pedal: ADC 321 bloquea entrada D sin ampliar permiso residual");
    reset();feedback();adc_accel_value=320;pins[0]=1;pins[2]=1;Kart_ControlTick();
    check(ui.gear==0,"Pedal: sin freno no se selecciona D");
    reset();feedback();adc_accel_value=320;received.status[4]=KART_STATUS_SPEED_VALID;received.left=1;pins[3]=1;Kart_ControlTick();
    check(ui.gear==0,"Pedal: movimiento impide R aunque el pedal sea cero");
    reset();feedback();adc_accel_value=320;pins[2]=pins[3]=1;Kart_ControlTick();
    check(ui.gear==0 && ui.input_flags!=7,"Pedal: selector incoherente sigue inhibido");
    reset();feedback();adc_accel_value=320;adc_steer_value=0;pins[2]=1;Kart_ControlTick();
    check(ui.gear==0 && ui.input_flags!=7,"Pedal: fallo de direccion sigue inhibiendo");
    reset();feedback();tick+=501;adc_accel_value=320;pins[2]=1;Kart_ControlTick();
    check(ui.gear==0 && kart_u16be(sent_command)==0,"Pedal: CAN caducado sigue inhibiendo");
    bool invalid_safe=true;
    for(unsigned adc : {0u,23u,1962u,4095u,65535u}) {
        reset();feedback();adc_accel_value=(uint16_t)adc;pins[2]=1;Kart_ControlTick();
        invalid_safe &= !(ui.input_flags&KART_INPUT_ACCEL_OK) && !ui.pedal_free && ui.gear==0 && kart_u16be(sent_command)==0;
    }
    check(invalid_safe,"Pedal: zona cero no oculta ADC invalido ni declara pedal libre");
    reset();feedback();adc_accel_value=24;pins[2]=1;Kart_ControlTick();
    check(ui.input_flags==7 && ui.accel_raw==0,"Pedal: plausibilidad inferior historica se conserva separada del cero");
    reset();feedback();adc_accel_value=1961;Kart_ControlTick();
    check(ui.input_flags==7 && ui.accel_raw==4095,"Pedal: plausibilidad superior historica y saturacion se conservan");
    reset();feedback();pins[2]=1;adc_accel_value=200;Kart_ControlTick();
    uint16_t previous=0;bool monotonic=true;
    for(unsigned adc=321;adc<=1861;adc++) {
        adc_accel_value=(uint16_t)adc;Kart_ControlTick();
        monotonic &= ui.gear==1 && ui.accel_raw>=previous;previous=ui.accel_raw;
    }
    for(unsigned i=0;i<3;i++)Kart_ControlTick();
    check(monotonic && ui.accel_raw==4095,"Pedal: recorrido continuo creciente conserva marcha seleccionada y fondo tras asentarse");
    adc_accel_value=245;Kart_ControlTick();
    check(ui.gear==1 && ui.accel_raw==0 && kart_u16be(sent_command)==0,"Pedal: retorno medido corta mando en la misma muestra, sin cola de filtro");
    adc_ok=0;Kart_ControlTick();
    check(ui.gear==0 && kart_u16be(sent_command)==0 && !ui.pedal_free,"Pedal: fallo de conversion durante D inhibe inmediatamente");
}
