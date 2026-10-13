#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>
using std::abs;
using byte = uint8_t;
constexpr double PI = 3.14159265358979323846;
constexpr int LOW=0, HIGH=1, OUTPUT=1, INPUT=0, FALLING=2;
constexpr int CAN_OK=0, CAN_MSGAVAIL=1, MCP_ANY=0, CAN_500KBPS=0, MCP_8MHZ=0, MCP_NORMAL=0;
#define IRAM_ATTR
unsigned long fake_ms = 1000;
int pins[64]{};
int duties[64]{};
struct Write { int pin; int value; };
std::vector<Write> pwm_writes;
unsigned long millis(){return fake_ms;}
void delay(unsigned long ms){fake_ms+=ms;}
long map(long x,long a,long b,long c,long d){return (x-a)*(d-c)/(b-a)+c;}
template<class T> T constrain(T x,T a,T b){return std::max(a,std::min(x,b));}
void pinMode(int,int){}
void digitalWrite(int pin,int value){pins[pin]=value;}
int digitalRead(int pin){return pins[pin];}
bool ledcWrite(int pin,int value){duties[pin]=value;pwm_writes.push_back({pin,value});return true;}
bool ledcAttach(int,int,int){return true;}
int digitalPinToInterrupt(int pin){return pin;}
void attachInterrupt(int,void(*)(),int){}
void noInterrupts(){}
void interrupts(){}
struct SerialStub {
    void setTxBufferSize(int){}
    void begin(int){}
    void println(const char*){}
} Serial;
struct SPIStub {void begin(int,int,int,int){}} SPI;
struct WireStub {void begin(int,int){}} Wire;
struct MPU6050 {
    void initialize(){}
    bool testConnection(){return true;}
    void getRotation(int16_t* x,int16_t* y,int16_t* z){*x=*y=*z=0;}
};
struct MCP_CAN {
    bool pending=false;
    unsigned long next_id=0x100;
    byte next_len=8, next_result=CAN_OK;
    byte next_data[8]{};
    explicit MCP_CAN(int){}
    int begin(int,int,int){return CAN_OK;}
    int setMode(int){return CAN_OK;}
    int checkReceive(){return pending?CAN_MSGAVAIL:0;}
    byte readMsgBuf(unsigned long* id,byte* len,byte* data){
        *id=next_id;*len=next_len;std::memcpy(data,next_data,8);pending=false;return next_result;
    }
    int sendMsgBuf(int,int,int,byte*){return CAN_OK;}
};
struct MuestraKart {
    unsigned long tick_ms{},can_rx_age_ms{},can_rx_count{},can_init_attempts{};
    bool mandos_validos{},relay1{},relay2{},carregador{},emergency_stop{},mpu_ok{},control_ready{},can_init_ok{},start_requested{};
    uint16_t acelerador_raw{},direcao_raw{};
    uint8_t travao{},modo_d{},modo_r{},modo_controlo{},estado_relays{};
    int motor_esq_speed{},motor_dir_speed{},rpm_esq{},rpm_dir{};
    float yaw_rate{};
    const char *control_state{},*drive_inhibit{},*drive_direction{};
};
void telemetria_iniciar(){}
void telemetria_control_iniciado(){}
bool telemetria_toca(unsigned long){return false;}
void telemetria_publicar(const MuestraKart&){}
