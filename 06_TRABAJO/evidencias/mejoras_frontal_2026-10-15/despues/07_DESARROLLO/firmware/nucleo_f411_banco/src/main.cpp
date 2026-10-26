#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <stdarg.h>
#include "bench_logic.h"
#include "visual_scenes.h"
#include "navigation.h"

// ENS-0016: LCD, CPU, UART y B1 reales; datos del kart sinteticos.
MCUFRIEND_kbv lcd;
static BenchNavigation navigation;
static unsigned page=0;
static BenchScene scene;
static bool healthy=false;
static constexpr uint16_t BG=0x0000,PANEL=0x10C3,WHITE=0xFFFF,MUTED=0x9D15;
static constexpr uint16_t CYAN=0x4E9F,GREEN=0x57B0,YELLOW=0xFEA8,RED=0xFB2C;
int benchPrintf(const char* format, ...) {
    char line[360];va_list args;va_start(args,format);
    const int n=vsnprintf(line,sizeof(line),format,args);va_end(args);
    Serial.print(line);return n;
}
static void textAt(int x,int y,uint8_t size,uint16_t color,const char* text) {
    lcd.setTextSize(size);lcd.setTextColor(color);lcd.setCursor(x,y);lcd.print(text);
}
static void centered(int x,int width,int y,uint8_t size,uint16_t color,const char* text) {
    textAt(x+(width-int(strlen(text))*6*size)/2,y,size,color,text);
}
static void report() {
    const auto& s=scene.snapshot;
    benchPrintf("VIEW v=" BENCH_VERSION " id=%u name=%s tick_sim=%lu speed=%d speed_valid=%u gear=%u start=%u brake=%u requested=%u received=%u applied=%u inhibit=%u feedback_fresh=%u status_fresh=%u sd=%u sd_error=%u sd_rows=%lu bms_valid=%u\n",
        page,scene.title,(unsigned long)s.tick,s.speed_kph,benchSpeedVisible(s),s.gear,s.start,s.brake,
        s.mode_requested,s.mode_received,s.mode_applied,s.inhibit,s.feedback_fresh,s.status_fresh,
        s.sd_state,s.sd_error,(unsigned long)s.sd_rows,s.soc_valid && s.temp_valid);
    for(unsigned i=0;i<5;i++)benchPrintf("LINE%u %s\n",i,scene.lines[i]);
}
static void show(unsigned index,const char* source) {
    page=index%BENCH_SCENE_COUNT;scene=benchScene(page);
    const auto& s=scene.snapshot;
    const uint16_t accent=page==7 || page==8?RED:page==1 || page==5 || page==9?YELLOW:CYAN;
    char value[32];lcd.fillScreen(BG);lcd.setTextWrap(false);
    lcd.fillRect(0,0,320,26,PANEL);
    textAt(10,6,2,WHITE,"KART / FRONTAL");
    snprintf(value,sizeof(value),"%02u/10",page+1);textAt(277,9,1,CYAN,value);
    lcd.fillRect(0,27,320,16,YELLOW);
    textAt(13,31,1,BG,"SIMULACION DE BANCO / DATOS SINTETICOS");
    textAt(10,52,2,accent,scene.title);
    if(s.brake){lcd.fillRoundRect(265,49,47,18,3,RED);textAt(273,54,1,BG,"FRENO");}
    lcd.fillRoundRect(8,76,62,71,5,PANEL);
    lcd.fillRoundRect(76,76,156,71,5,PANEL);
    lcd.fillRoundRect(238,76,74,71,5,PANEL);
    centered(8,62,81,1,MUTED,"MARCHA");centered(76,156,81,1,MUTED,"VELOCIDAD KM/H");
    centered(238,74,81,1,MUTED,"MODO SOL.");
    value[0]=s.gear==1?'D':s.gear==2?'R':'N';value[1]=0;centered(8,62,101,4,WHITE,value);
    if(benchSpeedVisible(s))snprintf(value,sizeof(value),"%d",s.speed_kph);
    else snprintf(value,sizeof(value),"--");
    centered(76,156,97,5,WHITE,value);
    centered(238,74,104,3,CYAN,s.mode_requested==0?"DIF":s.mode_requested==1?"ESC":"BLQ");
    lcd.fillRoundRect(8,153,304,18,3,accent);textAt(14,158,1,BG,scene.lines[1]);
    textAt(10,179,1,WHITE,scene.lines[0]);textAt(10,191,1,MUTED,scene.lines[2]);
    textAt(10,203,1,s.sd_state==KART_SD_ERROR?RED:WHITE,scene.lines[3]);
    textAt(10,215,1,MUTED,scene.lines[4]);lcd.drawFastHLine(8,226,304,PANEL);
    textAt(10,231,1,WHITE,"B1: SIGUIENTE");
    for(unsigned i=0;i<BENCH_SCENE_COUNT;i++)lcd.fillCircle(122+i*8,234,2,i==page?accent:PANEL);
    textAt(242,231,1,s.start?GREEN:MUTED,s.start?"START ON":"START OFF");
    benchPrintf("NAV source=%s id=%u uptime_ms=%lu\n",source,page,(unsigned long)millis());report();
}
static uint32_t crcByte(uint32_t crc,uint8_t b) {
    crc^=b;for(unsigned k=0;k<8;k++)crc=(crc>>1)^((crc&1)?0xEDB88320u:0);return crc;
}
static void capture() {
    // Solo 640 B RAM; la imagen se lee del LCD por filas, no se renderiza en PC.
    // La vista queda fija durante la transferencia y B1 no cambia de escena.
    uint16_t row[320];uint32_t crc=0xFFFFFFFFu;
    benchPrintf("CAPTURE_BEGIN v=" BENCH_VERSION " scene=%u w=320 h=240 bytes=153600 format=RGB565LE source=ILI9341_GRAM flow=ROWACK\n",page);
    for(int y=0;y<240;y++) {
        lcd.readGRAM(0,y,row,320,1);
        const uint8_t* bytes=reinterpret_cast<const uint8_t*>(row);
        for(unsigned x=0;x<sizeof(row);x++)crc=crcByte(crc,bytes[x]);
        Serial.write(bytes,sizeof(row));
        // Limita datos pendientes a una fila para evitar desbordar el VCP/host.
        Serial.flush();const uint32_t started=millis();
        while(!Serial.available() && millis()-started<10000)delay(1);
        if(!Serial.available() || Serial.read()!='A') {
            Serial.println("\nCAPTURE_ABORT missing_row_ack");navigation.reset(millis());return;
        }
    }
    benchPrintf("\nCAPTURE_END crc32=%08lX\n",(unsigned long)(crc^0xFFFFFFFFu));
    Serial.flush();navigation.reset(millis());
}
void setup() {
    Serial.begin(115200);pinMode(USER_BTN,INPUT_PULLUP);delay(3000);
    lcd.reset();const uint16_t id=lcd.readID();
    benchPrintf("BENCH_F411_v" BENCH_VERSION " ID=%04X cpu_hz=%lu HAL_CAN_ADC_SD=SIMULATED\n",id,(unsigned long)SystemCoreClock);
    if(id!=0x9341){Serial.println("ERROR LCD_ID");return;}
    lcd.begin(id);lcd.setRotation(1);lcd.fillScreen(BG);
    bool rgb=true;const uint16_t colors[]={0xF800,0x07E0,0x001F};
    for(unsigned i=0;i<3;i++){
        lcd.fillRect(i*10,0,10,10,colors[i]);const uint16_t read=lcd.readPixel(i*10+5,5);
        rgb&=read==colors[i];benchPrintf("LCD_PIXEL expected=%04X actual=%04X pass=%u\n",colors[i],read,read==colors[i]);
    }
    const int original=benchRunTests();const int visual=benchVisualTests();healthy=rgb && !original && !visual;
    if(!healthy){lcd.fillScreen(BG);textAt(10,80,2,RED,"ERROR: VER LOG SERIE");Serial.println("ERROR SELFTEST");return;}
    show(0,"BOOT");navigation.reset(millis());
    Serial.println("READY v" BENCH_VERSION ": B1 soltar=pagina; UART 0-9=pagina C=GRAM ?=estado N=siguiente. SIMULACION");
}
void loop() {
    if(!healthy)return;
    if(navigation.sample(digitalRead(USER_BTN)==LOW,millis()))show(page+1,"B1");
    if(Serial.available()) {
        const int c=Serial.read();
        if(c>='0' && c<='9')show(c-'0',"UART");
        else if(c=='N')show(page+1,"UART");
        else if(c=='?')report();
        else if(c=='C')capture();
    }
}
