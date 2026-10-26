#include "kart_frontend.h"
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc3;
extern CAN_HandleTypeDef hcan1;
// Referencias históricas, tolerancia de desarrollo 100 cuentas: validar/calibrar en banco.
#define ACCEL_MIN 124u
#define ACCEL_MAX 1861u
#define STEER_MIN 1042u
#define STEER_MAX 3313u
#define ADC_MARGIN 100u
#define FEEDBACK_MAX_AGE 500u
#define STATUS_MAX_AGE 300u
#define BMS_MAX_AGE 2000u
// Hasta verificar protocolo del BMS instalado, conservar bytes y ocultar valores no acreditados.
#define BMS_FORMAT_CONFIRMED 0
// Pulsador momentaneo activo alto, confirmado por Armando (13-10-2026).
// Valores de desarrollo: contrastar rebotes y latencias reales en banco.
#define START_PRESS_MS 40u
#define START_RELEASE_MS 40u

static struct {
    uint32_t changed_at;
    uint8_t seen,raw,armed,requested;
} start_button;

static uint8_t read_start_button(uint8_t pressed,uint32_t now) {
    if(!start_button.seen || pressed!=start_button.raw) {
        start_button.seen=1;start_button.raw=pressed;start_button.changed_at=now;
    }
    if(!pressed) {
        // Soltar solo rearma la siguiente pulsacion; no es una orden STOP.
        // Tambien exige liberacion al encender/reinicializar con boton sostenido.
        if(now-start_button.changed_at>=START_RELEASE_MS)start_button.armed=1;
    } else if(start_button.armed &&
              (start_button.requested || now-start_button.changed_at>=START_PRESS_MS)) {
        // START exige estabilidad; STOP se acepta en la primera muestra pulsada.
        // Consumir la pulsacion impide que mantener/rebotar repita el cambio.
        start_button.requested=!start_button.requested;start_button.armed=0;
    }
    return start_button.requested;
}

static KartUiSnapshot ui;
typedef struct {
    uint32_t feedback_tick,status_tick,soc_tick,temp_tick,rx_errors;
    uint8_t feedback_seen,status_seen,soc_seen,temp_seen,status[8],soc[8],temp[8];
    int16_t speed,left,right,yaw;
} CanData;
static volatile CanData received;
static volatile uint8_t mode_click=0,record_request=0;
static volatile uint32_t hall_l_time=0,hall_r_time=0,hall_l_period=0,hall_r_period=0;
static volatile uint32_t hall_l_tick=0,hall_r_tick=0;
static volatile uint8_t hall_l_seen=0,hall_r_seen=0;
static uint16_t sample_sequence=0;
static uint8_t selected_mode=0,selected_gear=0;
static uint32_t adc_errors=0,tx_errors=0,notice_tick=0;
static uint8_t notice=KART_UI_OK;
static IWDG_HandleTypeDef watchdog;
static uint8_t initialized=0,watchdog_ready=0;
static FATFS sd_fs;
static FIL sd_file;
static uint8_t sd_open=0,sd_state=KART_SD_STOPPED,sd_error=0;
static uint32_t sd_last_try=0,sd_last_log=0,sd_last_sync=0,sd_rows=0;

