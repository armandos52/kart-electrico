// ENS-0022: página «Condiciones de marcha». Funciones reales del frontal y CAN/pines simulados.
// Los escenarios son sintéticos: no son medidas del kart.
#include "../TouchGFX/gui/include/gui/common/KartFont.h"
static bool conditions_fit(const char lines[KART_CONDITION_ROWS][41],const uint8_t* tones) {
    for(unsigned row=0;row<KART_CONDITION_ROWS;row++) {
        const char* end=(const char*)memchr(lines[row],0,41);
        if(!end || end==lines[row] || tones[row]>KART_TONE_FAIL)return false;
        for(const char* p=lines[row];p<end;p++)if(!strchr(kartfont::symbols,*p))return false;
    }
    return true;
}
// Lógica de engranar D/R tal como estaba antes de extraer las máscaras (dev5), para comparar el resultado.
static uint8_t reference_gear(uint8_t previous,int drive,int reverse,int brake,int pedal_free,int stationary,int inputs_ok,int feedback_ok) {
    uint8_t gear=previous;
    if((drive && gear==2) || (reverse && gear==1))gear=0;
    if(!inputs_ok || !feedback_ok || (!drive && !reverse))gear=0;
    else if(drive && gear!=1 && brake && pedal_free && stationary)gear=1;
    else if(reverse && gear!=2 && brake && pedal_free && stationary)gear=2;
    return gear;
}
static uint8_t reference_mode_notice(int inputs_ok,int feedback_ok,int drive,int reverse,uint8_t gear,int stationary,int pedal_free) {
    if(!inputs_ok)return KART_UI_INPUT_BAD;
    if(!feedback_ok)return KART_UI_CAN_OLD;
    if(drive || reverse || gear)return KART_UI_REQUIRE_N;
    if(!stationary || !pedal_free)return KART_UI_REQUIRE_STOP;
    return KART_UI_OK;
}
static void send_status(bool stationary) {
    const uint8_t status[8]={1,10,0,255,uint8_t(KART_STATUS_SPEED_VALID|KART_STATUS_YAW_VALID|(stationary?KART_STATUS_STATIONARY:0)),3,0,0};
    rx(KART_CAN_STATUS,status);
}
static void test_conditions() {
    char lines[KART_CONDITION_ROWS][41]={};uint8_t tones[KART_CONDITION_ROWS]={};

    // C01: caso de PROB-0012. Reposo de 149 cuentas, todo lo demás correcto: solo bloquea el pedal.
    reset();feedback();pins[2]=1;adc_accel_value=149;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && ui.accel_adc==149 && ui.accel_raw==58 && !ui.pedal_free,"C01 reposo de 149 cuentas queda sobre el umbral y no engrana");
    check(strcmp(lines[1],"D/R: FALTA PEDAL")==0 && tones[1]==KART_TONE_FAIL,"C01 resumen D/R nombra solo el pedal");
    check(strcmp(lines[2],"PEDAL LIBRE:NO ADC:149 NORM:58 MAX:40")==0 && tones[2]==KART_TONE_FAIL,"C01 pedal muestra ADC fisico normalizado y umbral");
    check(strcmp(lines[3],"FRENO PISADO: SI")==0 && tones[3]==KART_TONE_OK && strcmp(lines[4],"SELECTOR: D")==0 && tones[4]==KART_TONE_OK,
          "C01 freno y selector se muestran cumplidos por separado");
    check(tones[5]==KART_TONE_OK && tones[6]==KART_TONE_OK && strstr(lines[7],"PARADO: SI") && tones[7]==KART_TONE_OK,
          "C01 CAN 0x123 CAN 0x124 y quietud cumplidos");
    check(strcmp(lines[10],"MODO FALTA: N PEDAL")==0,"C01 el pedal y el selector fuera de N bloquean el cambio de modo");

    // C02: pedal libre -> D engrana y la página lo dice.
    reset();feedback();pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==1 && strcmp(lines[1],"D/R: D ACTIVA")==0 && tones[1]==KART_TONE_OK && strstr(lines[2],"PEDAL LIBRE:SI"),
          "C02 con todas las condiciones D engrana y se muestra activa");
    check(ui.drive_cond==KART_DRIVE_COND_ALL && strcmp(lines[10],"MODO FALTA: N")==0,"C02 con D activa el modo exige volver a N");

    // C03: sin freno.
    reset();feedback();pins[2]=1;pins[0]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && strcmp(lines[1],"D/R: FALTA FRENO")==0 && strcmp(lines[3],"FRENO PISADO: NO")==0 && tones[3]==KART_TONE_FAIL,
          "C03 sin freno solo falta el freno");

    // C04: en movimiento segun 0x124 (SPEED_VALID sin STATIONARY).
    reset();feedback();send_status(false);pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && strcmp(lines[1],"D/R: FALTA PARADO")==0 && strcmp(lines[7],"PARADO: NO SEGUN 0X124")==0,
          "C04 en movimiento segun 0x124 impide engranar");

    // C05: sin 0x124 (ESP32 sin estado extendido). Quietud por RPM traseras; 0x124 se marca como nunca recibido.
    reset();const uint8_t still[8]={};rx(KART_CAN_FEEDBACK,still);pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==1 && strcmp(lines[6],"CAN 0X124: NUNCA RECIBIDO")==0 && tones[6]==KART_TONE_WARN && strcmp(lines[7],"PARADO: SI RPM 0/0")==0,
          "C05 sin 0x124 usa RPM y deja 0x124 como nunca recibido");
    reset();const uint8_t rolling[8]={0,0,0,100,0,5,0,0};rx(KART_CAN_FEEDBACK,rolling);pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && strcmp(lines[7],"PARADO: NO RPM 100/5")==0 && strcmp(lines[1],"D/R: FALTA PARADO")==0,
          "C05 sin 0x124 las RPM traseras distintas de cero impiden engranar");

    // C06: CAN caducado.
    reset();feedback();tick+=501;pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && strcmp(lines[5],"CAN 0X123: CADUCADO 501 MS LIM 500")==0 && strcmp(lines[6],"CAN 0X124: CADUCADO 501 MS LIM 300")==0,
          "C06 muestra edad y limite de cada trama caducada");
    check(strcmp(lines[1],"D/R: FALTA CAN PARADO")==0 && strstr(lines[7],"SIN FEEDBACK RECIENTE") && strstr(lines[9],"SIN DATOS CAN RECIENTES"),
          "C06 CAN caducado bloquea tambien la quietud y lo dice la causa");

    // C06b: feedback 0x123 caducado pero estado 0x124 reciente: la quietud tampoco se acredita sin 0x123.
    reset();feedback();tick+=400;send_status(true);tick+=150;pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(!ui.feedback_fresh && ui.status_fresh && ui.gear==0 && !ui.stationary,"C06b feedback caducado con estado fresco no acredita quietud");
    check(strcmp(lines[1],"D/R: FALTA CAN PARADO")==0 && strstr(lines[7],"SIN FEEDBACK RECIENTE") && strcmp(lines[5],"CAN 0X123: CADUCADO 550 MS LIM 500")==0 &&
          strcmp(lines[6],"CAN 0X124: OK 150 MS LIM 300")==0,"C06b la pagina distingue 0x123 caducado de 0x124 vigente");

    // C07: nunca hubo CAN.
    reset();pins[2]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(strcmp(lines[5],"CAN 0X123: NUNCA RECIBIDO")==0 && tones[5]==KART_TONE_FAIL && ui.gear==0,"C07 sin ninguna trama se muestra nunca recibido");

    // C08/C09: selector invalido y en N.
    reset();feedback();pins[2]=pins[3]=1;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && strcmp(lines[1],"D/R: SELECTOR INVALIDO D Y R")==0 && strcmp(lines[4],"SELECTOR: D Y R A LA VEZ INVALIDO")==0 && strstr(lines[8],"SEL:FALLO"),
          "C08 D y R a la vez es selector invalido y entrada con fallo");
    reset();feedback();Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(strcmp(lines[1],"D/R: SELECTOR EN N SIN SOLICITUD")==0 && strcmp(lines[4],"SELECTOR: N SIN SOLICITUD D/R")==0,"C09 selector en N no pide marcha");

    // C10: ADC de direccion fuera de rango bloquea D/R aunque el pedal este bien.
    reset();feedback();pins[2]=1;adc_steer_value=0;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==0 && strcmp(lines[1],"D/R: FALTA ENTR")==0 && strstr(lines[8],"DIR:FALLO") && strstr(lines[8],"PEDAL:OK"),
          "C10 ADC de direccion invalido se ve como entradas");
    reset();feedback();pins[2]=1;adc_accel_value=0;Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(strcmp(lines[2],"PEDAL LIBRE: -- ADC INVALIDO")==0 && ui.gear==0,"C10 ADC de pedal invalido no se presenta como pedal libre");

    // C11: ya en D, soltar freno y pisar el pedal no desengrana y no alarma como si faltaran condiciones de entrada.
    reset();feedback();pins[2]=1;Kart_ControlTick();
    pins[0]=1;adc_accel_value=1861;tick+=5;feedback();Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
    check(ui.gear==1 && strcmp(lines[1],"D/R: D ACTIVA")==0 && strcmp(lines[2],"PEDAL LIBRE:NO ADC:1861 NORM:4095 MAX:40")==0 && tones[2]==KART_TONE_TEXT && tones[3]==KART_TONE_TEXT,
          "C11 con D activa freno y pedal sin alarma y con su valor real");

    // C12: peores casos de longitud sin recorte.
    KartUiSnapshot w={};w.feedback_fresh=w.status_fresh=0;w.status_seen=1;w.can_age=w.status_age=100000;w.input_flags=KART_INPUT_ACCEL_OK;
    w.accel_adc=1961;w.accel_raw=4095;w.drive_input=1;w.drive_cond=0;w.mode_cond=0;w.rpm_left=w.rpm_right=2000;
    Kart_FormatConditions(&w,lines,tones);
    check(strcmp(lines[1],"D/R: FALTA ENTR CAN FRENO PEDAL PARADO")==0 && strcmp(lines[2],"PEDAL LIBRE:NO ADC:1961 NORM:4095 MAX:40")==0 &&
          strcmp(lines[6],"CAN 0X124: CADUCADO MAS DE 99 S LIM 300")==0 && strcmp(lines[10],"MODO FALTA: ENTR CAN N PARADO PEDAL")==0,
          "C12 peores casos caben enteros en 40 columnas");
    check(conditions_fit(lines,tones),"C12 peor caso usa solo glifos disponibles");
    KartUiSnapshot zero={};Kart_FormatConditions(&zero,lines,tones);check(conditions_fit(lines,tones),"C12 snapshot vacio se formatea sin vacios ni desbordes");

    // C13: equivalencia exhaustiva. El control con máscaras decide igual que la lógica anterior y la página no miente.
    bool gear_same=true,mode_same=true,fits=true,tokens_ok=true,engaged_ok=true;unsigned cases=0;
    for(unsigned prev=0;prev<3;prev++)for(unsigned bits=0;bits<128;bits++) {
        const int drive=bits&1,reverse=(bits>>1)&1,brake=(bits>>2)&1,pedal=(bits>>3)&1,stationary=(bits>>4)&1,steer_ok=(bits>>5)&1,fresh=(bits>>6)&1;
        reset();feedback();send_status(stationary);if(!fresh)tick+=501;
        pins[0]=brake?0:1;pins[2]=drive;pins[3]=reverse;adc_accel_value=pedal?124:300;adc_steer_value=steer_ok?2177:0;
        selected_gear=(uint8_t)prev;Kart_RequestNextMode();Kart_ControlTick();Kart_FormatConditions(&ui,lines,tones);
        const int inputs_ok=steer_ok && !(drive && reverse),feedback_ok=fresh,stopped=fresh && stationary;
        const uint8_t expected=reference_gear(uint8_t(prev),drive,reverse,brake,pedal,stopped,inputs_ok,feedback_ok);
        gear_same&=ui.gear==expected;
        const uint8_t expected_notice=reference_mode_notice(inputs_ok,feedback_ok,drive,reverse,expected,stopped,pedal);
        mode_same&=ui.ui_notice==expected_notice && ui.mode_requested==(expected_notice==KART_UI_OK?1:0) && (ui.mode_cond==KART_MODE_COND_ALL)==(expected_notice==KART_UI_OK);
        fits&=conditions_fit(lines,tones);
        // El resumen solo engrana cuando todas las condiciones de entrada se cumplen (o ya estaba engranada).
        if(ui.gear==0 && (drive!=reverse) && inputs_ok && feedback_ok) {
            tokens_ok&=(strstr(lines[1],"FRENO")!=nullptr)==!brake;
            tokens_ok&=(strstr(lines[1],"PEDAL")!=nullptr)==!pedal;
            tokens_ok&=(strstr(lines[1],"PARADO")!=nullptr)==!stopped;
        }
        if(ui.gear!=0)engaged_ok&=strncmp(lines[1],"D/R: ",5)==0 && strstr(lines[1],"ACTIVA")!=nullptr && tones[1]==KART_TONE_OK;
        cases++;
    }
    check(cases==384,"C13 recorre 384 combinaciones de selector freno pedal quietud ADC CAN y marcha previa");
    check(gear_same,"C13 la marcha resultante coincide con la logica anterior en todas las combinaciones");
    check(mode_same,"C13 el cambio de modo y su aviso coinciden con la logica anterior y con mode_cond");
    check(tokens_ok,"C13 FRENO PEDAL y PARADO aparecen en el resumen exactamente cuando faltan");
    check(engaged_ok,"C13 toda marcha engranada se presenta como activa");
    check(fits,"C13 las once filas usan solo glifos disponibles y caben en todas las combinaciones");

    // C14: tabla de glifos y tramos de la fuente propia.
    check(kartfont::glyphCount==strlen(kartfont::symbols),"C14 hay un glifo por simbolo del alfabeto");
    bool runs_same=true;
    for(unsigned g=0;g<kartfont::glyphCount;g++) {
        unsigned char drawn[7]={};
        kartfont::forEachRun(kartfont::glyphs[g],[&](unsigned y,unsigned x,unsigned n){for(unsigned i=0;i<n;i++)drawn[y]|=(unsigned char)(16>>(x+i));});
        runs_same&=memcmp(drawn,kartfont::glyphs[g],7)==0;
    }
    check(runs_same,"C14 los tramos horizontales reproducen cada glifo punto a punto");
}
