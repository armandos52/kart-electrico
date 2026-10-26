// ENS-0027: requisitos temporales y de mando con ADC/GPIO/CAN simulados.
static void pedal_sample(uint16_t adc,uint32_t dt=5){
    tick+=dt;feedback();adc_accel_value=adc;Kart_ControlTick();
}
static void test_pedal_filter(){
    reset();feedback();pedal_sample(1000);
    check(ui.accel_raw==2065 && !ui.pedal_free && ui.gear==0,
          "Filtro: arranque con pedal pisado no inventa reposo ni entra D");
    const uint16_t expected[]={2442,2725,2913,3008};
    bool weighted=true;
    for(unsigned i=0;i<4;i++){pedal_sample(1400);weighted &= ui.accel_raw==expected[i];}
    check(weighted,"Filtro: escalon usa pesos 4/3/2/1 y se asienta en cuatro muestras");
    check(ui.accel_adc==1400 && kart_u16be(sent_health+2)==1400,
          "Filtro: snapshot y trama de salud conservan ADC fisico sin filtrar");
    pedal_sample(200);
    check(ui.accel_raw==0 && ui.pedal_free && kart_u16be(sent_command)==0,
          "Filtro: soltar corta en primera muestra, sin cola de aceleracion");
    pedal_sample(321);pins[2]=1;Kart_ControlTick();
    check(ui.accel_raw==0 && !ui.pedal_free && ui.gear==0,
          "Filtro: promedio todavia bajo no permite D si lectura actual supera cero");
    pedal_sample(200);pedal_sample(800);
    check(ui.accel_raw==571,"Filtro: volver a pisar no reutiliza el historial de aceleracion anterior");
    pedal_sample(4095);
    check(!(ui.input_flags&KART_INPUT_ACCEL_OK) && ui.gear==0 && !ui.pedal_free && kart_u16be(sent_command)==0,
          "Filtro: una sola muestra invalida inhibe inmediatamente sin promediarla");
    pedal_sample(1400);
    check(ui.accel_raw==3008 && !ui.pedal_free && ui.gear==0,
          "Filtro: recuperacion con pedal pisado no hereda ceros ni reengancha D");
    adc_ok=0;Kart_ControlTick();
    check(ui.accel_adc==65535 && !(ui.input_flags&1) && kart_u16be(sent_command)==0,
          "Filtro: fallo HAL corta aun con historial elevado");
    adc_ok=1;pedal_sample(200);pedal_sample(1400);Kart_Init();pedal_sample(800);
    check(ui.accel_raw==1593,"Filtro: reinicializar elimina memoria anterior");
    pedal_sample(1400,20);pedal_sample(800,20);
    check(ui.accel_raw==1593,"Filtro: hueco de 20 ms descarta muestras viejas");
    reset();tick=0xfffffff0u;pedal_sample(1000,0);
    weighted=true;
    for(unsigned i=0;i<4;i++){pedal_sample(1400);weighted &= ui.accel_raw==expected[i];}
    check(weighted,"Filtro: pesos y memoria funcionan al desbordar el reloj");
    reset();pedal_sample(1000);uint16_t lowest=4095,highest=0;
    for(unsigned i=0;i<40;i++){
        pedal_sample(i%2?1040:960);
        if(i>=4){if(ui.accel_raw<lowest)lowest=ui.accel_raw;if(ui.accel_raw>highest)highest=ui.accel_raw;}
    }
    check(highest-lowest==38,"Filtro: ruido alterno sintetico de 80 ADC baja a 16 ADC, sin afirmar EMI real");
    bool continuous=true,legacy=true;uint16_t last=0;
    for(unsigned adc=24;adc<=1961;adc++) {
        reset();pedal_sample((uint16_t)adc);
        continuous &= ui.accel_raw>=last && ui.accel_raw-last<=5;
        last=ui.accel_raw;
        if(adc>=512)legacy &= ui.accel_raw==(adc>=1861?4095:(adc-124)*4095/1737);
    }
    check(continuous && last==4095,"Curva: recorrido estable monotono sin saltos mayores a 5/4095 por cuenta ADC");
    check(legacy,"Curva: respuesta estable identica a dev8 desde ADC 512 hasta fondo");
    reset();pedal_sample(800);check(ui.accel_raw==1593,"Curva: ADC 800 recupera 38,9 % de mando anterior");
    reset();pedal_sample(400);check(ui.accel_raw==381,"Curva: ADC 400 entrega 9,3 %, sin salto al salir de cero");
    reset();pedal_sample(200);pins[2]=1;Kart_ControlTick();pedal_sample(1400);
    adc_steer_value=813;Kart_ControlTick();
    check(ui.input_flags==5 && ui.gear==0 && kart_u16be(sent_command)==0,
          "Direccion: lectura 813 del tope sigue detectada, filtro pedal no oculta el fallo");
    reset();arm_button();press_start();pedal_sample(1400);arm_button();button(1,0);
    check(!ui.start && !start_sent(),"Filtro: historial del pedal no retrasa STOP");
}
