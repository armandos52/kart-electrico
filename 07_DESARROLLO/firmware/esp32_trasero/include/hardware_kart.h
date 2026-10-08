#pragma once
#include <stdint.h>
namespace hardware {
constexpr uint8_t CAN_CS=5, MOSI=23, MISO=19, SCK=18, SDA=21, SCL=22;
constexpr uint8_t HALL_L=34, HALL_R=4, PWM_L=32, PWM_R=2;
constexpr uint8_t BRAKE_PWM_L=33, BRAKE_PWM_R=15;
constexpr uint8_t FORWARD_L=25, REVERSE_L=26, BRAKE_L=27;
constexpr uint8_t FORWARD_R=13, REVERSE_R=12, BRAKE_R=14;
constexpr uint8_t RELAY1=16, RELAY2=17, CHARGER=35;
constexpr uint16_t PULSES_LEFT=89, PULSES_RIGHT=68; // históricos: verificar físicamente
constexpr uint32_t PWM_HZ=5000, HALL_WINDOW_MS=100, HALL_MAX_AGE_MS=250;
constexpr uint32_t GYRO_PERIOD_MS=10, GYRO_MAX_AGE_MS=30;
constexpr bool LEFT_INVERTED=true;
}
