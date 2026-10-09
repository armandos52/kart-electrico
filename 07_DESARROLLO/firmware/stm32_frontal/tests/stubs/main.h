#pragma once
#include <stdint.h>
#include <cstring>
struct ADC_HandleTypeDef {};
struct CAN_HandleTypeDef {int Instance;};
struct ADC_ChannelConfTypeDef {uint32_t Channel,Rank,SamplingTime;};
struct CAN_RxHeaderTypeDef {uint32_t StdId,IDE,RTR,DLC;};
struct CAN_TxHeaderTypeDef {uint32_t StdId,IDE,RTR,DLC,TransmitGlobalTime;};
struct CAN_FilterTypeDef {int FilterBank,FilterMode,FilterScale,FilterFIFOAssignment,FilterActivation,SlaveStartFilterBank;};
struct GPIO_InitTypeDef {uint32_t Mode,Pull,Pin;};
struct IWDG_HandleTypeDef {int Instance;struct {int Prescaler,Reload,Window;} Init;};
struct FakeDwt {uint32_t CYCCNT,CTRL;};struct FakeDebug {uint32_t DEMCR;};
extern FakeDwt fakeDwt;extern FakeDebug fakeDebug;
#define DWT (&fakeDwt)
#define CoreDebug (&fakeDebug)
#define CoreDebug_DEMCR_TRCENA_Msk 1
#define DWT_CTRL_CYCCNTENA_Msk 1
#define HAL_OK 0
#define CAN1 1
#define CAN_ID_STD 0
#define CAN_RTR_DATA 0
#define CAN_RX_FIFO0 0
#define ADC_CHANNEL_0 0
#define ADC_CHANNEL_8 8
#define ADC_REGULAR_RANK_1 1
#define ADC_SAMPLETIME_28CYCLES 28
#define CAN_FILTERMODE_IDMASK 0
#define CAN_FILTERSCALE_32BIT 0
#define CAN_IT_RX_FIFO0_MSG_PENDING 1
#define CAN_IT_ERROR 2
#define CAN_IT_BUSOFF 4
#define CAN_IT_RX_FIFO0_OVERRUN 8
#define CAN_TX_MAILBOX0 1
#define CAN_TX_MAILBOX1 2
#define CAN_TX_MAILBOX2 4
#define DISABLE 0
#define ENABLE 1
#define GPIO_MODE_IT_FALLING 0
#define GPIO_NOPULL 0
#define GPIO_PIN_SET 1
#define GPIO_PIN_RESET 0
#define HALL_DI_ESQ_Pin 16
#define HALL_DI_DIR_Pin 128
#define HALL_DI_ESQ_GPIO_Port 0
#define HALL_DI_DIR_GPIO_Port 1
#define BRAKE_Pin 0
#define START_STOP_Pin 1
#define MODE_D_Pin 2
#define MODE_R_Pin 3
#define BRAKE_GPIO_Port 0
#define START_STOP_GPIO_Port 0
#define MODE_D_GPIO_Port 0
#define MODE_R_GPIO_Port 0
#define EXTI4_IRQn 4
#define EXTI9_5_IRQn 5
#define IWDG 1
#define IWDG_PRESCALER_64 64
#define IWDG_WINDOW_DISABLE 4095
extern uint32_t SystemCoreClock,tick,adc_channel;
extern uint16_t adc_accel_value,adc_steer_value;
extern int adc_ok,pins[4],mailboxes,tx_ok,tx_count,rx_pending,wd_refresh;
extern uint8_t sent_command[8],sent_health[8],rx_bytes[8];extern CAN_RxHeaderTypeDef rx_header;
inline uint32_t __get_PRIMASK(){return 0;}inline void __disable_irq(){}inline void __set_PRIMASK(uint32_t){}inline void __NOP(){}
inline uint32_t HAL_GetTick(){return tick;}
inline unsigned HAL_CAN_GetRxFifoFillLevel(CAN_HandleTypeDef*,int){return rx_pending;}
inline int HAL_CAN_GetRxMessage(CAN_HandleTypeDef*,int,CAN_RxHeaderTypeDef* h,uint8_t* b){*h=rx_header;memcpy(b,rx_bytes,8);rx_pending=0;return 0;}
inline int HAL_ADC_ConfigChannel(ADC_HandleTypeDef*,ADC_ChannelConfTypeDef* c){adc_channel=c->Channel;return adc_ok?0:1;}
inline int HAL_ADC_Start(ADC_HandleTypeDef*){return adc_ok?0:1;}
inline int HAL_ADC_PollForConversion(ADC_HandleTypeDef*,int){return adc_ok?0:1;}
inline uint32_t HAL_ADC_GetValue(ADC_HandleTypeDef*){return adc_channel==0?adc_accel_value:adc_steer_value;}
inline int HAL_ADC_Stop(ADC_HandleTypeDef*){return 0;}
inline unsigned HAL_CAN_GetTxMailboxesFreeLevel(CAN_HandleTypeDef*){return mailboxes;}
inline int HAL_CAN_AddTxMessage(CAN_HandleTypeDef*,CAN_TxHeaderTypeDef* h,uint8_t* b,uint32_t*){memcpy(h->StdId==0x100?sent_command:sent_health,b,8);tx_count++;return tx_ok?0:1;}
inline int HAL_CAN_AbortTxRequest(CAN_HandleTypeDef*,int){mailboxes=3;return 0;}
inline int HAL_CAN_ConfigFilter(CAN_HandleTypeDef*,CAN_FilterTypeDef*){return 0;}
inline int HAL_CAN_Start(CAN_HandleTypeDef*){return 0;}
inline int HAL_CAN_ActivateNotification(CAN_HandleTypeDef*,int){return 0;}
inline void HAL_GPIO_Init(int,GPIO_InitTypeDef*){}
inline void HAL_NVIC_SetPriority(int,int,int){}inline void HAL_NVIC_EnableIRQ(int){}
inline int HAL_IWDG_Init(IWDG_HandleTypeDef*){return 0;}
inline int HAL_IWDG_Refresh(IWDG_HandleTypeDef*){wd_refresh++;return 0;}
inline int HAL_GPIO_ReadPin(int,int pin){return pins[pin];}