static uint32_t lock_data(void) {uint32_t key=__get_PRIMASK();__disable_irq();return key;}
static void unlock_data(uint32_t key) {__set_PRIMASK(key);}
static uint32_t age(uint8_t seen,uint32_t now,uint32_t then) {return seen?now-then:UINT32_MAX;}
void Kart_CopySnapshot(KartUiSnapshot* destination) {
    if(!destination)return;
    uint32_t key=lock_data();*destination=ui;unlock_data(key);
}
void Kart_RequestNextMode(void) {mode_click=1;}
void Kart_ToggleRecording(void) {uint32_t key=lock_data();record_request=!record_request;unlock_data(key);}
void Kart_HallEdge(uint16_t pin) {
    // DWT a frecuencia SystemCoreClock, sin sondeo cada 5 ms; 4 pulsos/vuelta históricos.
    const uint32_t now=DWT->CYCCNT;
    const uint32_t tick=HAL_GetTick();
    if(pin==HALL_DI_ESQ_Pin) {
        if(!hall_l_seen || now-hall_l_time>SystemCoreClock/10000u) {
            hall_l_period=hall_l_seen && tick-hall_l_tick<=500?now-hall_l_time:0;
            hall_l_time=now;hall_l_tick=tick;hall_l_seen=1;
        }
    } else if(pin==HALL_DI_DIR_Pin) {
        if(!hall_r_seen || now-hall_r_time>SystemCoreClock/10000u) {
            hall_r_period=hall_r_seen && tick-hall_r_tick<=500?now-hall_r_time:0;
            hall_r_time=now;hall_r_tick=tick;hall_r_seen=1;
        }
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t pin) {Kart_HallEdge(pin);}
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan) {
    if(hcan->Instance!=CAN1)return;
    for(unsigned i=0;i<3 && HAL_CAN_GetRxFifoFillLevel(hcan,CAN_RX_FIFO0)>0;i++) {
        CAN_RxHeaderTypeDef header={0};uint8_t bytes[8]={0};
        if(HAL_CAN_GetRxMessage(hcan,CAN_RX_FIFO0,&header,bytes)!=HAL_OK) {received.rx_errors++;break;}
        if(header.IDE!=CAN_ID_STD || header.RTR!=CAN_RTR_DATA || header.DLC!=8) {received.rx_errors++;continue;}
        const uint32_t now=HAL_GetTick();
        if(header.StdId==KART_CAN_FEEDBACK) {
            const int16_t speed=(int16_t)kart_u16be(bytes),left=(int16_t)kart_u16be(bytes+2),right=(int16_t)kart_u16be(bytes+4);
            if(speed<0 || speed>200 || left<0 || right<0 || left>2000 || right>2000) {received.rx_errors++;continue;}
            received.speed=speed;received.left=left;received.right=right;received.yaw=(int16_t)kart_u16be(bytes+6);
            received.feedback_tick=now;received.feedback_seen=1;
        } else if(header.StdId==KART_CAN_STATUS) {
            if(bytes[0]!=KART_PROTOCOL_VERSION || bytes[1]>20 || bytes[2]>2 ||
                (bytes[3]>2 && bytes[3]!=255) || bytes[5]>3 || bytes[6]>2 || bytes[7]>2 || (bytes[4]&128)) {
                received.rx_errors++;continue;
            }
            for(unsigned k=0;k<8;k++)received.status[k]=bytes[k];
            received.status_tick=now;received.status_seen=1;
        } else if(header.StdId==0x355) {
            for(unsigned k=0;k<8;k++)received.soc[k]=bytes[k];
            received.soc_tick=now;received.soc_seen=1;
        } else if(header.StdId==0x356) {
            for(unsigned k=0;k<8;k++)received.temp[k]=bytes[k];
            received.temp_tick=now;received.temp_seen=1;
        }
    }
}
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan) {
    if(hcan->Instance==CAN1)received.rx_errors++;
}
static uint8_t read_adc(uint32_t channel,uint16_t* raw,uint16_t low,uint16_t high,uint16_t* normalized) {
    ADC_ChannelConfTypeDef conf={0};conf.Channel=channel;conf.Rank=ADC_REGULAR_RANK_1;conf.SamplingTime=ADC_SAMPLETIME_28CYCLES;
    uint8_t ok=HAL_ADC_ConfigChannel(&hadc3,&conf)==HAL_OK;
    if(ok)ok=HAL_ADC_Start(&hadc3)==HAL_OK;
    if(ok)ok=HAL_ADC_PollForConversion(&hadc3,1)==HAL_OK;
    *raw=UINT16_MAX;*normalized=0;
    if(ok) *raw=(uint16_t)HAL_ADC_GetValue(&hadc3);
    if(HAL_ADC_Stop(&hadc3)!=HAL_OK)ok=0;
    if(!ok || *raw<low-ADC_MARGIN || *raw>high+ADC_MARGIN) {adc_errors++;return 0;}
    uint16_t bounded=*raw;if(bounded<low)bounded=low;if(bounded>high)bounded=high;
    *normalized=(uint16_t)(((uint32_t)(bounded-low)*4095u)/(high-low));return 1;
}
static void transmit(uint32_t id,uint8_t bytes[8]) {
    CAN_TxHeaderTypeDef header={0};uint32_t mailbox=0;
    header.StdId=id;header.IDE=CAN_ID_STD;header.RTR=CAN_RTR_DATA;header.DLC=8;header.TransmitGlobalTime=DISABLE;
    if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)==0 || HAL_CAN_AddTxMessage(&hcan1,&header,bytes,&mailbox)!=HAL_OK)tx_errors++;
}
void Kart_Init(void) {
    memset(&start_button,0,sizeof(start_button));
    memset(&ui,0,sizeof(ui));ui.mode_applied=255;ui.can_age=ui.status_age=UINT32_MAX;
    CAN_FilterTypeDef filter={0};filter.FilterBank=0;filter.FilterMode=CAN_FILTERMODE_IDMASK;
    filter.FilterScale=CAN_FILTERSCALE_32BIT;filter.FilterFIFOAssignment=CAN_RX_FIFO0;filter.FilterActivation=ENABLE;filter.SlaveStartFilterBank=14;
    if(HAL_CAN_ConfigFilter(&hcan1,&filter)!=HAL_OK || HAL_CAN_Start(&hcan1)!=HAL_OK ||
       HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING|CAN_IT_ERROR|CAN_IT_BUSOFF|CAN_IT_RX_FIFO0_OVERRUN)!=HAL_OK)Kart_Fatal();
    CoreDebug->DEMCR|=CoreDebug_DEMCR_TRCENA_Msk;DWT->CYCCNT=0;DWT->CTRL|=DWT_CTRL_CYCCNTENA_Msk;
    GPIO_InitTypeDef gpio={0};gpio.Mode=GPIO_MODE_IT_FALLING;gpio.Pull=GPIO_NOPULL;
    gpio.Pin=HALL_DI_ESQ_Pin;HAL_GPIO_Init(HALL_DI_ESQ_GPIO_Port,&gpio);
    gpio.Pin=HALL_DI_DIR_Pin;HAL_GPIO_Init(HALL_DI_DIR_GPIO_Port,&gpio);
    HAL_NVIC_SetPriority(EXTI4_IRQn,5,0);HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn,5,0);HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    watchdog.Instance=IWDG;watchdog.Init.Prescaler=IWDG_PRESCALER_64;watchdog.Init.Reload=1000;watchdog.Init.Window=IWDG_WINDOW_DISABLE;
    // Aproximadamente 2 s con LSI nominal; tolerancia del LSI no se presenta como tiempo medido.
    if(HAL_IWDG_Init(&watchdog)!=HAL_OK)Kart_Fatal();
    watchdog_ready=1;initialized=1;
}
void Kart_ControlTick(void) {
    if(!initialized)return;
    KartUiSnapshot next={0};CanData rx;
    uint32_t key=lock_data();memcpy(&rx,(const void*)&received,sizeof(rx));next.sd_state=sd_state;next.sd_error=sd_error;next.sd_rows=sd_rows;unlock_data(key);
    uint32_t now=HAL_GetTick();
    next.input_flags=read_adc(ADC_CHANNEL_0,&next.accel_adc,ACCEL_MIN,ACCEL_MAX,&next.accel_raw)?KART_INPUT_ACCEL_OK:0;
    if(read_adc(ADC_CHANNEL_8,&next.steer_adc,STEER_MIN,STEER_MAX,&next.steer_raw))next.input_flags|=KART_INPUT_STEER_OK;
    next.brake=HAL_GPIO_ReadPin(BRAKE_GPIO_Port,BRAKE_Pin)==GPIO_PIN_RESET;
    const uint8_t start_pressed=HAL_GPIO_ReadPin(START_STOP_GPIO_Port,START_STOP_Pin)==GPIO_PIN_SET;
    next.start=read_start_button(start_pressed,HAL_GetTick());
    const uint8_t drive=HAL_GPIO_ReadPin(MODE_D_GPIO_Port,MODE_D_Pin)==GPIO_PIN_SET;
    const uint8_t reverse=HAL_GPIO_ReadPin(MODE_R_GPIO_Port,MODE_R_Pin)==GPIO_PIN_SET;
    if(!(drive&&reverse))next.input_flags|=KART_INPUT_BUTTONS_OK;
    now=HAL_GetTick();next.tick=now;next.can_age=age(rx.feedback_seen,now,rx.feedback_tick);next.status_age=age(rx.status_seen,now,rx.status_tick);
    next.feedback_fresh=next.can_age<=FEEDBACK_MAX_AGE;next.status_fresh=next.status_age<=STATUS_MAX_AGE;
    next.status_seen=rx.status_seen;
    next.speed_kph=rx.speed;next.rpm_left=rx.left;next.rpm_right=rx.right;next.yaw_deci_dps=rx.yaw;
    next.backend_flags=next.status_fresh?rx.status[4]:0;next.inhibit=next.status_fresh?rx.status[1]:255;
    next.mode_received=next.status_fresh?rx.status[7]:255;next.mode_applied=next.status_fresh?rx.status[3]:255;
    // Si hay estado extendido, se exige siempre fresco. Cero entero km/h nunca basta.
    const uint8_t feedback_ok=next.feedback_fresh && (!rx.status_seen || (next.status_fresh && (next.backend_flags&KART_STATUS_SPEED_VALID)));
    const uint8_t stationary=feedback_ok && (rx.status_seen?(next.backend_flags&KART_STATUS_STATIONARY):(!rx.left&&!rx.right));
    const uint8_t pedal_free=next.accel_raw<=40;
    if((drive && selected_gear==2) || (reverse && selected_gear==1))selected_gear=0;
    if(next.input_flags!=7 || !feedback_ok || (!drive&&!reverse))selected_gear=0;
    else if(drive && selected_gear!=1 && next.brake && pedal_free && stationary)selected_gear=1;
    else if(reverse && selected_gear!=2 && next.brake && pedal_free && stationary)selected_gear=2;
    if(mode_click) {
        mode_click=0;notice_tick=now;
        if(next.input_flags!=7)notice=KART_UI_INPUT_BAD;
        else if(!feedback_ok)notice=KART_UI_CAN_OLD;
        else if(drive||reverse||selected_gear)notice=KART_UI_REQUIRE_N;
        else if(!stationary || !pedal_free)notice=KART_UI_REQUIRE_STOP;
        else {selected_mode=(uint8_t)((selected_mode+1)%3);notice=KART_UI_OK;}
    }
    if(now-notice_tick>3000)notice=KART_UI_OK;
    next.ui_notice=notice;next.gear=selected_gear;next.mode_requested=selected_mode;
    uint8_t health[8]={1,next.input_flags,0,0,0,0,0,0};
    kart_put_u16be(health+2,next.accel_adc);kart_put_u16be(health+4,next.steer_adc);kart_put_u16be(health+6,++sample_sequence);
    uint8_t command[8]={0,0,7,255,1,0,0,(uint8_t)(selected_mode|(next.start<<2))};
    if(next.input_flags==7 && feedback_ok) {
        kart_put_u16be(command,next.accel_raw);kart_put_u16be(command+2,next.steer_raw);
        command[4]=next.brake;command[5]=selected_gear==1;command[6]=selected_gear==2;
    }
    // Sin retransmisión automática de órdenes viejas; cada ciclo entrega la muestra actual.
    if(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1)==0) {
        HAL_CAN_AbortTxRequest(&hcan1,CAN_TX_MAILBOX0|CAN_TX_MAILBOX1|CAN_TX_MAILBOX2);tx_errors++;
    }
    transmit(KART_CAN_INPUT_HEALTH,health);transmit(KART_CAN_COMMAND,command);
    key=lock_data();
    const uint32_t tick_l=hall_l_tick,tick_r=hall_r_tick,period_l=hall_l_period,period_r=hall_r_period;
    const uint8_t seen_l=hall_l_seen,seen_r=hall_r_seen;
    unlock_data(key);
    if(seen_l && period_l && now-tick_l<=500){uint32_t rpm=(uint32_t)((uint64_t)SystemCoreClock*15u/period_l);next.rpm_front_left=rpm<=2000?(int16_t)rpm:0;}
    if(seen_r && period_r && now-tick_r<=500){uint32_t rpm=(uint32_t)((uint64_t)SystemCoreClock*15u/period_r);next.rpm_front_right=rpm<=2000?(int16_t)rpm:0;}
    next.bms_soc_age=age(rx.soc_seen,now,rx.soc_tick);next.bms_temp_age=age(rx.temp_seen,now,rx.temp_tick);
    memcpy(next.bms355,rx.soc,8);memcpy(next.bms356,rx.temp,8);
    const uint16_t soc=(uint16_t)(((uint16_t)rx.soc[5]<<8)|rx.soc[4]);
    const int16_t temp=(int16_t)(((uint16_t)rx.temp[5]<<8)|rx.temp[4]);
    next.soc_valid=BMS_FORMAT_CONFIRMED && next.bms_soc_age<=BMS_MAX_AGE && soc<=10000;
    next.temp_valid=BMS_FORMAT_CONFIRMED && next.bms_temp_age<=BMS_MAX_AGE && temp>=-500 && temp<=1500;
    next.soc_pct=soc/100;next.battery_temp_c=temp/10;
    next.can_rx_errors=rx.rx_errors;next.can_tx_errors=tx_errors;next.adc_errors=adc_errors;next.sd_requested=record_request;
    key=lock_data();ui=next;unlock_data(key);
    if(watchdog_ready && HAL_IWDG_Refresh(&watchdog)!=HAL_OK)Kart_Fatal();
}
static void sd_publish(uint8_t state,uint8_t error) {
    // Contar la espera desde el fallo, no desde el inicio de una llamada bloqueante.
    if(state==KART_SD_ERROR)sd_last_try=HAL_GetTick();
    uint32_t key=lock_data();sd_state=state;sd_error=error;unlock_data(key);
}
static void sd_close(void) {
    if(sd_open) {FRESULT sync=f_sync(&sd_file),close=f_close(&sd_file);sd_open=0;
        if(sync!=FR_OK || close!=FR_OK)sd_publish(KART_SD_ERROR,(uint8_t)(sync!=FR_OK?sync:close));}
    f_mount(NULL,"",0);
    // Sync/cierre/desmontaje tambien pueden tardar: esperar despues de completarlos.
    if(sd_state==KART_SD_ERROR)sd_last_try=HAL_GetTick();
}
static void sd_pause(void) {
    if(sd_open)sd_close();
    if(sd_state!=KART_SD_ERROR)sd_publish((uint8_t)(sd_error?KART_SD_ERROR:KART_SD_STOPPED),sd_error);
}
static uint8_t sd_write(const char* text) {
    UINT actual=0;const UINT expected=(UINT)strlen(text);
    FRESULT result=f_write(&sd_file,text,expected,&actual);
    if(result!=FR_OK || actual!=expected) {sd_publish(KART_SD_ERROR,(uint8_t)(result!=FR_OK?result:FR_DISK_ERR));return 0;}
    return 1;
}
void Kart_SdTick(void) {
    uint32_t now=HAL_GetTick();
    if(!record_request) {sd_pause();return;}
    if(!sd_open) {
        if(sd_state==KART_SD_ERROR && now-sd_last_try<1000)return;
        // E conserva el ultimo fallo durante el reintento; se borra al grabar.
        sd_publish(KART_SD_OPENING,sd_error);
        FRESULT result=f_mount(&sd_fs,"",1);
        if(result!=FR_OK) {f_mount(NULL,"",0);sd_publish(KART_SD_ERROR,(uint8_t)result);return;}
        if(!record_request) {sd_close();sd_pause();return;}
        result=f_open(&sd_file,"kart_v13.csv",FA_OPEN_APPEND|FA_WRITE);
        if(result!=FR_OK) {f_mount(NULL,"",0);sd_publish(KART_SD_ERROR,(uint8_t)result);return;}
        sd_open=1;
        if(!record_request) {sd_pause();return;}
        if(f_size(&sd_file)==0 && !sd_write("# firmware=" KART_FRONTEND_VERSION ",schema=2,BMS_format_confirmed=0\r\nFirmware,Tick_ms,AccelADC,SteerADC,AccelCAN,SteerCAN,InputFlags,Brake,Start,GearReq,ModeReq,ModeRecv,ModeApplied,Inhibit,BackendFlags,Vel_kmh,RpmFL,RpmFR,RpmRL,RpmRR,Yaw_deci_dps,CanAge_ms,StatusAge_ms,SOC_valid,SOC_pct,Temp_valid,Temp_C,BMS355Age_ms,BMS356Age_ms,BMS355_hex,BMS356_hex,CanTxErr,CanRxErr,AdcErr\r\n")) {sd_close();return;}
        if(!record_request) {sd_pause();return;}
        now=HAL_GetTick();
        sd_publish(KART_SD_RECORDING,0);sd_last_log=now-200;sd_last_sync=now;
    }
    if(now-sd_last_log>=200) {
        sd_last_log=now;KartUiSnapshot snapshot;Kart_CopySnapshot(&snapshot);
        char raw355[17],raw356[17],line[512];
        for(unsigned i=0;i<8;i++){snprintf(raw355+i*2,3,"%02X",snapshot.bms355[i]);snprintf(raw356+i*2,3,"%02X",snapshot.bms356[i]);}
        const int n=snprintf(line,sizeof(line),KART_FRONTEND_VERSION ",%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%d,%d,%d,%d,%d,%d,%lu,%lu,%u,%u,%u,%d,%lu,%lu,%s,%s,%lu,%lu,%lu\r\n",
            (unsigned long)snapshot.tick,snapshot.accel_adc,snapshot.steer_adc,snapshot.accel_raw,snapshot.steer_raw,snapshot.input_flags,snapshot.brake,snapshot.start,
            snapshot.gear,snapshot.mode_requested,snapshot.mode_received,snapshot.mode_applied,snapshot.inhibit,snapshot.backend_flags,snapshot.speed_kph,
            snapshot.rpm_front_left,snapshot.rpm_front_right,snapshot.rpm_left,snapshot.rpm_right,snapshot.yaw_deci_dps,
            (unsigned long)snapshot.can_age,(unsigned long)snapshot.status_age,snapshot.soc_valid,snapshot.soc_pct,snapshot.temp_valid,snapshot.battery_temp_c,
            (unsigned long)snapshot.bms_soc_age,(unsigned long)snapshot.bms_temp_age,raw355,raw356,(unsigned long)snapshot.can_tx_errors,(unsigned long)snapshot.can_rx_errors,(unsigned long)snapshot.adc_errors);
        if(n<0 || n>=(int)sizeof(line)){sd_publish(KART_SD_ERROR,FR_INT_ERR);sd_close();return;}
        if(!sd_write(line)){sd_close();return;}
        uint32_t key=lock_data();sd_rows++;unlock_data(key);
    }
    if(now-sd_last_sync>=1000) {
        sd_last_sync=now;FRESULT result=f_sync(&sd_file);
        if(result!=FR_OK){sd_publish(KART_SD_ERROR,(uint8_t)result);sd_close();}
    }
}
void Kart_Fatal(void) {
    // Parar la emisión hace expirar 0x100 en el ESP32. No continuar tras init fallida.
    __disable_irq();for(;;) {__NOP();} // IWDG reinicia si ya se había habilitado; nuevo ESP32 exige nuevo START.
}
