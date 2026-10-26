#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <stdarg.h>
#include "bench_logic.h"

// ENS-0015: hardware real de pantalla/CPU/B1; CAN, ADC y salidas simulados.
MCUFRIEND_kbv lcd;
uint16_t display_id;
static constexpr uint16_t BG=0x0843, WHITE=0xEF7D, GREEN=0x3E6F, YELLOW=0xFE69;
static const char* relayName(uint8_t relay) {
    return relay==1?"PRECARGA":relay==2?"SOLAPE":relay==3?"LISTOS":"OFF";
}
int benchPrintf(const char* format, ...) {
    char line[256];
    va_list args;va_start(args,format);
    const int n=vsnprintf(line,sizeof(line),format,args);va_end(args);
    Serial.print(line);return n;
}
static void textAt(int x,int y,uint8_t size,uint16_t color,const char* text) {
    lcd.setTextSize(size);lcd.setTextColor(color,BG);lcd.setCursor(x,y);lcd.print(text);
}
static void drawFrame() {
    lcd.fillScreen(BG);
    textAt(12,10,2,WHITE,"KART | BANCO F411");
    lcd.fillRect(0,36,lcd.width(),18,YELLOW);
    lcd.setTextSize(1);lcd.setTextColor(BG,YELLOW);lcd.setCursor(12,41);
    lcd.print("DATOS SIMULADOS | SIN MOTORES");
    textAt(12,206,1,WHITE,"B1 azul: pulsar y soltar = START");
    textAt(12,220,1,WHITE,"Segunda pulsacion = STOP");
    // Verifica transporte de pixeles, no demuestra visualmente el panel.
    const uint16_t colors[]={0xF800,0x07E0,0x001F};
    for(int i=0;i<3;i++) {
        const int x=lcd.width()-18+i*6;
        lcd.fillRect(x,8,5,12,colors[i]);
        const uint16_t actual=lcd.readPixel(x+2,10);
        benchPrintf("LCD_PIXEL expected=%04X actual=%04X pass=%u\n",colors[i],actual,actual==colors[i]);
    }
}
static void showState(const BenchState& s) {
    char line[64];
    lcd.fillRect(10,62,lcd.width()-20,134,BG);
    snprintf(line,sizeof(line),"PRUEBAS: %d  FALLOS: %d",s.checks,s.failures);
    textAt(12,64,1,s.failures?0xF800:GREEN,line);
    textAt(12,80,1,WHITE,"Logica frontal + trasera v1.3-dev3");
    snprintf(line,sizeof(line),"START: %s",s.start?"ON":"OFF");
    textAt(12,104,3,s.start?GREEN:WHITE,line);
    snprintf(line,sizeof(line),"B1 fisico: %s",s.raw_pressed?"PULSADO":"LIBRE");
    textAt(12,140,1,WHITE,line);
    snprintf(line,sizeof(line),"RELES: %s",relayName(s.relay));
    textAt(12,164,2,YELLOW,line);
    textAt(12,188,1,WHITE,"Reles virtuales. N + freno simulados.");
}
void setup() {
    Serial.begin(115200);
    pinMode(USER_BTN,INPUT_PULLUP);
    delay(3000); // permite abrir la captura tras verificar/grabar.
    lcd.reset();
    display_id = lcd.readID();
    benchPrintf("BENCH_F411_v0.2 ID=%04X cpu_hz=%lu HAL_CAN_ADC_SD=SIMULATED\n",display_id,(unsigned long)SystemCoreClock);
    if(display_id!=0x9341) {
        while(true) {Serial.println("ERROR: se esperaba ILI9341; pantalla no inicializada");delay(2000);}
    }
    lcd.begin(display_id);
    lcd.setRotation(1);
    benchPrintf("LCD_CONFIG width=%d height=%d rotation=1\n",lcd.width(),lcd.height());
    drawFrame();
    benchRunTests();
    benchBegin();
    Serial.println("READY: B1 fisico; bus CAN y reles SIMULADOS. No tactil configurado.");
}
void loop() {
    static uint32_t lastTick=0,lastLog=0;
    static uint8_t oldRaw=255,oldStart=255,oldRelay=255;
    const uint32_t now=millis();
    if(now-lastTick<5)return;
    lastTick=now;
    const BenchState s=benchTick(now,digitalRead(USER_BTN)==LOW);
    const bool changed=s.raw_pressed!=oldRaw||s.start!=oldStart||s.relay!=oldRelay;
    if(changed || now-lastLog>=1000) {
        benchPrintf("STATE ms=%lu raw=%u start=%u relay=%u inhibit=%u checks=%d failures=%d\n",
            (unsigned long)now,s.raw_pressed,s.start,s.relay,s.inhibit,s.checks,s.failures);
        if(changed)showState(s);
        oldRaw=s.raw_pressed;oldStart=s.start;oldRelay=s.relay;lastLog=now;
    }
}
