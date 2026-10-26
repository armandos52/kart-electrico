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

    // B04: logger_cases cubre sesiones nuevas y preservacion de CSV parcial.
}
