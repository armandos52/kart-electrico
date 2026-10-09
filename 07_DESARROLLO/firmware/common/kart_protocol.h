#ifndef KART_PROTOCOL_H
#define KART_PROTOCOL_H
#include <stdint.h>
#define KART_CAN_COMMAND 0x100u
#define KART_CAN_INPUT_HEALTH 0x101u
#define KART_CAN_FEEDBACK 0x123u
#define KART_CAN_STATUS 0x124u
#define KART_PROTOCOL_VERSION 1u
#define KART_INPUT_ACCEL_OK 1u
#define KART_INPUT_STEER_OK 2u
#define KART_INPUT_BUTTONS_OK 4u
#define KART_STATUS_READY 1u
#define KART_STATUS_REARM 2u
#define KART_STATUS_SPEED_VALID 4u
#define KART_STATUS_YAW_VALID 8u
#define KART_STATUS_STATIONARY 16u
#define KART_STATUS_REVERSE_LIMIT 32u
#define KART_STATUS_FRONTEND_HEALTH 64u
#define KART_MODE_NOT_APPLIED 255u
static inline uint16_t kart_u16be(const uint8_t* b) {return (uint16_t)(((uint16_t)b[0]<<8)|b[1]);}
static inline void kart_put_u16be(uint8_t* b,uint16_t x) {b[0]=(uint8_t)(x>>8);b[1]=(uint8_t)x;}
#endif
