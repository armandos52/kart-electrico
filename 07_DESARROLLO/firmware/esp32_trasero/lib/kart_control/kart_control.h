#pragma once
#include <stdint.h>

// Lógica sin periféricos. Todas las salidas se deciden antes de aplicarlas.
namespace kart {
enum class Gear : uint8_t { Neutral=0, Drive=1, Reverse=2 };
enum class Mode : uint8_t { Dif=0, Esc=1, Equal=2 };
enum class Relay : uint8_t { Off=0, Precharge=1, Overlap=2, Ready=3 };
enum class Inhibit : uint8_t {
    None=0, NoCommand, CanTimeout, InvalidCommand, StartOff, Relays,
    Charger, Rearm, Brake, PedalZero, Neutral, DirectionChange,
    ModeChange, SpeedInvalid, HallFault, MpuUnavailable, FrontendInvalid,
    ReverseLimit, OutputFault, CanFault, Configuration
};
const char* inhibitName(Inhibit value);

struct Config {
    // Geometría/PPR de referencia histórica, pendientes de contraste físico.
    float wheelbase_m=1.3f;
    float track_m=1.0f;
    float wheel_diameter_m=0.48f;
    float reference_motor_rpm=570.0f;
    float reverse_limit_kph=20.0f; // requisito de Armando, 08-10-2026
    float reverse_resume_kph=19.0f;
    float ramp_rpm_per_s=300.0f;
    float steer_center=2047.0f, steer_left=1089.0f, steer_right=3005.0f;
    float steer_max_deg=20.0f, steer_deadband=200.0f;
    float yaw_sign=1.0f; // positivo = giro a la derecha; verificar montaje del MPU
    float esc_gain_per_dps=1.0f/200.0f;
    float esc_max_correction=0.10f; // límite de desarrollo, no ganancia validada
    float left_gain=1.0f, right_gain=1.0f; // sin calibración individual inventada
    uint32_t command_timeout_ms=100, standstill_ms=500, hall_grace_ms=1500;
    uint16_t pedal_release_raw=40;
};
struct Command {
    uint16_t accelerator=0, steer=2047;
    bool brake=false, start=false;
    Gear gear=Gear::Neutral;
    Mode mode=Mode::Dif;
};
bool decodeCommand(const uint8_t* bytes, uint8_t length, Command& result);
struct Sensors {
    float rpm_left=0, rpm_right=0, yaw_dps=0;
    bool speed_valid=false, pulse_left_recent=false, pulse_right_recent=false;
    bool yaw_valid=false, charger=false, outputs_ok=true, can_ok=true;
    bool frontend_ok=true; // wrapper exige frescura si se recibe extensión 0x101
};
struct Output {
    float effort_left=0, effort_right=0; // 0..1; 0 = sin aceleración
    Gear direction=Gear::Neutral;
    Mode selected_mode=Mode::Dif;
    uint8_t applied_mode=255; // 255 = ningún modo aplicando tracción
    Relay relay=Relay::Off;
    Inhibit inhibit=Inhibit::NoCommand;
    bool rearm_required=true, reverse_limited=false, stationary=false;
    float speed_kph=0;
};
class Controller {
public:
    explicit Controller(const Config& config=Config()) : cfg_(config) {}
    bool receive(const uint8_t* bytes, uint8_t length, uint32_t now);
    Output step(const Sensors& sensors, uint32_t now);
    const Command& command() const { return command_; }
    bool hasCommand() const { return received_; }
    uint32_t age(uint32_t now) const { return now-last_command_; }
    uint32_t accepted() const { return accepted_; }
    uint32_t rejected() const { return rejected_; }
    const Config& config() const { return cfg_; }
private:
    Config cfg_;
    Command command_;
    bool received_=false, invalid_=false, rearm_=true, start_off_seen_=false;
    bool stop_pending_=false; // STOP recibido debe aplicarse aunque otra trama lo siga en el lote.
    bool start_previous_=false, was_still_=false, motion_expected_=false;
    bool reverse_cut_=false, clock_started_=false;
    uint32_t last_command_=0, last_step_=0, relay_since_=0, still_since_=0, drive_since_=0;
    uint32_t accepted_=0, rejected_=0;
    Relay relay_=Relay::Off;
    Gear gear_=Gear::Neutral;
    Mode mode_=Mode::Dif;
    float effort_left_=0, effort_right_=0;
    Output stop(Output output, Inhibit why, bool require_rearm=false);
};
float rpmToKph(float rpm, float diameter_m);
float steeringDegrees(uint16_t raw, const Config& cfg);
}
