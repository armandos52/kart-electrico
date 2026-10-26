// ENS-0023: fallos reproducibles con código real y eventos periféricos simulados.
static void hall_during_transmit(uint32_t id){
    if(id!=KART_CAN_COMMAND)return;
    ++tick;fakeDwt.CYCCNT+=216000;Kart_HallEdge(HALL_DI_ESQ_Pin);
}
static void sd_pause_during_read(FakeFsOp op){if(op==FS_READ && record_request)Kart_ToggleRecording();}
static void test_bugfixes(){
    reset();const uint8_t bytes[8]={};
    rx(0x321,bytes,3);
    check(received.rx_errors==0,"B02 tramas cortas de otros nodos no cuentan como error frontal");
    rx_header={KART_CAN_FEEDBACK,1,0,8};rx_pending=1;HAL_CAN_RxFifo0MsgPendingCallback(&hcan1);
    check(received.rx_errors==0,"B02 identificador extendido ajeno se ignora");
    const uint32_t errors=received.rx_errors;
    rx(KART_CAN_FEEDBACK,bytes,7);
    check(received.rx_errors==errors+1 && !received.feedback_seen,"B02 DLC incorrecto de feedback propio sigue rechazado");
    rx_header={KART_CAN_STATUS,0,1,8};rx_pending=1;HAL_CAN_RxFifo0MsgPendingCallback(&hcan1);
    check(received.rx_errors==errors+2 && !received.status_seen,"B02 RTR propio sigue rechazado");
    feedback();Kart_ControlTick();check(ui.feedback_usable,"B02 trafico valido sigue utilizable tras trama ajena");

    reset();fakeDwt.CYCCNT=100000;Kart_HallEdge(HALL_DI_ESQ_Pin);
    tick+=99;fakeDwt.CYCCNT+=21384000;feedback();
    tx_hook=hall_during_transmit;Kart_ControlTick();tx_hook=nullptr;
    printf("TRACE B03 ui_tick=%lu now=%lu hall_age=%lu valid=%u rpm=%d\n",(unsigned long)ui.tick,(unsigned long)tick,(unsigned long)ui.front_left_age,ui.front_left_valid,ui.rpm_front_left);
    check(ui.front_left_valid && ui.front_left_age==0 && ui.rpm_front_left==150,
          "B03 pulso Hall entre timestamp y publicacion conserva RPM valida");
    tick+=501;Kart_ControlTick();check(!ui.front_left_valid && ui.front_left_age==501,"B03 Hall antiguo sigue caducando");

    reset();Kart_ToggleRecording();Kart_SdTick();
    const size_t first_good=file_content.size();
    short_write=1;tick+=200;Kart_SdTick();
    const std::string damaged=file_content;
    check(damaged.size()>first_good && damaged.back()!='\n',"B04 reproduce fila parcialmente escrita");
    short_write=0;tick+=1000;Kart_SdTick();
    check(file_content==damaged && sd_state==KART_SD_ERROR && !sd_open && sd_rows==1,
          "B04 no concatena una fila nueva a un CSV incompleto");
    reset();short_write=1;Kart_ToggleRecording();Kart_SdTick();
    const std::string header_fragment=file_content;short_write=0;tick+=1000;Kart_SdTick();
    check(file_content==header_fragment && sd_state==KART_SD_ERROR && sd_rows==0,
          "B04 no trata cabecera parcial como archivo preparado");
    check(sd_error==KART_SD_CSV_INVALID,"B04 archivo incompleto tiene codigo distinto de error fisico");
    file_content.clear();tick+=1000;Kart_SdTick();
    check(sd_state==KART_SD_RECORDING && sd_rows==1 && sd_error==0,"B04 archivo nuevo permite recuperar grabacion");
    const std::string complete=file_content;
    Kart_ToggleRecording();Kart_SdTick();Kart_ToggleRecording();tick+=200;Kart_SdTick();
    check(file_content.compare(0,complete.size(),complete)==0 && sd_rows==2 && sd_state==KART_SD_RECORDING,
          "B04 CSV completo conserva bytes anteriores y permite append");
    Kart_ToggleRecording();Kart_SdTick();const std::string preserved=file_content;
    read_result=FR_DISK_ERR;Kart_ToggleRecording();Kart_SdTick();
    check(sd_state==KART_SD_ERROR && sd_error==FR_DISK_ERR && file_content==preserved,
          "B04 fallo de lectura no se interpreta como permiso para escribir");
    read_result=0;seek_result=FR_DISK_ERR;tick+=1000;Kart_SdTick();
    check(sd_state==KART_SD_ERROR && file_content==preserved,"B04 fallo al posicionar archivo no escribe");
    seek_result=0;tick+=1000;fs_hook=sd_pause_during_read;Kart_SdTick();fs_hook=nullptr;
    check(!record_request && !sd_open && file_content==preserved,"B04 pausa durante validacion no escribe fila nueva");
    reset();file_content="# firmware=stm32_frontal-v1.3-dev4,schema=2,BMS_format_confirmed=0\r\n";
    const std::string metadata_only=file_content;Kart_ToggleRecording();Kart_SdTick();
    check(file_content==metadata_only && sd_error==KART_SD_CSV_INVALID && sd_rows==0,
          "B04 metadata completa sin columnas no basta para continuar");
    reset();file_content=complete;
    const size_t version_at=file_content.find("v1.3-dev7-ui");
    if(version_at!=std::string::npos)file_content.replace(version_at,12,"v1.3-dev4");
    Kart_ToggleRecording();Kart_SdTick();
    check(sd_state==KART_SD_RECORDING && sd_rows==1,"B04 CSV esquema 2 de dev4 sigue siendo compatible");
    reset();file_content=complete;const size_t schema_at=file_content.find("schema=2");file_content[schema_at+7]='1';
    const std::string incompatible=file_content;Kart_ToggleRecording();Kart_SdTick();
    check(sd_error==KART_SD_CSV_INVALID && file_content==incompatible,"B04 esquema distinto se conserva sin mezclar filas");
    reset();fs_hook=sd_pause_during_header;Kart_ToggleRecording();Kart_SdTick();fs_hook=nullptr;
    Kart_ToggleRecording();Kart_SdTick();
    check(sd_state==KART_SD_RECORDING && sd_rows==1,"B04 pausa durante cabecera completa permite reanudar despues");
}
