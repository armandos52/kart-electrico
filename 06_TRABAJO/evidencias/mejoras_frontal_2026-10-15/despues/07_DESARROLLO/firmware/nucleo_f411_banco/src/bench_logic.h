#pragma once
#include <stdint.h>
#define BENCH_VERSION "0.4-dev"
struct BenchState {
    uint32_t tick_ms;
    uint8_t raw_pressed, start, relay, mode, inhibit;
    int checks, failures;
};
int benchRunTests();
void benchBegin();
BenchState benchTick(uint32_t now, bool pressed);
int benchPrintf(const char* format, ...);
