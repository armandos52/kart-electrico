#include "kart_control.h"
#include <cstdio>
#include <cmath>
#include <limits>
#include <algorithm>
using namespace kart;
static int checks=0,failures=0;
void check(bool condition,const char* name) {
    ++checks;if(!condition){++failures;std::printf("FAIL %s\n",name);}else std::printf("OK %s\n",name);
}
struct Fixture {
    Controller c;
    Sensors s;
    Output out;
    uint32_t now=0;
    uint8_t b[8]={0,0,7,255,1,0,0,0};
    explicit Fixture(Config cfg=Config()):c(cfg) {s.speed_valid=true;s.yaw_valid=true;s.pulse_left_recent=s.pulse_right_recent=true;}
    Output step(uint32_t dt=5,bool receive=true) {now+=dt;if(receive)c.receive(b,8,now);out=c.step(s,now);return out;}
    void throttle(uint16_t value) {b[0]=uint8_t(value>>8);b[1]=uint8_t(value);}
    void steer(uint16_t value) {b[2]=uint8_t(value>>8);b[3]=uint8_t(value);}
    void start(Mode mode=Mode::Dif) {
        b[7]=uint8_t(mode);step();step(600);b[7]|=4;step();step(2000);step(1000);
    }
    void gear(Gear gear) {b[4]=1;throttle(0);b[5]=gear==Gear::Drive;b[6]=gear==Gear::Reverse;step();}
    void drive(Gear gear=Gear::Drive,Mode mode=Mode::Dif) {start(mode);this->gear(gear);b[4]=0;throttle(4095);step();}
};
int main() {
    Command parsed;
    uint8_t good[8]={0,0,7,255,1,0,0,4};
    check(decodeCommand(good,8,parsed),"CAN valido");
    check(!decodeCommand(good,7,parsed),"DLC invalido");
    check(!decodeCommand(nullptr,8,parsed),"buffer ausente");
    for(unsigned idx:{0u,2u,4u,5u,6u,7u}) {
        uint8_t bad[8];std::copy(good,good+8,bad);bad[idx]=255;
        check(!decodeCommand(bad,8,parsed),"rechazar campo fuera de rango");
    }
    uint8_t both[8]={0,0,7,255,0,1,1,4};check(!decodeCommand(both,8,parsed),"D y R simultaneos");
    uint8_t mode3[8]={0,0,7,255,0,1,0,7};check(!decodeCommand(mode3,8,parsed),"modo 3 rechazado");
    Fixture startup;startup.b[7]=4;startup.step(1000);startup.step(4000);
    check(startup.out.relay==Relay::Off,"START alto al encender no arranca");
    Fixture stop;stop.step();stop.step(600);stop.b[7]=4;stop.step();
    check(stop.out.relay==Relay::Precharge,"inicio precarga");
    stop.b[7]=0;stop.step(100);check(stop.out.relay==Relay::Off,"STOP a 100ms no se pierde");
    stop.step(4000);check(stop.out.relay==Relay::Off,"STOP persistente no reactiva");
    Fixture f;f.drive();
    check(f.out.relay==Relay::Ready,"traccion solo tras PRONTO");
    check(f.out.effort_left>0 && f.out.effort_left<0.004f,"rampa aplicada primer paso 5ms");
    float before=f.out.effort_left;f.step();check(f.out.effort_left-before<=300.0f/570*0.005f+1e-6f,"tasa rampa por salida");
    f.b[4]=1;f.step();check(f.out.effort_left==0 && f.out.direction==Gear::Neutral,"freno inmediato");
    f.b[4]=0;f.step();check(f.out.rearm_required && f.out.effort_left==0,"doble pedal exige liberacion");
    Fixture timeout;timeout.drive();timeout.step(101,false);
    check(timeout.out.inhibit==Inhibit::CanTimeout && timeout.out.effort_left==0,"timeout corta");
    timeout.step();check(timeout.out.effort_left==0 && timeout.out.rearm_required,"CAN recuperado no rearranca pisado");
    timeout.gear(Gear::Neutral);timeout.gear(Gear::Drive);timeout.b[4]=0;timeout.throttle(4095);timeout.step();
    check(timeout.out.effort_left>0,"rearme deliberado N freno pedal libre");
    Fixture charger;charger.drive();charger.s.charger=true;charger.step();
    check(charger.out.relay==Relay::Off && charger.out.effort_left==0,"cargador anula PWM y reles juntos");
    charger.s.charger=false;charger.step();check(charger.out.relay==Relay::Off,"quitar cargador no autoarranca");
    Fixture invalid;invalid.drive();invalid.b[4]=2;invalid.step();
    check(invalid.out.inhibit==Inhibit::InvalidCommand && invalid.out.effort_left==0,"dato invalido corta");
    Fixture neutral;neutral.drive();neutral.b[5]=neutral.b[6]=0;neutral.step();
    check(neutral.out.effort_left==0 && neutral.out.effort_right==0,"N nunca entrega consigna transitoria");
    for(Mode mode:{Mode::Dif,Mode::Esc,Mode::Equal}) {
        Fixture reverse;reverse.drive(Gear::Reverse,mode);reverse.steer(3005);
        for(int i=0;i<300;i++)reverse.step();
        const float cap=20.0f/rpmToKph(570,0.48f);
        check(reverse.out.effort_left<=cap+1e-6f && reverse.out.effort_right<=cap+1e-6f,"techo R 20 km/h por rueda todos modos");
        reverse.s.rpm_left=reverse.s.rpm_right=20.01f/rpmToKph(1,0.48f);reverse.step();
        check(reverse.out.effort_left==0 && reverse.out.effort_right==0,"R a 20 corta sin esperar rampa");
        reverse.s.rpm_left=reverse.s.rpm_right=19.5f/rpmToKph(1,0.48f);reverse.step();
        check(reverse.out.effort_left==0,"histeresis no oscila bajo 20");
        reverse.s.rpm_left=reverse.s.rpm_right=18.5f/rpmToKph(1,0.48f);reverse.step();
        check(reverse.out.effort_left>0 && reverse.out.effort_left<0.004f,"R recupera con rampa al bajar de 19");
    }
    Fixture left,right;left.drive(Gear::Drive,Mode::Esc);right.drive(Gear::Drive,Mode::Esc);
    left.steer(3005);right.steer(1089);left.s.yaw_dps=30;right.s.yaw_dps=-30;
    left.s.rpm_left=right.s.rpm_right=150;left.s.rpm_right=right.s.rpm_left=100;
    for(int i=0;i<300;i++){left.step();right.step();}
    check(fabs(left.out.effort_left-right.out.effort_right)<1e-6f && fabs(left.out.effort_right-right.out.effort_left)<1e-6f,"ESC espejo firmado");
    left.s.yaw_valid=false;left.step();check(left.out.inhibit==Inhibit::MpuUnavailable && left.out.effort_left==0,"MPU perdido en marcha corta ESC");
    Fixture hall;hall.drive();hall.s.pulse_left_recent=false;hall.step(1501);
    check(hall.out.inhibit==Inhibit::HallFault && hall.out.effort_left==0,"Hall ausente tras gracia corta");
    Fixture quality;quality.drive();quality.s.speed_valid=false;quality.step();check(quality.out.effort_left==0,"velocidad invalida corta");
    Fixture front;front.drive();front.s.frontend_ok=false;front.step();check(front.out.inhibit==Inhibit::FrontendInvalid,"frontal invalido corta");
    Fixture outputs;outputs.drive();outputs.s.outputs_ok=false;outputs.step();check(outputs.out.relay==Relay::Off && outputs.out.effort_left==0,"fallo salida abre y para");
    Config wrong;wrong.esc_gain_per_dps=std::numeric_limits<float>::quiet_NaN();Fixture nan(wrong);nan.drive();
    check(nan.out.inhibit==Inhibit::Configuration && nan.out.effort_left==0,"configuracion no finita rechazada");
    std::printf("TOTAL %d; FALLOS %d\n",checks,failures);return failures?1:0;
}
