#include <cstdio>
#include <cstdarg>
#include "../../../07_DESARROLLO/firmware/nucleo_f411_banco/src/bench_logic.h"
#include "../../../07_DESARROLLO/firmware/nucleo_f411_banco/src/visual_scenes.h"
int benchPrintf(const char* fmt,...) {
    va_list args;va_start(args,fmt);const int n=vprintf(fmt,args);va_end(args);return n;
}
int main() {
    const int regression=benchRunTests();const int visual=benchVisualTests();
    for(unsigned i=0;i<BENCH_SCENE_COUNT;i++) {
        const auto scene=benchScene(i);printf("SCENE %u %s\n",i,scene.title);
        for(unsigned row=0;row<5;row++)printf("LINE%u %s\n",row,scene.lines[row]);
    }
    return regression || visual;
}
