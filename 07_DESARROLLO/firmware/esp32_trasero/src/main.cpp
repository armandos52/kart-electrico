#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <mcp_can.h>
#include <MPU6050.h>
#include <esp_task_wdt.h>
#include "kart_control.h"
#include "hardware_kart.h"
#include "telemetria.h"
#include "../../common/kart_protocol.h"
#include <algorithm>
#include <math.h>

namespace {
kart::Controller controller;
kart::Sensors sensors;
kart::Output output;
MCP_CAN can(hardware::CAN_CS);
MPU6050 mpu;
portMUX_TYPE hall_mux=portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t pulses_l=0,pulses_r=0,last_pulse_l=0,last_pulse_r=0;
volatile bool seen_l=false,seen_r=false;
uint32_t hall_tick=0,gyro_tick=0,yaw_tick=0,can_attempt_tick=0,can_attempts=0;
uint32_t tx_tick=0,status_tick=0,can_tx_errors=0,can_rx_errors=0,mpu_errors=0;
bool can_ready=false,outputs_ready=false,watchdog_ok=false,gyro_present=false,gyro_calibrated=false;
uint16_t gyro_samples=0;
double gyro_sum=0,gyro_sum_square=0;
float gyro_offset=0;
uint32_t calibration_start=0;
bool health_seen=false,health_valid=false;
uint16_t health_seq=0,adc_accel=0,adc_steer=0;
uint32_t health_tick=0;
uint8_t can_errors=0;
int pwm_left=255,pwm_right=255;

void IRAM_ATTR hallLeft() {
    const uint32_t now=micros();
    portENTER_CRITICAL_ISR(&hall_mux);
    if(!seen_l || now-last_pulse_l>=100) {++pulses_l;last_pulse_l=now;seen_l=true;}
    portEXIT_CRITICAL_ISR(&hall_mux);
}
void IRAM_ATTR hallRight() {
    const uint32_t now=micros();
    portENTER_CRITICAL_ISR(&hall_mux);
    if(!seen_r || now-last_pulse_r>=100) {++pulses_r;last_pulse_r=now;seen_r=true;}
    portEXIT_CRITICAL_ISR(&hall_mux);
}
void updateHall(uint32_t now) {
    const uint32_t dt=now-hall_tick;
    if(dt>=hardware::HALL_WINDOW_MS) {
        portENTER_CRITICAL(&hall_mux);
        const uint32_t left=pulses_l,right=pulses_r;
        pulses_l=pulses_r=0;
        portEXIT_CRITICAL(&hall_mux);
        sensors.rpm_left=float(left)*60000.0f/(float(hardware::PULSES_LEFT)*dt);
        sensors.rpm_right=float(right)*60000.0f/(float(hardware::PULSES_RIGHT)*dt);
        sensors.speed_valid=dt<=hardware::HALL_MAX_AGE_MS && sensors.rpm_left<=2000 && sensors.rpm_right<=2000;
        hall_tick=now;
    }
    portENTER_CRITICAL(&hall_mux);
    const uint32_t last_l=last_pulse_l,last_r=last_pulse_r;
    const bool have_l=seen_l,have_r=seen_r;
    portEXIT_CRITICAL(&hall_mux);
    const uint32_t us=micros();
    sensors.pulse_left_recent=have_l && us-last_l<=300000;
    sensors.pulse_right_recent=have_r && us-last_r<=300000;
    if(now-hall_tick>hardware::HALL_MAX_AGE_MS) sensors.speed_valid=false;
}
bool readGyroZ(int16_t& result) {
    uint8_t bytes[2]={0,0};
    if(I2Cdev::readBytes(MPU6050_DEFAULT_ADDRESS,MPU6050_RA_GYRO_ZOUT_H,2,bytes,5,&Wire)!=2) return false;
    result=int16_t((uint16_t(bytes[0])<<8)|bytes[1]);return true;
}
void updateGyro(uint32_t now) {
    if(!gyro_present) {sensors.yaw_valid=false;return;}
    if(!gyro_calibrated && now-calibration_start>5000) {gyro_present=false;sensors.yaw_valid=false;++mpu_errors;return;}
    const uint32_t period=gyro_calibrated?hardware::GYRO_PERIOD_MS:2;
    if(now-gyro_tick<period) return;
    gyro_tick=now;
    int16_t raw=0;
    if(!readGyroZ(raw)) {++mpu_errors;sensors.yaw_valid=false;return;}
    if(!gyro_calibrated) {
        // Calibración con tracción inhibida; una ventana movida se rechaza, no se inventa offset.
        if(now-calibration_start>5000 || sensors.rpm_left>1 || sensors.rpm_right>1) {
            gyro_present=false;++mpu_errors;return;
        }
        gyro_sum+=raw;gyro_sum_square+=double(raw)*raw;++gyro_samples;
        if(gyro_samples==500) {
            const double mean=gyro_sum/500.0;
            const double variance=std::max(0.0,gyro_sum_square/500.0-mean*mean);
            if(sqrt(variance)/131.0>1.0 || fabs(mean)/131.0>10.0) {gyro_present=false;++mpu_errors;return;}
            gyro_offset=float(mean);gyro_calibrated=true;
        }
        return;
    }
    sensors.yaw_dps=(float(raw)-gyro_offset)/131.0f;
    sensors.yaw_valid=abs(int(raw))<32700 && isfinite(sensors.yaw_dps);
    if(sensors.yaw_valid) yaw_tick=now;else ++mpu_errors;
}
bool initOutputs() {
    // Estado de reposo antes de cualquier espera de CAN/I2C/Wi-Fi.
    for(uint8_t pin:{hardware::RELAY1,hardware::RELAY2,hardware::FORWARD_L,hardware::REVERSE_L,
            hardware::FORWARD_R,hardware::REVERSE_R,hardware::BRAKE_L,hardware::BRAKE_R}) {
        digitalWrite(pin,LOW);pinMode(pin,OUTPUT);
    }
    digitalWrite(hardware::PWM_L,HIGH);digitalWrite(hardware::PWM_R,HIGH);
    pinMode(hardware::PWM_L,OUTPUT);pinMode(hardware::PWM_R,OUTPUT);
    pinMode(hardware::CHARGER,INPUT); // GPIO35 no tiene pull interno: verificar circuito real
    bool ok=true;
    for(uint8_t pin:{hardware::PWM_L,hardware::PWM_R,hardware::BRAKE_PWM_L,hardware::BRAKE_PWM_R})
        ok=ledcAttach(pin,hardware::PWM_HZ,8)&&ok;
    ok=ledcWrite(hardware::PWM_L,255)&&ok;ok=ledcWrite(hardware::PWM_R,255)&&ok;
    ok=ledcWrite(hardware::BRAKE_PWM_L,0)&&ok;ok=ledcWrite(hardware::BRAKE_PWM_R,0)&&ok;
    return ok;
}
void applyOutputs(const kart::Output& value) {
    const bool driving=value.direction!=kart::Gear::Neutral && (value.effort_left>0 || value.effort_right>0);
    // Al parar, PWM antes de conmutar dirección/contactores. Nunca se escribe una consigna no autorizada.
    if(!driving) {
        const bool left_ok=ledcWrite(hardware::PWM_L,255);
        const bool right_ok=ledcWrite(hardware::PWM_R,255);
        if(!left_ok || !right_ok)outputs_ready=false;
    }
    const bool forward=driving && value.direction==kart::Gear::Drive;
    const bool reverse=driving && value.direction==kart::Gear::Reverse;
    digitalWrite(hardware::FORWARD_R,forward);digitalWrite(hardware::REVERSE_R,reverse);
    digitalWrite(hardware::FORWARD_L,hardware::LEFT_INVERTED?reverse:forward);
    digitalWrite(hardware::REVERSE_L,hardware::LEFT_INVERTED?forward:reverse);
    digitalWrite(hardware::RELAY1,value.relay==kart::Relay::Precharge || value.relay==kart::Relay::Overlap);
    digitalWrite(hardware::RELAY2,value.relay==kart::Relay::Overlap || value.relay==kart::Relay::Ready);
    // Ceil nunca supera la consigna de aceleración por redondeo (PWM invertido).
    pwm_left=driving?int(ceilf(255*(1-value.effort_left))):255;
    pwm_right=driving?int(ceilf(255*(1-value.effort_right))):255;
    if(driving) {
        const bool left_ok=ledcWrite(hardware::PWM_L,pwm_left);
        const bool right_ok=ledcWrite(hardware::PWM_R,pwm_right);
        if(!left_ok || !right_ok) {
            outputs_ready=false;ledcWrite(hardware::PWM_L,255);ledcWrite(hardware::PWM_R,255);
            digitalWrite(hardware::FORWARD_L,LOW);digitalWrite(hardware::REVERSE_L,LOW);
            digitalWrite(hardware::FORWARD_R,LOW);digitalWrite(hardware::REVERSE_R,LOW);
            digitalWrite(hardware::RELAY1,LOW);digitalWrite(hardware::RELAY2,LOW);
            pwm_left=pwm_right=255;
        }
    }
}
void receiveCan() {
    const uint32_t start_us=micros();
    // Drenar con límites evita atrasar control bajo carga de bus.
    for(unsigned n=0;n<8 && micros()-start_us<2000 && can.checkReceive()==CAN_MSGAVAIL;++n) {
        unsigned long id=0;uint8_t len=0,data[8]={0};
        if(can.readMsgBuf(&id,&len,data)!=CAN_OK) {++can_rx_errors;controller.receive(nullptr,0,millis());break;}
        if(id==KART_CAN_COMMAND) controller.receive(data,len,millis()); // id con flags RTR/EXT no coincide
        else if(id==KART_CAN_INPUT_HEALTH) {
            if(len!=8 || data[0]!=KART_PROTOCOL_VERSION || (data[1]&0xf8)) {health_seen=true;health_valid=false;continue;}
            const uint16_t seq=uint16_t((uint16_t(data[6])<<8)|data[7]);
            if(!health_seen || seq!=health_seq) {health_tick=millis();health_seq=seq;}
            health_seen=true;health_valid=(data[1]&7)==7;
            adc_accel=uint16_t((uint16_t(data[2])<<8)|data[3]);
            adc_steer=uint16_t((uint16_t(data[4])<<8)|data[5]);
            if(adc_accel>4095 || adc_steer>4095) health_valid=false;
        }
    }
}
void put16(uint8_t* b,int16_t value){b[0]=uint8_t(uint16_t(value)>>8);b[1]=uint8_t(value);}
void sendCan(unsigned long id,uint8_t* bytes) {
    if(can.sendMsgBuf(id,0,8,bytes)!=CAN_OK) ++can_tx_errors;
}
void transmitCan(uint32_t now) {
    if(now-tx_tick>=200) {
        tx_tick=now;uint8_t data[8]={0};
        const float mean=(sensors.rpm_left+sensors.rpm_right)*0.5f;
        put16(data,int16_t(kart::rpmToKph(mean,controller.config().wheel_diameter_m)));
        put16(data+2,int16_t(sensors.rpm_left));put16(data+4,int16_t(sensors.rpm_right));
        put16(data+6,sensors.yaw_valid?int16_t(sensors.yaw_dps*10):0);
        sendCan(KART_CAN_FEEDBACK,data); // compatible: validez está en 0x124
    }
    if(now-status_tick>=100) {
        status_tick=now;
        const uint8_t flags=(sensors.outputs_ok?KART_STATUS_READY:0)|(output.rearm_required?KART_STATUS_REARM:0)|(sensors.speed_valid?KART_STATUS_SPEED_VALID:0)|
            (sensors.yaw_valid?KART_STATUS_YAW_VALID:0)|(output.stationary?KART_STATUS_STATIONARY:0)|(output.reverse_limited?KART_STATUS_REVERSE_LIMIT:0)|(health_seen?KART_STATUS_FRONTEND_HEALTH:0);
        uint8_t data[8]={KART_PROTOCOL_VERSION,uint8_t(output.inhibit),uint8_t(output.selected_mode),output.applied_mode,
            flags,uint8_t(output.relay),uint8_t(output.direction),uint8_t(controller.command().mode)};
        sendCan(KART_CAN_STATUS,data);
    }
}
void publish(uint32_t now) {
    MuestraKart m{};const auto& cmd=controller.command();
    m.tick_ms=now;m.mandos_validos=controller.hasCommand();m.acelerador_raw=cmd.accelerator;m.direcao_raw=cmd.steer;
    m.travao=cmd.brake;m.modo_d=cmd.gear==kart::Gear::Drive;m.modo_r=cmd.gear==kart::Gear::Reverse;m.modo_controlo=uint8_t(cmd.mode);
    m.motor_esq_speed=pwm_left;m.motor_dir_speed=pwm_right;m.rpm_esq=int16_t(sensors.rpm_left);m.rpm_dir=int16_t(sensors.rpm_right);
    m.yaw_rate=sensors.yaw_dps;m.mpu_ok=sensors.yaw_valid;m.control_ready=sensors.outputs_ok;
    m.relay1=digitalRead(hardware::RELAY1);m.relay2=digitalRead(hardware::RELAY2);m.estado_relays=uint8_t(output.relay);
    m.carregador=sensors.charger;m.emergency_stop=output.inhibit==kart::Inhibit::CanTimeout;
    m.can_rx_age_ms=controller.age(now);m.can_rx_count=controller.accepted();m.can_init_ok=can_ready;m.can_init_attempts=can_attempts;
    m.start_requested=cmd.start;m.control_state=!can_ready?"can_wait":(!gyro_calibrated&&gyro_present?"mpu_calibration":"running");
    m.drive_inhibit=kart::inhibitName(output.inhibit);
    m.drive_direction=output.direction==kart::Gear::Drive?"D":(output.direction==kart::Gear::Reverse?"R":"N");
    m.mode_applied=output.applied_mode;m.rearm_required=output.rearm_required;m.reverse_limited=output.reverse_limited;
    m.speed_valid=sensors.speed_valid;m.speed_kph=output.speed_kph;m.reverse_limit_kph=controller.config().reverse_limit_kph;
    m.frontend_health_seen=health_seen;m.frontend_health_ok=sensors.frontend_ok;m.watchdog_ok=watchdog_ok;
    m.can_rx_invalid=controller.rejected()+can_rx_errors;m.can_tx_errors=can_tx_errors;m.can_error_flags=can_errors;m.mpu_errors=mpu_errors;
    m.accel_adc=adc_accel;m.steer_adc=adc_steer;
    telemetria_publicar(m);
}
}

