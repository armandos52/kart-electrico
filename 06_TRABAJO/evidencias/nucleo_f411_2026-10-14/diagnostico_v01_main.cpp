#include <Arduino.h>
#include <MCUFRIEND_kbv.h>

// Primera etapa: identificar el controlador. Sin CAN, SD, motores ni reles.
MCUFRIEND_kbv lcd;
uint16_t display_id;
void setup() {
    Serial.begin(115200);
    delay(500);
    lcd.reset();
    display_id = lcd.readID();
}
void loop() {
    Serial.print("BENCH_F411_DIAG_v0.1 ID=0x");
    Serial.print(display_id, HEX);
    Serial.print(" REG00=0x"); Serial.print(lcd.readReg(0x00), HEX);
    Serial.print(" REG04=");
    for (int i=0;i<3;i++) { Serial.print(lcd.readReg(0x04,i),HEX); Serial.print(' '); }
    Serial.print(" REGD3=");
    for (int i=0;i<3;i++) { Serial.print(lcd.readReg(0xD3,i),HEX); Serial.print(' '); }
    Serial.println();
    delay(2000);
}
