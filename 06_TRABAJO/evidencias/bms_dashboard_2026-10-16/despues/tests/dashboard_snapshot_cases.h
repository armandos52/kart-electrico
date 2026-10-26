// GPIO/CAN/ADC/Hall simulados, lógica real. No son medidas físicas.
static void test_dashboard_snapshot(){
    reset();Kart_ControlTick();
    check(!ui.feedback_usable && ui.front_left_age==UINT32_MAX && !ui.front_left_valid && ui.relay_state==255,
          "Dashboard: ausencia CAN/Hall explicita");
    feedback();pins[2]=1;adc_accel_value=170;Kart_ControlTick();
    check(ui.accel_raw==108 && !ui.pedal_free && ui.drive_input && !ui.reverse_input && ui.gear==0,
          "Dashboard: ADC 170 reproduce bloqueo por pedal sin alterar umbral");
    adc_accel_value=124;Kart_ControlTick();
    check(ui.pedal_free && ui.stationary && ui.feedback_usable && ui.gear==1 && ui.relay_state==3,
          "Dashboard: condiciones reflejan permiso D y rele logico recibido");
    pins[1]=1;Kart_ControlTick();check(ui.start_input && !ui.start,"Dashboard: pin START distinto de peticion enclavada");
    received.left=120;received.status[4]=KART_STATUS_SPEED_VALID;adc_accel_value=500;pins[0]=1;Kart_ControlTick();
    check(ui.gear==1 && !ui.stationary && !ui.pedal_free && !ui.brake,
          "Dashboard: D ya seleccionado permite pedal con movimiento");
    tick+=301;received.feedback_tick=tick;Kart_ControlTick();
    check(!ui.feedback_usable && ui.gear==0 && ui.relay_state==255,"Dashboard: estado caducado no permite D ni rele confirmado");
    reset();const uint8_t zero[8]={};rx(0x123,zero);Kart_ControlTick();
    check(ui.feedback_usable && ui.stationary && !ui.status_seen && ui.relay_state==255,
          "Dashboard: legado fresco mantiene compatibilidad sin inventar estado ESP");
    fakeDwt.CYCCNT=100000;Kart_HallEdge(HALL_DI_ESQ_Pin);Kart_ControlTick();
    check(!ui.front_left_valid && ui.front_left_age==0,"Dashboard: un pulso Hall no determina RPM");
    tick+=100;fakeDwt.CYCCNT+=21600000;Kart_HallEdge(HALL_DI_ESQ_Pin);Kart_ControlTick();
    check(ui.front_left_valid && ui.rpm_front_left==150 && !ui.front_right_valid,"Dashboard: periodo valido solo para rueda observada");
    tick+=501;Kart_ControlTick();
    check(!ui.front_left_valid && ui.front_left_age==501,"Dashboard: Hall caducado deja de ser medida valida");
}
