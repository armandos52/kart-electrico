#ifndef KART_FRONTEND_H
#define KART_FRONTEND_H
#include <stdint.h>
#include "../../../common/kart_protocol.h"
#ifdef __cplusplus
extern "C" {
#endif
#define KART_FRONTEND_VERSION "stm32_frontal-v1.3-dev2"
enum { KART_SD_STOPPED=0,KART_SD_RECORDING=1,KART_SD_ERROR=2,KART_SD_OPENING=3 };
enum { KART_UI_OK=0,KART_UI_REQUIRE_N=1,KART_UI_REQUIRE_STOP=2,KART_UI_CAN_OLD=3,KART_UI_INPUT_BAD=4 };
typedef struct {
    uint32_t tick,can_age,status_age,bms_soc_age,bms_temp_age,can_tx_errors,can_rx_errors,adc_errors,sd_rows;
    uint16_t accel_adc,steer_adc,accel_raw,steer_raw;
    int16_t speed_kph,rpm_left,rpm_right,yaw_deci_dps,rpm_front_left,rpm_front_right;
    uint16_t soc_pct;
    int16_t battery_temp_c;
    uint8_t input_flags,brake,start,gear,mode_requested,mode_received,mode_applied,inhibit,backend_flags;
    uint8_t feedback_fresh,status_fresh,status_seen,soc_valid,temp_valid,sd_requested,sd_state,sd_error,ui_notice;
    uint8_t bms355[8],bms356[8];
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
