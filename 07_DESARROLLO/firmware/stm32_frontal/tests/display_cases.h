// Regresiones ENS-0018: UI de desarrollo con funciones reales y CAN simulado.
static bool display_fits(const char lines[5][41]) {
    const char* supported="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-.%/ ";
    for(unsigned row=0;row<5;row++) {
        const char* end=(const char*)memchr(lines[row],0,41);
        if(!end)return false;
        for(const char* p=lines[row];p<end;p++)if(!strchr(supported,*p))return false;
    }
    return true;
}
static void test_display_developer() {
    char lines[5][41]={};
    reset();feedback();Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    std::string visible=lines[4];
    const size_t end=visible.find(' ',4);
    std::string release=visible.substr(4,end-4);
    for(char& c:release)if(c>='A' && c<='Z')c=char(c-'A'+'a');
    check("stm32_frontal-"+release==KART_FRONTEND_VERSION,
          "F01 version visible identifica la misma version que el CSV y build");
    ui.sd_rows=UINT32_MAX;Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[4],"4294967295") && display_fits(lines),
          "F01 version y contador SD maximo caben y usan glifos disponibles");

    reset();Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[2],"R:--") && !strstr(lines[2],"20"),"F02 sin CAN no afirma limite R");
    const uint8_t legacy[8]={};rx(KART_CAN_FEEDBACK,legacy);Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(!ui.status_seen && strstr(lines[2],"R:--"),"F02 legado sin 0x124 deja limitador desconocido");
    const uint8_t incompatible[8]={2,0,0,0,32,3,2,0};rx(KART_CAN_STATUS,incompatible);
    Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(!ui.status_seen && strstr(lines[2],"R:--"),"F02 version CAN incompatible no acredita limitador");
    feedback();Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[2],"R:INACT") && !strstr(lines[2],"20"),"F02 estado compatible sin actuacion indica INACT");
    const uint8_t limited[8]={1,17,0,0,KART_STATUS_SPEED_VALID|KART_STATUS_REVERSE_LIMIT,3,2,0};
    rx(KART_CAN_STATUS,limited);Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[2],"R:LIM") && strcmp(lines[1],"LIMITE R ACTIVO")==0,
          "F02 limitacion confirmada muestra LIM sin inventar umbral numerico");
    tick+=301;rx(KART_CAN_FEEDBACK,legacy);Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[2],"R:--"),"F02 estado caducado oculta limitacion anterior");
    rx(KART_CAN_STATUS,limited);tick+=200;rx(KART_CAN_STATUS,limited);tick+=200;
    rx(KART_CAN_STATUS,limited);tick+=101;rx(KART_CAN_STATUS,limited);Kart_ControlTick();
    Kart_FormatDisplay(&ui,lines);
    check(!ui.feedback_fresh && ui.status_fresh && strstr(lines[2],"R:--"),
          "F02 feedback caducado con estado fresco no acredita limitacion vigente");

    reset();feedback();pins[2]=1;Kart_RequestNextMode();Kart_ControlTick();
    const uint8_t charger[8]={1,6,0,255,4|16|8,0,0,0};rx(KART_CAN_STATUS,charger);
    tick+=100;Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    printf("DISPLAY charger: %s / %s\n",lines[1],lines[4]);
    check(strcmp(lines[1],"CARGADOR CONECTADO")==0 && strstr(lines[4],"MODO: PONER N"),
          "F03 cargador y rechazo de modo se ven a la vez sin ocultar inhibicion");
    check(strncmp(lines[4],"STM ",4)==0 && strstr(lines[3],"SD:PAUSA"),
          "F03 durante aviso conserva version y estado SD");
    tick+=501;Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(strcmp(lines[1],"SIN DATOS CAN RECIENTES")==0 && strstr(lines[4],"MODO: PONER N"),
          "F03 perdida CAN conserva prioridad frente al aviso pendiente");
    tick+=3001;feedback();Kart_ControlTick();Kart_FormatDisplay(&ui,lines);
    check(strstr(lines[4],"SD FILAS:") && !strstr(lines[4],"MODO:"),
          "F03 tras caducar aviso reaparece contador SD");

    bool causes_preserved=true,notices_visible=true,layout_valid=true;
    for(unsigned cause=0;cause<=21;cause++)for(unsigned note=1;note<=4;note++) {
        KartUiSnapshot s={};s.feedback_fresh=s.status_fresh=s.status_seen=1;
        s.inhibit=(uint8_t)cause;s.sd_rows=UINT32_MAX;
        char base[5][41]={};Kart_FormatDisplay(&s,base);
        s.ui_notice=(uint8_t)note;Kart_FormatDisplay(&s,lines);
        causes_preserved&=strcmp(base[1],lines[1])==0;
        const char* expected_notice[]={"","MODO: PONER N","PARAR Y SOLTAR PEDAL","MODO: CAN CADUCADO","MODO: VER ENTRADAS"};
        notices_visible&=strstr(lines[4],expected_notice[note])!=nullptr;
        layout_valid&=display_fits(lines);
    }
    check(causes_preserved,"F03 todos los motivos y el desconocido conservan prioridad");
    check(notices_visible,"F03 los cuatro avisos siguen visibles en fila de desarrollo");
    check(layout_valid,"F03 combinaciones de aviso e inhibicion caben en cinco filas de 40");
    KartUiSnapshot s={};s.feedback_fresh=1;s.ui_notice=KART_UI_REQUIRE_STOP;
    Kart_FormatDisplay(&s,lines);
    check(strcmp(lines[1],"ESP SIN ESTADO RECIENTE")==0 && strstr(lines[4],"SOLTAR PEDAL"),
          "F03 estado ESP antiguo mantiene motivo y aviso completo");
    s.ui_notice=255;Kart_FormatDisplay(&s,lines);
    check(strstr(lines[4],"SD FILAS:"),"F03 aviso desconocido no indexa fuera ni borra diagnostico");
}
