// ENS-0024: tramas copiadas de captura real; ejecucion y reloj de estas pruebas son simulados.
static void test_bms_dashboard(FILE* output){
    KartDashboardState st;KartDashboard_Init(&st);st.page=KART_PAGE_BMS;
    auto s=sample(1);s.bms_soc_age=s.bms_temp_age=100;
    const uint8_t soc_frame[8]={0x57,0,0x64,0,0x2e,0x22,0,0};
    const uint8_t temp_frame[8]={0x8c,0x14,0xfa,0xff,0xdc,0,0,0};
    memcpy(s.bms355,soc_frame,8);memcpy(s.bms356,temp_frame,8);
    render(st,s);
    check(contains("SOC* 87 %")&&contains("22 °C*")&&contains("Lectura provisional"),"BMS recibido sin confirmar muestra valores con marca provisional");
    for(int theme=0;theme<2;theme++)for(int page: {KART_PAGE_HOME,KART_PAGE_BMS}){
        st.light=(uint8_t)theme;st.page=(uint8_t)page;render(st,s);
        check(within_screen()&&contains(page==KART_PAGE_HOME?"Bat* 87%  22°C":"SOC* 87 %"),"BMS provisional visible y completo en resumen/BMS y ambos temas");
        if(output)save_json(output,6,theme,page,0);
    }
    st.page=KART_PAGE_BMS;s.bms_soc_age=s.bms_temp_age=2000;render(st,s);
    check(contains("SOC* 87 %")&&contains("22 °C*"),"BMS admite limite exacto de 2000 ms");
    s.bms_soc_age=2001;render(st,s);
    check(contains("SOC -- %")&&!contains("87 %")&&contains("22 °C*"),"BMS SOC caduca sin ocultar temperatura reciente");
    s.bms_temp_age=2001;render(st,s);
    check(contains("SOC -- %")&&contains("-- °C")&&!contains("22 °C*")&&!contains("Lectura provisional"),"BMS caducado no conserva porcentajes ni temperaturas");
    s.bms_soc_age=s.bms_temp_age=100;s.bms355[4]=0x11;s.bms355[5]=0x27;render(st,s);
    check(contains("SOC -- %"),"BMS rechaza SOC 10001 antes de truncar a porcentaje entero");
    s.bms355[4]=s.bms355[5]=0;render(st,s);
    check(contains("SOC* 0 %"),"BMS SOC cero recibido no se confunde con ausencia");
    s.bms355[4]=0x10;s.bms355[5]=0x27;s.bms356[4]=0xdc;s.bms356[5]=5;render(st,s);
    check(contains("SOC* 100 %")&&contains("150 °C*")&&within_screen(),"BMS limites superiores plausibles caben sin recorte");
    s.bms356[4]=0xdd;render(st,s);check(contains("-- °C"),"BMS rechaza 1501 decimas antes de truncar");
    s.bms356[4]=0x0b;s.bms356[5]=0xfe;render(st,s);check(contains("-- °C"),"BMS rechaza menos 501 decimas");
    s.bms356[4]=0x0c;render(st,s);check(contains("-50 °C*"),"BMS acepta limite inferior con signo");
    s.soc_valid=s.temp_valid=1;s.soc_pct=87;s.battery_temp_c=22;render(st,s);
    check(contains("SOC 87 %")&&contains("22 °C")&&!contains("SOC*")&&contains("Datos recibidos y validados"),"BMS confirmado no se etiqueta provisional");
    s.bms_soc_age=s.bms_temp_age=2001;render(st,s);
    check(contains("SOC -- %")&&contains("-- °C")&&!contains("validados"),"BMS flags confirmados no reviven una muestra caducada");
    s.soc_valid=s.temp_valid=0;s.bms_soc_age=s.bms_temp_age=UINT32_MAX;render(st,s);
    check(contains("SOC -- %")&&contains("-- °C"),"BMS nunca recibido oculta bytes residuales");
    st.page=KART_PAGE_HOME;s.bms_soc_age=s.bms_temp_age=100;render(st,s);
    check(contains("Bat* 100%  -50°C")&&within_screen(),"Resumen admite extremos y aviso provisional sin recorte");
}
