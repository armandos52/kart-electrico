#pragma once
#include "kart_frontend.h"
enum { BENCH_SCENE_COUNT=10 };
struct BenchScene {const char* title;KartUiSnapshot snapshot;char lines[5][41];};
BenchScene benchScene(unsigned index);
bool benchSpeedVisible(const KartUiSnapshot& s);
int benchVisualTests();
