// ENS-0021: tiempo y respuestas FatFs sinteticos; funciones de produccion reales.
static void sd_pause_during_mount(FakeFsOp op) {if(op==FS_MOUNT)Kart_ToggleRecording();}
static void sd_pause_during_open(FakeFsOp op) {if(op==FS_OPEN)Kart_ToggleRecording();}
static void sd_pause_during_header(FakeFsOp op) {if(op==FS_WRITE)Kart_ToggleRecording();}
static bool sd_retry_error_visible=false;
static void sd_inspect_retry(FakeFsOp op) {
    if(op!=FS_MOUNT)return;
    Kart_ControlTick();char lines[5][41]={};Kart_FormatDisplay(&ui,lines);
    sd_retry_error_visible=ui.sd_state==KART_SD_OPENING && ui.sd_error==FR_DISK_ERR &&
          strstr(lines[3],"SD:ABRE E:1");
}
static void test_sd_retries() {
    reset();mount_result=FR_DISK_ERR;fs_delays[FS_MOUNT]=30000;
    fs_delays[FS_UNMOUNT]=500;Kart_ToggleRecording();Kart_SdTick();
    check(sd_state==KART_SD_ERROR && tick==31500,"SD reproduce montaje de 30 s y desmontaje de 500 ms");
    Kart_SdTick();check(mounts==1,"SD no reintenta inmediatamente despues de fallo lento");
    tick+=999;Kart_SdTick();check(mounts==1,"SD espera 1000 ms completos desde fin del fallo");
    tick++;sd_retry_error_visible=false;fs_hook=sd_inspect_retry;Kart_SdTick();fs_hook=nullptr;
    check(sd_retry_error_visible,"SD conserva ultimo error visible durante montaje de reintento");
    check(mounts==2 && sd_state==KART_SD_ERROR && sd_error==FR_DISK_ERR,
          "SD reintenta al cumplir espera y conserva error si vuelve a fallar");
    tick+=1000;mount_result=FR_OK;Kart_SdTick();
    check(sd_state==KART_SD_RECORDING && sd_error==0 && sd_rows==1,
          "SD recuperacion borra error solo al grabar y escribe primera fila");
    Kart_SdTick();check(sd_rows==1 && syncs==0,"SD montaje lento no adelanta fila ni sync siguientes");
    tick+=200;Kart_SdTick();check(sd_rows==2,"SD mantiene periodo de filas tras montaje lento");

    reset();open_result=FR_DISK_ERR;fs_delays[FS_OPEN]=30000;Kart_ToggleRecording();Kart_SdTick();
    Kart_SdTick();check(opens==1 && mounts==1,"SD espera tambien tras apertura lenta fallida");
    reset();short_write=1;fs_delays[FS_WRITE]=30000;fs_delays[FS_CLOSE]=700;fs_delays[FS_UNMOUNT]=500;
    Kart_ToggleRecording();Kart_SdTick();tick+=999;Kart_SdTick();
    check(mounts==1 && sd_rows==0 && !sd_open,"SD escritura parcial espera desde cierre y desmontaje completos");
    tick++;short_write=0;Kart_SdTick();check(mounts==2,"SD escritura fallida permite reintentar tras espera");
    reset();Kart_ToggleRecording();Kart_SdTick();sync_result=FR_DISK_ERR;
    fs_delays[FS_SYNC]=30000;fs_delays[FS_CLOSE]=700;tick+=1000;Kart_SdTick();
    tick+=999;Kart_SdTick();check(mounts==1 && !sd_open && sd_state==KART_SD_ERROR,
          "SD sync fallido no permite reabrir antes de espera tras cierre");

    reset();fs_delays[FS_MOUNT]=30000;fs_hook=sd_pause_during_mount;
    Kart_ToggleRecording();Kart_SdTick();fs_hook=nullptr;
    check(!record_request && !sd_open && opens==0 && file_content.empty() && sd_state==KART_SD_STOPPED,
          "SD pausa durante montaje evita abrir y escribir al volver");
    reset();fs_delays[FS_OPEN]=30000;fs_hook=sd_pause_during_open;
    Kart_ToggleRecording();Kart_SdTick();fs_hook=nullptr;
    check(!sd_open && closes==1 && file_content.empty() && sd_state==KART_SD_STOPPED,
          "SD pausa durante apertura cierra sin escribir cabecera ni filas");
    reset();fs_hook=sd_pause_during_header;Kart_ToggleRecording();Kart_SdTick();fs_hook=nullptr;
    check(!sd_open && closes==1 && sd_rows==0 && sd_state==KART_SD_STOPPED,
          "SD pausa durante cabecera cierra sin registrar fila posterior");
    reset();mount_result=FR_DISK_ERR;Kart_ToggleRecording();Kart_SdTick();
    tick+=1000;mount_result=FR_OK;fs_hook=sd_pause_during_mount;Kart_SdTick();fs_hook=nullptr;
    check(!sd_open && opens==0 && sd_state==KART_SD_ERROR && sd_error==FR_DISK_ERR,
          "SD pausa durante recuperacion conserva ultimo error sin afirmar grabacion");
    const int attempted=mounts;tick+=60000;Kart_SdTick();
    check(mounts==attempted,"SD pausada no reintenta aunque pase el tiempo");

    reset();tick=0xfffffff0u;mount_result=FR_DISK_ERR;fs_delays[FS_MOUNT]=30000;
    Kart_ToggleRecording();Kart_SdTick();tick+=999;Kart_SdTick();
    check(mounts==1,"SD espera correcta si tick desborda durante montaje");
    tick++;Kart_SdTick();check(mounts==2,"SD reintenta tras wrap del tick");
}
