// Requisitos propuestos para la siguiente implementacion. Fallar aqui identifica
// un comportamiento reproducido del fuente dev3, no una medida del kart.
#define main baseline_main
#include "fuentes/07_DESARROLLO/firmware/esp32_trasero/tests/control_test.cpp"
#undef main

int main() {
    for(Gear gear:{Gear::Drive,Gear::Reverse}) {
        Fixture f;f.steer(3005);f.drive(gear,Mode::Dif);
        const float split=0.5f/1.3f*std::tan(20.0f*3.14159265358979323846f/180.0f);
        const float ratio=(1-split)/(1+split);
        std::printf("RAMP gear=%u left=%.9f right=%.9f expected_ratio=%.9f actual_ratio=%.9f\n",
            unsigned(gear),f.out.effort_left,f.out.effort_right,ratio,f.out.effort_right/f.out.effort_left);
        check(std::fabs(f.out.effort_right/f.out.effort_left-ratio)<0.00001f,
            "propuesta: conservar reparto geometrico durante arranque D/R");
        for(int n=0;n<400;n++)f.step();
        check(std::fabs(f.out.effort_right/f.out.effort_left-ratio)<0.00001f,
            "control positivo: reparto geometrico despues de rampa D/R");
    }
    for(Mode mode:{Mode::Dif,Mode::Esc,Mode::Equal}) {
        Fixture coast;coast.drive(Gear::Reverse,mode);
        coast.throttle(0);coast.s.rpm_left=coast.s.rpm_right=21.0f/rpmToKph(1,0.48f);coast.step();
        coast.s.rpm_left=coast.s.rpm_right=19.5f/rpmToKph(1,0.48f);coast.throttle(4095);coast.step();
        std::printf("COAST mode=%u speed=%.6f left=%.9f right=%.9f inhibit=%s\n",
            unsigned(mode),coast.out.speed_kph,coast.out.effort_left,coast.out.effort_right,inhibitName(coast.out.inhibit));
        check(coast.out.effort_left==0 && coast.out.effort_right==0,
            "propuesta: exceso a pedal libre memoriza corte hasta <=19");
        // El supervisor usa la rueda mas rapida, no la media de pantalla.
        Fixture peak;peak.drive(Gear::Reverse,mode);
        peak.s.rpm_left=50.0f/rpmToKph(1,0.48f);peak.s.rpm_right=0;peak.step();
        check(peak.out.effort_left==0 && peak.out.effort_right==0 && peak.out.inhibit==Inhibit::ReverseLimit,
            "control positivo: una rueda a 50 corta ambos mandos en R");
        peak.s.rpm_left=peak.s.rpm_right=19.5f/rpmToKph(1,0.48f);peak.step();
        check(peak.out.effort_left==0 && peak.out.effort_right==0,
            "control positivo: histeresis con pedal sostenido");
        peak.s.speed_valid=false;peak.step();
        check(peak.out.effort_left==0 && peak.out.effort_right==0 && peak.out.rearm_required,
            "control positivo: velocidad invalida inhibe y exige rearme");
    }
    std::printf("TOTAL %d; FALLOS %d (requisitos propuestos no implementados)\n",checks,failures);
    return failures?1:0;
}
