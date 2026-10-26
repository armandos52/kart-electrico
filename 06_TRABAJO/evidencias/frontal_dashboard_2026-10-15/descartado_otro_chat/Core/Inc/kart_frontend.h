#ifndef KART_FRONTEND_H
#define KART_FRONTEND_H
#include <stdint.h>
#include "../../../common/kart_protocol.h"
#ifdef __cplusplus
extern "C" {
#endif
#define KART_FRONTEND_VERSION "stm32_frontal-v1.3-dev6"
// Edades maximas para aceptar tramas CAN y umbral de pedal libre (normalizado 0-4095).
// Valores de desarrollo: PROB-0012 muestra que el reposo medido supera este umbral; no se ha recalibrado.
#define KART_FEEDBACK_MAX_AGE_MS 500u
#define KART_STATUS_MAX_AGE_MS 300u
#define KART_PEDAL_FREE_MAX 40u
enum { KART_SD_STOPPED=0,KART_SD_RECORDING=1,KART_SD_ERROR=2,KART_SD_OPENING=3 };
enum { KART_UI_OK=0,KART_UI_REQUIRE_N=1,KART_UI_REQUIRE_STOP=2,KART_UI_CAN_OLD=3,KART_UI_INPUT_BAD=4 };
// Condiciones para engranar D/R (bit = condicion cumplida). Una sola definicion para control y pantalla.
enum {
    KART_DRIVE_COND_INPUTS=1,KART_DRIVE_COND_CAN=2,KART_DRIVE_COND_SELECTOR=4,
    KART_DRIVE_COND_BRAKE=8,KART_DRIVE_COND_PEDAL=16,KART_DRIVE_COND_STOPPED=32,KART_DRIVE_COND_ALL=63
};
// Condiciones para cambiar de modo DIF/ESC/BLQ (bit = condicion cumplida).
enum {
    KART_MODE_COND_INPUTS=1,KART_MODE_COND_CAN=2,KART_MODE_COND_NEUTRAL=4,
    KART_MODE_COND_STOPPED=8,KART_MODE_COND_PEDAL=16,KART_MODE_COND_ALL=31
};
// SELECTOR cumplido: D o R, nunca ambos ni ninguno. INPUTS: ADC pedal/direccion y botones validos (input_flags==7).
static inline uint8_t kart_drive_conditions(uint8_t input_flags,uint8_t feedback_ok,uint8_t drive,uint8_t reverse,
                                            uint8_t brake,uint8_t pedal_free,uint8_t stationary) {
    uint8_t mask=0;
    if(input_flags==7)mask|=KART_DRIVE_COND_INPUTS;
    if(feedback_ok)mask|=KART_DRIVE_COND_CAN;
    if(drive||reverse)mask|=KART_DRIVE_COND_SELECTOR;
    if(brake)mask|=KART_DRIVE_COND_BRAKE;
    if(pedal_free)mask|=KART_DRIVE_COND_PEDAL;
    if(stationary)mask|=KART_DRIVE_COND_STOPPED;
    return mask;
}
static inline uint8_t kart_mode_conditions(uint8_t input_flags,uint8_t feedback_ok,uint8_t drive,uint8_t reverse,
                                           uint8_t gear,uint8_t pedal_free,uint8_t stationary) {
    uint8_t mask=0;
    if(input_flags==7)mask|=KART_MODE_COND_INPUTS;
    if(feedback_ok)mask|=KART_MODE_COND_CAN;
    if(!drive && !reverse && !gear)mask|=KART_MODE_COND_NEUTRAL;
    if(stationary)mask|=KART_MODE_COND_STOPPED;
    if(pedal_free)mask|=KART_MODE_COND_PEDAL;
    return mask;
}
typedef struct {
    uint32_t tick,can_age,status_age,bms_soc_age,bms_temp_age,can_tx_errors,can_rx_errors,adc_errors,sd_rows;
    uint16_t accel_adc,steer_adc,accel_raw,steer_raw;
    int16_t speed_kph,rpm_left,rpm_right,yaw_deci_dps,rpm_front_left,rpm_front_right;
    uint16_t soc_pct;
    int16_t battery_temp_c;
    uint8_t input_flags,brake,start,gear,mode_requested,mode_received,mode_applied,inhibit,backend_flags;
    uint8_t feedback_fresh,status_fresh,status_seen,soc_valid,temp_valid,sd_requested,sd_state,sd_error,ui_notice;
    uint8_t bms355[8],bms356[8];
    // Condiciones de marcha tal como las evaluó el control en este ciclo (página «Condiciones de marcha»).
    uint8_t drive_input,reverse_input,pedal_free,stationary,feedback_ok,drive_cond,mode_cond;
} KartUiSnapshot;
void Kart_Init(void);
void Kart_ControlTick(void);
void Kart_SdTick(void);
void Kart_CopySnapshot(KartUiSnapshot* destination);
void Kart_RequestNextMode(void);
void Kart_ToggleRecording(void);
void Kart_HallEdge(uint16_t pin);
void Kart_Fatal(void);
#ifdef __cplusplus
}
#endif
#endif
