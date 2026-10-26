/* Included by kart_frontend.c: single producer (control), single consumer (SD).
 * Short critical sections copy fixed structs; all FatFs work stays in SDTask. */
#include "kart_storage.h"
#define LOG_CAPACITY 128u
#define LOG_ROTATE_BYTES (16u*1024u*1024u)
#define LOG_PERIOD_MS 20u
#define LOG_COLUMNS "Firmware,Tick_ms,AccelADC,SteerADC,AccelCAN,SteerCAN,InputFlags,Brake,Start,GearReq,ModeReq,ModeRecv,ModeApplied,Inhibit,BackendFlags,Vel_kmh,RpmFL,RpmFR,RpmRL,RpmRR,Yaw_deci_dps,CanAge_ms,StatusAge_ms,SOC_valid,SOC_pct,Temp_valid,Temp_C,BMS355Age_ms,BMS356Age_ms,BMS355_hex,BMS356_hex,CanTxErr,CanRxErr,AdcErr,Seq,Elapsed_ms,EventMask,Dropped,DriveInput,ReverseInput,StartInput,PedalFree,Stationary,FeedbackUsable,FrontLeftAge_ms,FrontRightAge_ms,FrontLeftValid,FrontRightValid,RelayState,BackendGear,LogAge_ms,LogWords_hex\r\n"
typedef struct {KartUiSnapshot s;uint64_t elapsed;uint32_t seq,event;} LogItem;
static LogItem log_queue[LOG_CAPACITY];
static uint16_t log_head,log_tail,log_count;
static KartUiSnapshot log_previous;
static uint8_t log_have_previous,log_clock_seen,sd_open,sd_state,sd_error,log_accept;
static uint32_t log_last_tick,log_last_sample,log_sequence,log_dropped,sd_rows,sd_events,sd_session,sd_part;
static uint32_t sd_last_try,sd_last_sync,sd_last_save,sd_free_kib,sd_max_call;
static uint64_t log_clock,log_origin,log_elapsed;
static FATFS sd_fs;
static FIL sd_data,sd_event_file;
static char sd_dir[32];
static uint8_t sd_data_open,sd_events_open;
static void sd_publish(uint8_t state,uint8_t error){uint32_t key=lock_data();sd_state=state;sd_error=error;unlock_data(key);}
static void logger_init(void){
    log_head=log_tail=log_count=0;log_have_previous=log_clock_seen=0;log_clock=log_origin=log_elapsed=0;
    log_last_tick=log_last_sample=log_sequence=log_dropped=sd_rows=sd_events=0;
    sd_session=sd_part=sd_last_try=sd_last_sync=sd_last_save=sd_max_call=0;
    sd_free_kib=UINT32_MAX;sd_open=sd_data_open=sd_events_open=0;sd_state=KART_SD_STOPPED;sd_error=0;
    record_request=1;log_accept=1;
}
static void logger_status(KartUiSnapshot* s){
    s->sd_state=sd_state;s->sd_error=sd_error;s->sd_rows=sd_rows;s->sd_requested=record_request;
    s->sd_session=sd_session;s->sd_dropped=log_dropped;s->sd_elapsed_ms=(uint32_t)log_elapsed;
    s->sd_free_kib=sd_free_kib;s->sd_last_save=sd_last_save;s->sd_max_call_ms=sd_max_call;s->sd_queued=log_count;
}
static uint32_t logger_events(const KartUiSnapshot* a,const KartUiSnapshot* b){
    uint32_t e=0;
    if(a->start!=b->start || a->start_input!=b->start_input)e|=1;
    if(a->gear!=b->gear || a->drive_input!=b->drive_input || a->reverse_input!=b->reverse_input)e|=2;
    if(a->mode_requested!=b->mode_requested || a->mode_applied!=b->mode_applied)e|=4;
    if(a->brake!=b->brake)e|=8;
    if(a->input_flags!=b->input_flags || a->adc_errors!=b->adc_errors)e|=16;
    if(a->feedback_fresh!=b->feedback_fresh || a->status_fresh!=b->status_fresh || a->can_rx_errors!=b->can_rx_errors || a->can_tx_errors!=b->can_tx_errors)e|=32;
    if(a->inhibit!=b->inhibit || a->backend_flags!=b->backend_flags)e|=64;
    if(a->log_words[0]!=b->log_words[0] || (a->log_age<=KART_LOG_MAX_AGE)!=(b->log_age<=KART_LOG_MAX_AGE))e|=128;
    return e;
}
static void logger_capture(const KartUiSnapshot* s){
    /* GUI resets the capture too; protect clock and previous together. */
    uint32_t key=lock_data();
    if(log_clock_seen)log_clock+=(uint32_t)(s->tick-log_last_tick);else log_clock_seen=1;
    log_last_tick=s->tick;
    const uint32_t event=log_have_previous?logger_events(s,&log_previous):256u;
    log_previous=*s;log_have_previous=1;
    if(record_request && log_accept && (event || s->tick-log_last_sample>=LOG_PERIOD_MS || !log_sequence)){
        log_elapsed=log_clock-log_origin;
        log_last_sample=s->tick;const uint32_t seq=++log_sequence;
        if(log_count==LOG_CAPACITY)log_dropped++;
        else {LogItem* item=&log_queue[log_head];item->s=*s;item->elapsed=log_clock-log_origin;item->seq=seq;item->event=event;
            log_head=(uint16_t)((log_head+1)%LOG_CAPACITY);log_count++;}
    }
    unlock_data(key);
}
static int log_pop(LogItem* item){uint32_t key=lock_data();if(!log_count){unlock_data(key);return 0;}
    *item=log_queue[log_tail];log_tail=(uint16_t)((log_tail+1)%LOG_CAPACITY);log_count--;unlock_data(key);return 1;}
