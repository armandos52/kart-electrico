#if !BENCH_DIAGNOSTIC_ONLY
// Reutiliza las pruebas y funciones reales sin alterar el firmware F746/ESP32.
// Esta unidad NO incluye Arduino/HAL real: CAN, ADC, SD y reloj son simulados.
#include <cstdio>
#include <string>
#include "bench_logic.h"
#define main benchOriginalTests
#define SystemCoreClock benchTestCoreClock
#define printf benchPrintf
#include "../../stm32_frontal/tests/frontend_test.cpp"
#undef printf
#undef SystemCoreClock
#undef main

static kart::Controller liveBackend;
int benchRunTests() {
    checks=failures=0;
    return benchOriginalTests();
}
void benchBegin() {
    reset();
    liveBackend=kart::Controller();
}
BenchState benchTick(uint32_t now, bool pressed) {
    tick=now;
    pins[1]=pressed?1:0;
    // Banco neutral: freno activo, pedal libre, sin movimiento. Feedback sintetico.
    pins[0]=pins[2]=pins[3]=0;
    feedback();
    Kart_ControlTick();
    kart::Sensors sensors;
    sensors.speed_valid=true;
    sensors.yaw_valid=sensors.pulse_left_recent=sensors.pulse_right_recent=true;
    liveBackend.receive(sent_command,8,now);
    const auto output=liveBackend.step(sensors,now);
    return {now,uint8_t(pressed),uint8_t(start_sent()),uint8_t(output.relay),
            uint8_t(output.selected_mode),uint8_t(output.inhibit),checks,failures};
}
#endif