void setup() {
    outputs_ready=initOutputs();
    Serial.setTxBufferSize(3072);Serial.begin(115200);telemetria_iniciar();
    SPI.begin(hardware::SCK,hardware::MISO,hardware::MOSI,hardware::CAN_CS);
    Wire.begin(hardware::SDA,hardware::SCL);Wire.setTimeOut(5);I2Cdev::readTimeout=5;
    mpu.initialize();gyro_present=mpu.testConnection();
    // Verificar escala y que no siga dormido antes de aceptar /131.
    uint8_t gyro_cfg=0,power=0;
    gyro_present=gyro_present && I2Cdev::readByte(MPU6050_DEFAULT_ADDRESS,MPU6050_RA_GYRO_CONFIG,&gyro_cfg,5,&Wire)==1 &&
        I2Cdev::readByte(MPU6050_DEFAULT_ADDRESS,MPU6050_RA_PWR_MGMT_1,&power,5,&Wire)==1 && (gyro_cfg&0x18)==0 && !(power&0x40);
    calibration_start=millis();hall_tick=millis();
    pinMode(hardware::HALL_L,INPUT);pinMode(hardware::HALL_R,INPUT);
    attachInterrupt(digitalPinToInterrupt(hardware::HALL_L),hallLeft,FALLING);
    attachInterrupt(digitalPinToInterrupt(hardware::HALL_R),hallRight,FALLING);
    esp_task_wdt_config_t cfg{2000,0,true};
    esp_err_t result=esp_task_wdt_init(&cfg);
    if(result==ESP_ERR_INVALID_STATE) result=esp_task_wdt_reconfigure(&cfg);
    if(result==ESP_OK) {
        result=esp_task_wdt_add(nullptr);
        watchdog_ok=result==ESP_OK || esp_task_wdt_status(nullptr)==ESP_OK;
    }
    // Sin supervisión configurada se inhiben salidas: no declarar protección inexistente.
    outputs_ready=outputs_ready && watchdog_ok;
    telemetria_control_iniciado();
}
void loop() {
    if(watchdog_ok) esp_task_wdt_reset();
    uint32_t now=millis();
    if(!can_ready && (can_attempts==0 || now-can_attempt_tick>=500)) {
        can_attempt_tick=now;++can_attempts;
        can_ready=can.begin(MCP_ANY,CAN_500KBPS,MCP_8MHZ)==CAN_OK;
        if(can_ready) can_ready=can.setMode(MCP_NORMAL)==CAN_OK;
    }
    if(can_ready) {
        receiveCan();can_errors=can.getError();
        if(can_errors&0xe0) {++can_rx_errors;can_ready=false;controller.receive(nullptr,0,millis());}
    }
    now=millis();updateHall(now);updateGyro(now);
    now=millis();
    if(now-yaw_tick>hardware::GYRO_MAX_AGE_MS) sensors.yaw_valid=false;
    sensors.charger=digitalRead(hardware::CHARGER)==HIGH;
    sensors.can_ok=can_ready;sensors.outputs_ok=outputs_ready;
    sensors.frontend_ok=!health_seen || (health_valid && now-health_tick<=100);
    // Calibración inmóvil: no permitir tracción durante la ventana inicial.
    if(gyro_present && !gyro_calibrated) sensors.outputs_ok=false;
    output=controller.step(sensors,now);applyOutputs(output);
    if(!outputs_ready && sensors.outputs_ok) {
        sensors.outputs_ok=false;output=controller.step(sensors,millis());applyOutputs(output);
    }
    if(can_ready) transmitCan(now);
    if(telemetria_toca(millis())) publish(millis());
    delay(1);
}
