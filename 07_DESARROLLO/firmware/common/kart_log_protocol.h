#ifndef KART_LOG_PROTOCOL_H
#define KART_LOG_PROTOCOL_H
#include <stdint.h>
#include <string.h>
#include "kart_protocol.h"
/* Diagnostico exclusivamente: nunca habilita movimiento. Un bloque coherente
 * son 16 fragmentos de 8 bytes, 0x125, version/indice/seq16/word32 BE.
 * Un emisor nuevo solo empieza por indice 0 (boot id); sin bloque completo
 * no se publica. Ausencia del canal es compatible con ESP32 v1.2. */
#define KART_CAN_LOG 0x125u
#define KART_LOG_WORDS 16u
#define KART_LOG_PROTOCOL 1u
#define KART_LOG_MAX_AGE 1000u
enum { KL_BOOT,KL_BUILD,KL_TICK,KL_OUTPUT,KL_STATE,KL_LOOP_US,KL_RX_COUNT,
       KL_TX_ERRORS,KL_RX_ERRORS,KL_MPU_ERRORS,KL_DROPPED,KL_TELEM_US,
       KL_INIT_TRIES,KL_DIAG,KL_SPEED_LIMIT,KL_COMMAND_AGE };
static inline uint32_t kart_u32be(const uint8_t* b){return ((uint32_t)kart_u16be(b)<<16)|kart_u16be(b+2);}
static inline void kart_put_u32be(uint8_t* b,uint32_t v){kart_put_u16be(b,(uint16_t)(v>>16));kart_put_u16be(b+2,(uint16_t)v);}
static inline uint32_t kart_build_id(const char* s){uint32_t h=2166136261u;while(*s){h^=(uint8_t)*s++;h*=16777619u;}return h;}
static inline void kart_log_packet(uint8_t* b,const uint32_t* words,uint16_t seq,uint8_t part){b[0]=1;b[1]=part;kart_put_u16be(b+2,seq);kart_put_u32be(b+4,words[part]);}
typedef struct {uint32_t words[16],published[16],started,received;uint16_t seq,mask;uint8_t seen,active;} KartLogReceiver;
static inline int kart_log_receive(KartLogReceiver* r,const uint8_t* b,uint32_t now){
    if(b[0]!=1 || b[1]>=16)return -1;
    const uint16_t seq=kart_u16be(b+2);const uint8_t part=b[1];const uint32_t value=kart_u32be(b+4);
    if(part==0){
        if(r->active && seq==r->seq && value==r->words[0])return 0;
        if(r->seen && value==r->published[0] && (seq==r->seq || (uint16_t)(seq-r->seq)>=32768u))return 0;
        r->seq=seq;r->mask=0;r->started=now;r->active=1;
    }
    if(!r->active || seq!=r->seq || now-r->started>500u)return 0;
    if(r->mask&(1u<<part)){if(r->words[part]!=value){r->active=0;return -1;}return 0;}
    r->words[part]=value;r->mask|=(uint16_t)(1u<<part);
    if(r->mask==65535u){memcpy(r->published,r->words,sizeof(r->words));r->received=now;r->seen=1;r->active=0;return 1;}
    return 0;
}
#endif
