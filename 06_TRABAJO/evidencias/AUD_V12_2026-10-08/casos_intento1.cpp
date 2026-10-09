// Casos aislados ejecutados sobre los cuerpos originales del firmware v1.2.
// "reproduced" comprueba la observación descrita, NO la corrección del firmware.
void base() {
    fake_ms=1000;
    std::fill(std::begin(pins),std::end(pins),0);
    std::fill(std::begin(duties),std::end(duties),255);
    pwm_writes.clear();
    estado_relays=PRONTO; pins[RELAY2_PIN]=HIGH;
    tempo_relay=0;start_anterior=true;
    acelerador_raw=4095;direcao_raw=2047;travao=0;modo_d=1;modo_r=0;
    modo_controlo=0;modo_anterior=0;
    rpm_esq=rpm_dir=rpm_medio=velocidade_kph=0;
    pulsos_esq=pulsos_dir=0;last_hall_calc=fake_ms;
    pwm_atual=255;last_ramp_time=fake_ms-5;
    motor_esq_speed=motor_dir_speed=255;
    command_received=true;emergency_stop=false;last_command_time=fake_ms;
    mpu_ok=true;control_ready=true;can_init_ok=true;
    drive_inhibit="none";yaw_rate=0;
    CAN.pending=false;CAN.next_len=8;CAN.next_id=0x100;CAN.next_result=CAN_OK;
}
int failed=0;
void result(const char* name,bool reproduced){
    std::printf("{\"case\":\"%s\",\"reproduced\":%s,\"pwm_left\":%d,\"pwm_right\":%d,\"inhibit\":\"%s\"}\n",
        name,reproduced?"true":"false",motor_esq_speed,motor_dir_speed,drive_inhibit);
    if(!reproduced)failed++;
}
void command(byte mode=0,byte brake=0,uint16_t accel=4095){
    CAN.pending=true;
    byte data[8]={byte(accel>>8),byte(accel&255),7,255,brake,1,0,byte(mode|4)};
    std::memcpy(CAN.next_data,data,8);
}
int main(){
    base();processarComando();
    std::printf("{\"detail\":\"rampa_5ms\",\"pwm_atual\":%.6f,\"pwm_salida\":%d}\n",pwm_atual,motor_esq_speed);
    result("rampa_no_aplicada",pwm_atual>254 && motor_esq_speed==0);

    base();acelerador_raw=2048;direcao_raw=2247;modo_controlo=0;processarComando();
    int dif_l=motor_esq_speed,dif_r=motor_dir_speed;
    modo_controlo=2;processarComando();
    result("DIF_igual_BLQ_en_zona_central",motor_esq_speed==dif_l&&motor_dir_speed==dif_r);

    base();direcao_raw=3005;modo_d=0;modo_r=1;modo_anterior=1;processarComando();
    result("marcha_R_sin_limite_1_kmh",motor_esq_speed<255&&motor_dir_speed<255&&pins[SW_DIR_MT_DIR]==HIGH);

    base();acelerador_raw=2048;modo_controlo=1;direcao_raw=3005;yaw_rate=30;processarComando();
    int esc_pos_l=motor_esq_speed,esc_pos_r=motor_dir_speed;
    direcao_raw=1089;yaw_rate=-30;processarComando();
    std::printf("{\"detail\":\"ESC_espejo\",\"positivo\":[%d,%d],\"negativo\":[%d,%d]}\n",esc_pos_l,esc_pos_r,motor_esq_speed,motor_dir_speed);
    result("ESC_asimetrico_con_giro_y_yaw_invertidos",motor_esq_speed!=esc_pos_r||motor_dir_speed!=esc_pos_l);

    base();travao=1;processarComando();
    result("freno_1_corta_PWM",motor_esq_speed==255&&motor_dir_speed==255);
    base();travao=2;processarComando();
    result("freno_2_invalido_no_rechazado",motor_esq_speed==0);
    base();modo_d=modo_r=1;processarComando();
    result("D_y_R_simultaneos_prioriza_D",pins[SW_DIR_F_DIR]==HIGH&&motor_esq_speed==0);
    base();modo_controlo=3;processarComando();
    result("modo_3_invalido_se_usa_como_BLQ",motor_esq_speed==0&&motor_dir_speed==0);
    base();acelerador_raw=65535;processarComando();
    result("acelerador_CAN_fuera_rango_pleno",motor_esq_speed==0);
    base();modo_controlo=1;mpu_ok=false;processarComando();
    result("ESC_sin_MPU_inicial_corta_PWM",motor_esq_speed==255&&std::strcmp(drive_inhibit,"mpu_unavailable")==0);

    base();modo_d=modo_r=0;processarComando();
    bool torque_write=false;
    for(const auto& w:pwm_writes) if((w.pin==MOTOR_PWM_ESQ||w.pin==MOTOR_PWM_DIR)&&w.value<255)torque_write=true;
    result("PWM_se_escribe_antes_de_resolver_N",torque_write&&motor_esq_speed==255);

    base();estado_relays=DESLIGADO;pins[RELAY2_PIN]=LOW;start_anterior=false;
    processarSequenciaRelays(true);fake_ms+=100;processarSequenciaRelays(false);
    fake_ms+=1000;processarSequenciaRelays(false);
    result("STOP_temprano_perdido",estado_relays==LIGOU_RELE1&&pins[RELAY1_PIN]==HIGH&&!start_anterior);

    base();processarComando();fake_ms+=101;loop();
    result("timeout_CAN_corta_PWM",emergency_stop&&motor_esq_speed==255);
    command();loop();
    result("reanudacion_CAN_con_pedal_pisado",!emergency_stop&&motor_esq_speed==0);

    base();processarComando();loop(); // activa temporizador del cargador
    fake_ms+=2001;last_command_time=fake_ms;pins[PIN_CARREGADOR]=HIGH;loop();
    result("cargador_abre_reles_pero_PWM_espera_otra_orden_o_timeout",estado_relays==DESLIGADO&&pins[RELAY2_PIN]==LOW&&motor_esq_speed==0);

    base();pulsos_esq=89;pulsos_dir=68;fake_ms+=1000;atualizarRPM();
    std::printf("{\"detail\":\"ventana_RPM_1000ms_1_vuelta\",\"rpm_left\":%d,\"rpm_right\":%d,\"referencia_matematica\":60}\n",rpm_esq,rpm_dir);
    result("RPM_divide_por_500ms_aunque_pasa_1000ms",rpm_esq==120&&rpm_dir==120);
    base();rpm_esq=rpm_dir=30;fake_ms+=500;atualizarRPM();
    result("Hall_sin_pulsos_se_interpreta_cero_sin_validez",rpm_medio==0);

    base();command();CAN.next_result=2;loop();
    result("CAN_error_lectura_no_aplica_mando",motor_esq_speed==255);
    base();command();CAN.next_len=7;loop();
    result("CAN_DLC7_no_aplica_mando",motor_esq_speed==255);
    std::printf("{\"summary\":\"observaciones_sinteticas_host\",\"cases\":19,\"failed_reproductions\":%d}\n",failed);
    return failed?1:0;
}
