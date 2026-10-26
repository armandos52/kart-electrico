// FatFs simulado + funciones de produccion; no prueba fisica de tarjeta.
static void drain(){for(unsigned i=0;i<100 && (log_count || sd_state==KART_SD_CLOSING);i++)Kart_SdTick();}
static void test_sd_retries(){
    char decimal[21];log_decimal64(decimal,UINT64_MAX);check(std::string(decimal)=="18446744073709551615","Tiempo 64 bits independiente de printf nano");
    reset();check(record_request==1,"SD autograbacion al encender sin START");
    storage_present=false;Kart_ControlTick();Kart_SdTick();check(sd_error==KART_SD_ABSENT && mounts==0,"Ausencia detectada evita montaje bloqueante");
    int before=mounts;Kart_SdTick();check(mounts==before,"Reintento no gira en bucle");
    storage_present=true;tick+=1000;Kart_SdTick();tick+=20;Kart_ControlTick();Kart_SdTick();
    check(sd_open && sd_rows==1 && sd_session==1,"Reinsercion abre sesion nueva con primera muestra");
    Kart_ToggleRecording();drain();check(sd_state==KART_SD_SAVED && fs_files.count("LOGS/S0000001/END.TXT"),"Detener drena sincroniza cierra y deja marcador");
    const uint64_t saved_elapsed=log_elapsed;tick+=1000;Kart_ControlTick();check(log_elapsed==saved_elapsed,"Tiempo de sesion detenido permanece congelado");
    const auto closed=fs_files;Kart_ToggleRecording();tick+=20;Kart_ControlTick();Kart_SdTick();
    check(sd_session==2 && fs_files["LOGS/S0000001/D0000000.CSV"]==closed.at("LOGS/S0000001/D0000000.CSV"),"Reanudar crea sesion diferente sin alterar anterior");
    Kart_ToggleRecording();drain();logger_init();Kart_ControlTick();Kart_SdTick();check(sd_session==3,"Reinicio con mismo tick no colisiona");
    reset();for(unsigned i=0;i<180;i++){tick+=20;Kart_ControlTick();}check(log_count==128 && log_dropped==52,"Cola acotada: perdidas contadas sin espera");
    Kart_SdTick();check(sd_rows==8 && log_count==120,"Escritura en lotes acotados");Kart_ToggleRecording();drain();
    check(sd_state==KART_SD_SAVED && sd_rows==128 && fs_files["LOGS/S0000001/END.TXT"].find("dropped=52")!=std::string::npos,"Perdidas conservadas en cierre");
    reset();Kart_ControlTick();for(unsigned i=0;i<3;i++){tick+=5;Kart_ControlTick();}check(log_count==1,"Muestreo nominal 20 ms independiente del refresco");
    pins[0]=1;tick+=1;Kart_ControlTick();check(log_count==2 && (log_queue[1].event&8),"Evento freno capturado entre muestras periodicas");
    reset();tick=0xfffffff0u;Kart_ControlTick();tick+=40;Kart_ControlTick();check(log_queue[1].elapsed==40,"Tiempo relativo continuo en wrap");
    reset();mount_result=FR_DISK_ERR;fs_delays[FS_MOUNT]=30000;Kart_SdTick();before=mounts;tick+=999;Kart_SdTick();check(mounts==before,"Reintento desde fin del fallo lento");
    tick++;mount_result=0;Kart_SdTick();check(sd_open,"Recuperacion tras fallo lento");
    reset();short_write=1;Kart_ControlTick();Kart_SdTick();check(sd_state==KART_SD_ERROR&&!sd_open&&!fs_files.count("LOGS/S0000001/END.TXT"),"Escritura parcial no crea cierre falso");
    short_write=0;tick+=1000;Kart_SdTick();check(sd_session==2,"Reintento no concatena archivo parcial");
    reset();Kart_ControlTick();Kart_SdTick();sync_result=FR_DISK_ERR;Kart_ToggleRecording();drain();check(sd_state==KART_SD_ERROR&&!fs_files.count("LOGS/S0000001/END.TXT"),"Fallo sync no afirma Guardado");
    reset();Kart_ControlTick();Kart_SdTick();close_result=FR_DISK_ERR;Kart_ToggleRecording();drain();check(sd_state==KART_SD_ERROR,"Fallo cierre visible");
    reset();Kart_ControlTick();Kart_SdTick();storage_present=false;Kart_SdTick();check(sd_state==KART_SD_ERROR&&!sd_open,"Retirada aborta con error");
    reset();fs_files["kart_v13.csv"]="CSV parcial original";Kart_ControlTick();Kart_SdTick();check(fs_files["kart_v13.csv"]=="CSV parcial original","CSV anterior preservado sin repararlo");
}
static void test_log_can(){
    reset();uint32_t words[16];for(unsigned i=0;i<16;i++)words[i]=100+i;uint8_t b[8];
    for(unsigned i=0;i<15;i++){kart_log_packet(b,words,1,(uint8_t)i);rx(KART_CAN_LOG,b);}Kart_ControlTick();check(ui.log_age==UINT32_MAX,"Bloque parcial no se publica");
    kart_log_packet(b,words,1,15);rx(KART_CAN_LOG,b);Kart_ControlTick();check(ui.log_age==0&&ui.log_words[15]==115,"Bloque completo coherente publicado");
    tick+=1001;Kart_ControlTick();check(ui.log_age==1001,"Diagnostico conserva edad y caduca");
    kart_log_packet(b,words,2,0);rx(KART_CAN_LOG,b);kart_log_packet(b,words,3,1);rx(KART_CAN_LOG,b);check(log_receiver.mask==1,"Secuencias diferentes no mezcladas");
    words[0]=999;for(unsigned i=0;i<16;i++){kart_log_packet(b,words,0,(uint8_t)i);rx(KART_CAN_LOG,b);}Kart_ControlTick();check(ui.log_words[0]==999,"Nuevo arranque acepta secuencia reiniciada");
}
