#include "log_can.h"
#include "../../common/kart_log_protocol.h"
#include <esp_system.h>
namespace {
uint32_t words[16],boot=0,last_send=0,missed=0;
uint16_t sequence=0;uint8_t part=16;
uint8_t inhibit_code(const char* value){
    const char* names[]={"none","no_command","can_timeout","invalid_command","start_off","relays","charger","rearm_required","brake","pedal_zero","neutral","direction_change","mode_change","speed_invalid","hall_fault","mpu_unavailable","frontend_invalid","reverse_limit","output_fault","can_fault","configuration"};
    for(uint8_t i=0;i<21;i++)if(value && strcmp(value,names[i])==0)return i;return 255;
}
}
void log_can_snapshot(const MuestraKart& m,uint8_t profile){
    if(!boot)boot=esp_random()|1u;
    if(part<16){missed++;return;} // do not tear a partly transmitted block
    uint32_t loop,cost,dropped;uint8_t clients,ap;telemetria_log_stats(&loop,&cost,&dropped,&clients,&ap);
    words[0]=boot;words[1]=kart_build_id(FW_VERSION);words[2]=m.tick_ms;
    words[3]=(uint8_t)m.motor_esq_speed|((uint32_t)(uint8_t)m.motor_dir_speed<<8)|((uint32_t)m.relay1<<16)|((uint32_t)m.relay2<<17)|((uint32_t)m.carregador<<18)|((uint32_t)m.estado_relays<<24);
    const uint8_t gear=m.drive_direction && m.drive_direction[0]=='D'?1:m.drive_direction && m.drive_direction[0]=='R'?2:0;
    words[4]=inhibit_code(m.drive_inhibit)|((uint32_t)m.modo_controlo<<8)|((uint32_t)gear<<16)|((uint32_t)m.mpu_ok<<24)|((uint32_t)m.speed_valid<<25)|((uint32_t)m.control_ready<<26)|((uint32_t)m.can_init_ok<<27)|((uint32_t)profile<<28);
    words[5]=loop;words[6]=m.can_rx_count;words[7]=m.can_tx_errors;words[8]=m.can_rx_invalid;words[9]=m.mpu_errors;words[10]=dropped+missed;words[11]=cost;words[12]=m.can_init_attempts;
    words[13]=(uint8_t)esp_reset_reason()|((uint32_t)m.can_error_flags<<8)|((uint32_t)clients<<16)|((uint32_t)ap<<24);
    words[14]=(uint16_t)(m.speed_kph*100)|((uint32_t)(uint16_t)(m.reverse_limit_kph*100)<<16);words[15]=m.mandos_validos?m.can_rx_age_ms:UINT32_MAX;
    sequence++;part=0;
}
void log_can_service(MCP_CAN& can,uint32_t now,bool ready){
    if(!ready){if(part<16)missed++;part=16;return;}
    if(part>=16 || now-last_send<2)return;last_send=now;
    uint8_t bytes[8];kart_log_packet(bytes,words,sequence,part);
    // Just queue in a free MCP2515 TX buffer. Never wait for ACK or a free slot.
    if(can.trySendMsgBuf(KART_CAN_LOG,8,bytes)==CAN_OK)part++;
    else if(now-words[2]>150u){missed++;part=16;}
}
