#pragma once
#include <stdint.h>
// Avance al soltar una pulsacion estable; no controla START del kart.
class BenchNavigation {
    bool raw_=true, stable_=true, armed_=false, pressed_=false;
    uint32_t changed_=0;
public:
    void reset(uint32_t now) {raw_=stable_=true;armed_=pressed_=false;changed_=now;}
    bool sample(bool down,uint32_t now) {
        if(down!=raw_) {raw_=down;changed_=now;}
        if(now-changed_<40)return false;
        if(stable_!=raw_) {
            stable_=raw_;
            if(stable_) {if(armed_)pressed_=true;}
            else {
                const bool advance=armed_ && pressed_;
                armed_=true;pressed_=false;return advance;
            }
        }
        return false;
    }
};