static void log_reset_capture_locked(void){log_head=log_tail=log_count=0;log_origin=log_clock;log_elapsed=0;
    log_sequence=log_dropped=sd_rows=sd_events=sd_last_save=0;log_accept=1;log_have_previous=0;}
static void log_new_capture(void){uint32_t key=lock_data();log_reset_capture_locked();unlock_data(key);}
void Kart_ToggleRecording(void){
    uint32_t key=lock_data();
    if(sd_state==KART_SD_CLOSING){unlock_data(key);return;}
    if(sd_state==KART_SD_ERROR && record_request){sd_last_try=HAL_GetTick()-1000u;unlock_data(key);return;}
    record_request=!record_request;
    if(!record_request && sd_open)sd_state=KART_SD_CLOSING;
    if(record_request)log_reset_capture_locked();unlock_data(key);
}
static FRESULT log_write(FIL* f,const char* text){UINT done=0;const UINT n=(UINT)strlen(text);FRESULT r=f_write(f,text,n,&done);return r==FR_OK && done!=n?FR_DISK_ERR:r;}
static void log_abort(uint8_t error){
    uint32_t key=lock_data();log_accept=0;log_dropped+=log_count;log_count=log_head=log_tail=0;unlock_data(key);
    if(sd_data_open)f_close(&sd_data);if(sd_events_open)f_close(&sd_event_file);
    sd_data_open=sd_events_open=sd_open=0;f_mount(NULL,"",0);sd_last_try=HAL_GetTick();sd_publish(KART_SD_ERROR,error);
}
static FRESULT log_part_open(void){
    char path[64];snprintf(path,sizeof(path),"%s/D%07lu.CSV",sd_dir,(unsigned long)sd_part);
    FRESULT r=f_open(&sd_data,path,FA_CREATE_NEW|FA_WRITE);if(r!=FR_OK)return r;sd_data_open=1;
    char header[180];snprintf(header,sizeof(header),"# firmware=%s,schema=3,session=%lu,part=%lu,BMS_format_confirmed=0\r\n",KART_FRONTEND_VERSION,(unsigned long)sd_session,(unsigned long)sd_part);
    r=log_write(&sd_data,header);if(r==FR_OK)r=log_write(&sd_data,LOG_COLUMNS);if(r!=FR_OK)return r;
    snprintf(path,sizeof(path),"%s/E%07lu.CSV",sd_dir,(unsigned long)sd_part);
    r=f_open(&sd_event_file,path,FA_CREATE_NEW|FA_WRITE);if(r!=FR_OK)return r;sd_events_open=1;
    return log_write(&sd_event_file,"Seq,Elapsed_ms,EventMask,Dropped\r\n");
}
static FRESULT log_sync_parts(void){FRESULT a=f_sync(&sd_data),b=f_sync(&sd_event_file);return a!=FR_OK?a:b;}
static FRESULT log_close_parts(void){
    FRESULT r=log_sync_parts(),a=f_close(&sd_data),b=f_close(&sd_event_file);sd_data_open=sd_events_open=0;
    return r!=FR_OK?r:a!=FR_OK?a:b;
}
static FRESULT log_small_file(const char* name,const char* text){
    FIL f;char path[64];snprintf(path,sizeof(path),"%s/%s",sd_dir,name);
    FRESULT r=f_open(&f,path,FA_CREATE_NEW|FA_WRITE);if(r!=FR_OK)return r;
    r=log_write(&f,text);FRESULT sync=f_sync(&f),close=f_close(&f);return r!=FR_OK?r:sync!=FR_OK?sync:close;
}
static void log_free_space(void){DWORD clusters=0;FATFS* fs=NULL;
    if(f_getfree("",&clusters,&fs)==FR_OK && fs)sd_free_kib=(uint32_t)(((uint64_t)clusters*fs->csize)/2u);else sd_free_kib=UINT32_MAX;
}
static int log_begin(void){
    sd_publish(KART_SD_OPENING,sd_error);
    if(!Kart_StoragePresent()){log_abort(KART_SD_ABSENT);return 0;}
    FRESULT r=f_mount(&sd_fs,"",1);if(r!=FR_OK){log_abort((uint8_t)r);return 0;}
    if(!record_request){f_mount(NULL,"",0);sd_publish(KART_SD_STOPPED,0);return 0;}
    r=f_mkdir("LOGS");if(r!=FR_OK && r!=FR_EXIST){log_abort((uint8_t)r);return 0;}
    /* Bounded work per attempt, never erase/reuse an existing directory. */
    for(unsigned n=0;n<64;n++){
        if(sd_session>=9999999u){log_abort(KART_SD_ID_EXHAUSTED);return 0;}
        snprintf(sd_dir,sizeof(sd_dir),"LOGS/S%07lu",(unsigned long)++sd_session);
        r=f_mkdir(sd_dir);if(r==FR_OK)break;if(r!=FR_EXIST){log_abort((uint8_t)r);return 0;}
    }
    if(r==FR_EXIST){f_mount(NULL,"",0);return 0;}
    if(!log_accept)log_new_capture();
    sd_part=0;sd_rows=sd_events=0;
    char meta[640];snprintf(meta,sizeof(meta),"schema=3\nfirmware=%s\nbuild=%08lX\nuid=%08lX%08lX%08lX\nsession=%lu\nsample_period_ms=20\nqueue_capacity=128\ntime_origin=relative_no_rtc\naccel_zero_adc=%u\naccel_join_adc=%u\nBMS_format_confirmed=0\n",KART_FRONTEND_VERSION,(unsigned long)kart_build_id(KART_FRONTEND_VERSION),(unsigned long)Kart_StorageUid(0),(unsigned long)Kart_StorageUid(1),(unsigned long)Kart_StorageUid(2),(unsigned long)sd_session,KART_ACCEL_ZERO_ADC,KART_ACCEL_LEGACY_JOIN_ADC);
    r=log_small_file("META.TXT",meta);if(r==FR_OK)r=log_part_open();
    if(r!=FR_OK){log_abort((uint8_t)r);return 0;}
    sd_open=1;sd_last_sync=HAL_GetTick();log_free_space();sd_publish(record_request?KART_SD_RECORDING:KART_SD_CLOSING,0);return 1;
}
/* newlib-nano printf does not promise long-long formatting. */
static void log_decimal64(char out[21],uint64_t value){char reverse[20];unsigned n=0,j=0;
    do{reverse[n++]=(char)('0'+value%10u);value/=10u;}while(value);
    while(n)out[j++]=reverse[--n];out[j]=0;
}
static FRESULT log_row(const LogItem* item){
    char elapsed[21];log_decimal64(elapsed,item->elapsed);
    const KartUiSnapshot* s=&item->s;char b355[17],b356[17],words[129],line[1100];
    for(unsigned i=0;i<8;i++){snprintf(b355+2*i,3,"%02X",s->bms355[i]);snprintf(b356+2*i,3,"%02X",s->bms356[i]);}
    for(unsigned i=0;i<16;i++)snprintf(words+8*i,9,"%08lX",(unsigned long)s->log_words[i]);
    int n=snprintf(line,sizeof(line),"%s,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%d,%d,%d,%d,%lu,%lu,%u,%u,%u,%d,%lu,%lu,%s,%s,%lu,%lu,%lu,%lu,%s,%lu,%lu,%u,%u,%u,%u,%u,%u,%lu,%lu,%u,%u,%u,%u,%lu,%s\r\n",
        KART_FRONTEND_VERSION,(unsigned long)s->tick,s->accel_adc,s->steer_adc,s->accel_raw,s->steer_raw,s->input_flags,s->brake,s->start,s->gear,s->mode_requested,s->mode_received,s->mode_applied,s->inhibit,s->backend_flags,s->speed_kph,s->rpm_front_left,s->rpm_front_right,s->rpm_left,s->rpm_right,s->yaw_deci_dps,(unsigned long)s->can_age,(unsigned long)s->status_age,s->soc_valid,s->soc_pct,s->temp_valid,s->battery_temp_c,(unsigned long)s->bms_soc_age,(unsigned long)s->bms_temp_age,b355,b356,(unsigned long)s->can_tx_errors,(unsigned long)s->can_rx_errors,(unsigned long)s->adc_errors,(unsigned long)item->seq,elapsed,(unsigned long)item->event,(unsigned long)log_dropped,s->drive_input,s->reverse_input,s->start_input,s->pedal_free,s->stationary,s->feedback_usable,(unsigned long)s->front_left_age,(unsigned long)s->front_right_age,s->front_left_valid,s->front_right_valid,s->relay_state,s->backend_gear,(unsigned long)s->log_age,words);
    if(n<0 || n>=(int)sizeof(line))return FR_INT_ERR;
    FRESULT r=log_write(&sd_data,line);if(r!=FR_OK)return r;sd_rows++;
    if(item->event){snprintf(line,sizeof(line),"%lu,%s,%lu,%lu\r\n",(unsigned long)item->seq,elapsed,(unsigned long)item->event,(unsigned long)log_dropped);r=log_write(&sd_event_file,line);if(r==FR_OK)sd_events++;}
    return r;
}
static void logger_sd_step(void){
    if(!sd_open){
        if(!record_request)return;
        if(sd_state==KART_SD_ERROR && HAL_GetTick()-sd_last_try<1000u)return;
        if(!log_begin())return;
    }
    if(!Kart_StoragePresent()){log_abort(KART_SD_ABSENT);return;}
    LogItem item;
    for(unsigned i=0;i<8 && log_pop(&item);i++){
        FRESULT r=log_row(&item);if(r!=FR_OK){log_abort((uint8_t)r);return;}
    }
    if(!record_request && !log_count){
        sd_publish(KART_SD_CLOSING,0);FRESULT r=log_close_parts();sd_open=0;
        if(r==FR_OK){char end[180];snprintf(end,sizeof(end),"closed=1\nrows=%lu\nevents=%lu\ndropped=%lu\nparts=%lu\n",(unsigned long)sd_rows,(unsigned long)sd_events,(unsigned long)log_dropped,(unsigned long)(sd_part+1));r=log_small_file("END.TXT",end);}
        if(r!=FR_OK){log_abort((uint8_t)r);return;}f_mount(NULL,"",0);sd_last_save=HAL_GetTick();sd_publish(KART_SD_SAVED,0);return;
    }
    if(f_size(&sd_data)>=LOG_ROTATE_BYTES || f_size(&sd_event_file)>=LOG_ROTATE_BYTES){
        FRESULT r=log_close_parts();if(r==FR_OK && sd_part<9999999u){sd_part++;r=log_part_open();}else if(r==FR_OK)r=FR_INT_ERR;
        if(r!=FR_OK){log_abort((uint8_t)r);return;}
    }
    if(HAL_GetTick()-sd_last_sync>=1000u){FRESULT r=log_sync_parts();if(r!=FR_OK){log_abort((uint8_t)r);return;}
        sd_last_save=sd_last_sync=HAL_GetTick();log_free_space();}
}
void Kart_SdTick(void){const uint32_t began=HAL_GetTick();logger_sd_step();const uint32_t cost=HAL_GetTick()-began;if(cost>sd_max_call)sd_max_call=cost;}
