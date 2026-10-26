// Reutiliza cuerpos originales y stubs conservados en la auditoria anterior.
#define main historical_cases_main
#include "../AUD_V12_2026-10-08/control_host.cpp"
#undef main
int main() {
    for(int mode=0;mode<3;mode++) {
        base();modo_d=0;modo_r=1;modo_anterior=1;modo_controlo=mode;
        // RPM de entrada sinteticas equivalentes a 50 km/h con D=0.48 m.
        rpm_esq=rpm_dir=rpm_medio=int(50.0/(PI*0.48*0.06));
        processarComando();
        std::printf("V12 mode=%d reverse_pin=%d pwm_left=%d pwm_right=%d (0=pleno,255=cero)\n",
            mode,pins[SW_DIR_MT_DIR],motor_esq_speed,motor_dir_speed);
        result("reproduccion: R a velocidad elevada conserva PWM pleno",
            pins[SW_DIR_MT_DIR]==HIGH && motor_esq_speed==0 && motor_dir_speed==0);
    }
    std::printf("TOTAL 3; DISCREPANCIAS %d (reproducir defecto no es aprobar firmware)\n",failed);
    return failed?1:0;
}
