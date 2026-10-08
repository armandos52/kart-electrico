#include "kart_control.h"
#include <math.h>
#include <algorithm>

namespace kart {
static constexpr float PI=3.14159265358979323846f;
static float limit(float value,float low,float high) {return std::max(low,std::min(value,high));}
const char* inhibitName(Inhibit value) {
    static const char* names[]={"none","no_command","can_timeout","invalid_command","start_off","relays",
        "charger","rearm_required","brake","pedal_zero","neutral","direction_change","mode_change",
        "speed_invalid","hall_fault","mpu_unavailable","frontend_invalid","reverse_limit",
        "output_fault","can_fault","configuration"};
    const unsigned index=static_cast<unsigned>(value);
    return index<sizeof(names)/sizeof(names[0])?names[index]:"unknown";
}
bool decodeCommand(const uint8_t* b,uint8_t n,Command& result) {
    if(!b || n!=8) return false;
    const uint16_t accelerator=uint16_t((uint16_t(b[0])<<8)|b[1]);
    const uint16_t steer=uint16_t((uint16_t(b[2])<<8)|b[3]);
    if(accelerator>4095 || steer>4095 || b[4]>1 || b[5]>1 || b[6]>1 ||
       (b[5]&&b[6]) || (b[7]&0xf8) || (b[7]&3)>2) return false;
    Command next;
    next.accelerator=accelerator;next.steer=steer;next.brake=b[4]==1;
    next.gear=b[5]?Gear::Drive:(b[6]?Gear::Reverse:Gear::Neutral);
    next.mode=static_cast<Mode>(b[7]&3);next.start=(b[7]&4)!=0;
    result=next;return true;
}
float rpmToKph(float rpm,float diameter) {return rpm*PI*diameter*0.06f;}
float steeringDegrees(uint16_t raw,const Config& cfg) {
    const float offset=float(raw)-cfg.steer_center;
    if(fabsf(offset)<=cfg.steer_deadband) return 0;
    const float span=offset<0?cfg.steer_center-cfg.steer_left:cfg.steer_right-cfg.steer_center;
    // Zona muerta continua; topes configurables sin inferir ángulos del PC.
    const float fraction=limit((fabsf(offset)-cfg.steer_deadband)/(span-cfg.steer_deadband),0,1);
    return copysignf(cfg.steer_max_deg*fraction,offset);
}
bool Controller::receive(const uint8_t* b,uint8_t n,uint32_t now) {
    Command next;
    if(!decodeCommand(b,n,next)) {invalid_=true;rearm_=true;++rejected_;return false;}
    command_=next;last_command_=now;received_=true;invalid_=false;++accepted_;return true;
}
Output Controller::stop(Output out,Inhibit why,bool require_rearm) {
    if(require_rearm) rearm_=true;
    effort_left_=effort_right_=0;motion_expected_=false;
    out.inhibit=why;out.relay=relay_;out.selected_mode=mode_;
    out.rearm_required=rearm_;out.direction=Gear::Neutral;
    out.effort_left=out.effort_right=0;out.applied_mode=255;
    return out;
}
Output Controller::step(const Sensors& s,uint32_t now) {
    Output out;
    float dt=clock_started_?limit(float(now-last_step_)*0.001f,0,0.05f):0;
    last_step_=now;clock_started_=true;
    const bool speed_ok=s.speed_valid && isfinite(s.rpm_left) && isfinite(s.rpm_right) && s.rpm_left>=0 && s.rpm_right>=0;
    out.speed_kph=speed_ok?rpmToKph(std::max(s.rpm_left,s.rpm_right),cfg_.wheel_diameter_m):0;
    const bool still=speed_ok && s.rpm_left<1.0f && s.rpm_right<1.0f;
    if(!still) was_still_=false;
    else if(!was_still_) {was_still_=true;still_since_=now;}
    out.stationary=still && now-still_since_>=cfg_.standstill_ms;
    const bool released=command_.accelerator<=cfg_.pedal_release_raw;
    const bool fresh=received_ && now-last_command_<=cfg_.command_timeout_ms;
    // El estado detenido siempre anula inmediatamente las consignas.
    if(s.charger) {relay_=Relay::Off;start_off_seen_=false;start_previous_=false;return stop(out,Inhibit::Charger,true);}
    if(!s.outputs_ok) {relay_=Relay::Off;return stop(out,Inhibit::OutputFault,true);}
    if(!fresh || invalid_ || !s.can_ok || !s.frontend_ok) {
        // No continuar una precarga sin mandos; conservar contactor ya cerrado evita ciclarlo bajo carga.
        if(relay_!=Relay::Ready) relay_=Relay::Off;
        start_off_seen_=false;start_previous_=false;
        return stop(out,invalid_?Inhibit::InvalidCommand:(!s.can_ok?Inhibit::CanFault:
            (!s.frontend_ok?Inhibit::FrontendInvalid:(received_?Inhibit::CanTimeout:Inhibit::NoCommand))),true);
    }
    if(!command_.start) {
        relay_=Relay::Off;start_off_seen_=true;start_previous_=false;gear_=Gear::Neutral;
        return stop(out,Inhibit::StartOff,true);
    }
    const bool start_edge=!start_previous_;
    start_previous_=true;
    // Un nuevo START requiere observar OFF, N, freno, pedal libre y quietud.
    if(relay_==Relay::Off && start_edge && start_off_seen_ && released && command_.brake &&
       command_.gear==Gear::Neutral && out.stationary) {
        relay_=Relay::Precharge;relay_since_=now;start_off_seen_=false;
    }
    if(relay_==Relay::Precharge && now-relay_since_>=2000) {relay_=Relay::Overlap;relay_since_=now;}
    if(relay_==Relay::Overlap && now-relay_since_>=1000) relay_=Relay::Ready;
    out.relay=relay_;
    if(relay_!=Relay::Ready) return stop(out,Inhibit::Relays,true);
    if(!speed_ok) return stop(out,Inhibit::SpeedInvalid,true);
    const bool config_ok=isfinite(cfg_.wheelbase_m) && cfg_.wheelbase_m>0 && isfinite(cfg_.track_m) && cfg_.track_m>0 &&
        isfinite(cfg_.wheel_diameter_m) && cfg_.wheel_diameter_m>0 && isfinite(cfg_.reference_motor_rpm) && cfg_.reference_motor_rpm>0 &&
        isfinite(cfg_.ramp_rpm_per_s) && cfg_.ramp_rpm_per_s>0 && cfg_.steer_center-cfg_.steer_left>cfg_.steer_deadband &&
        cfg_.steer_right-cfg_.steer_center>cfg_.steer_deadband && cfg_.steer_deadband>=0 && cfg_.steer_max_deg>0 && cfg_.steer_max_deg<45 &&
        isfinite(cfg_.reverse_limit_kph) && isfinite(cfg_.reverse_resume_kph) && cfg_.reverse_limit_kph>0 &&
        cfg_.reverse_resume_kph<cfg_.reverse_limit_kph && cfg_.reverse_resume_kph>=0 &&
        isfinite(cfg_.yaw_sign) && fabsf(cfg_.yaw_sign)==1 && isfinite(cfg_.esc_gain_per_dps) && cfg_.esc_gain_per_dps>=0 &&
        isfinite(cfg_.esc_max_correction) && cfg_.esc_max_correction>=0 && cfg_.esc_max_correction<=0.20f &&
        cfg_.left_gain>0 && cfg_.right_gain>0 && isfinite(cfg_.left_gain) && isfinite(cfg_.right_gain);
    if(!config_ok) return stop(out,Inhibit::Configuration,true);
    if(rearm_ && released && command_.gear==Gear::Neutral && command_.brake && out.stationary) rearm_=false;
    if(command_.mode!=mode_) {
        if(command_.gear==Gear::Neutral && released && out.stationary) mode_=command_.mode;
        else return stop(out,Inhibit::ModeChange,true);
    }
    if(command_.gear==Gear::Neutral) gear_=Gear::Neutral;
    else if(command_.gear!=gear_) {
        if(out.stationary && released && command_.brake && !rearm_) gear_=command_.gear;
        else return stop(out,Inhibit::DirectionChange,true);
    }
    if(command_.brake) return stop(out,Inhibit::Brake,!released);
    if(rearm_) return stop(out,Inhibit::Rearm);
    if(gear_==Gear::Neutral) return stop(out,Inhibit::Neutral);
    if(released) return stop(out,Inhibit::PedalZero);
    if(mode_==Mode::Esc && (!s.yaw_valid || !isfinite(s.yaw_dps))) return stop(out,Inhibit::MpuUnavailable,true);
    // Sin pulsos con aceleración sostenida se impide seguir confiando en velocidad cero.
    if(!motion_expected_) {motion_expected_=true;drive_since_=now;}
    if(now-drive_since_>cfg_.hall_grace_ms && (!s.pulse_left_recent || !s.pulse_right_recent))
        return stop(out,Inhibit::HallFault,true);
    float base=float(command_.accelerator)/4095.0f;
    float left=base,right=base;
    const float angle=steeringDegrees(command_.steer,cfg_)*PI/180.0f;
    if(mode_!=Mode::Equal) {
        const float split=(cfg_.track_m*0.5f/cfg_.wheelbase_m)*tanf(angle);
        left=base*(1+split);right=base*(1-split);
        if(mode_==Mode::Esc) {
            const float sign=gear_==Gear::Reverse?-1.0f:1.0f;
            const float speed_ms=rpmToKph((s.rpm_left+s.rpm_right)*0.5f,cfg_.wheel_diameter_m)/3.6f;
            const float ideal=sign*speed_ms*tanf(angle)/cfg_.wheelbase_m*180.0f/PI;
            const float error=ideal-cfg_.yaw_sign*s.yaw_dps;
            const float correction=fabsf(error)>2.0f?limit(sign*error*cfg_.esc_gain_per_dps,
                -cfg_.esc_max_correction,cfg_.esc_max_correction)*base:0;
            left+=correction;right-=correction;
        }
    }
    left=std::max(0.0f,left*cfg_.left_gain);right=std::max(0.0f,right*cfg_.right_gain);
    const float maximum=std::max(1.0f,std::max(left,right));left/=maximum;right/=maximum;
    if(gear_==Gear::Reverse) {
        if(out.speed_kph>=cfg_.reverse_limit_kph) reverse_cut_=true;
        else if(out.speed_kph<=cfg_.reverse_resume_kph) reverse_cut_=false;
        const float cap=limit(cfg_.reverse_limit_kph/rpmToKph(cfg_.reference_motor_rpm,cfg_.wheel_diameter_m),0,1);
        const float peak=std::max(left,right);
        if(peak>cap) {left*=cap/peak;right*=cap/peak;out.reverse_limited=true;}
        const float governor=reverse_cut_?0:limit((cfg_.reverse_limit_kph-out.speed_kph)/2.0f,0,1);
        left*=governor;right*=governor;out.reverse_limited|=governor<1;
    } else reverse_cut_=false;
    const float delta=(cfg_.ramp_rpm_per_s/cfg_.reference_motor_rpm)*dt;
    // Aumentos limitados en ambas salidas, reducciones y parada inmediatas.
    effort_left_=std::min(left,effort_left_+delta);effort_right_=std::min(right,effort_right_+delta);
    out.effort_left=effort_left_;out.effort_right=effort_right_;
    out.direction=(effort_left_>0 || effort_right_>0)?gear_:Gear::Neutral;
    out.selected_mode=mode_;out.applied_mode=out.direction==Gear::Neutral?255:uint8_t(mode_);
    out.rearm_required=false;out.inhibit=reverse_cut_?Inhibit::ReverseLimit:Inhibit::None;
    return out;
}
}
