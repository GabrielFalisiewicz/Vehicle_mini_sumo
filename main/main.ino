#include "PCF8574.h"
#include "Wire.h"

//Konfiguracja ekspandera
PCF8574 PCF_01(0x20);

//Konfiguracja PWM
#define PWM_A 2
#define PWM_B 3

//PWM Ferquency
#define FERQ 5000

void setup() {
    Serial.begin(115200);
    Wire.begin(6, 7);
    PCF_01.begin();
    analogWriteFrequency(FERQ);
    analogWriteResolution(8);
    pinMode(PWM_A, OUTPUT);
    pinMode(PWM_B, OUTPUT);
    PCF_01.write(0, LOW);
    PCF_01.write(1, HIGH);
    PCF_01.write(2, HIGH);
    PCF_01.write(3, LOW);
    analogWrite(PWM_A, 240);
}

void loop() {
    uint8_t read = PCF_01.read8();
    Serial.println(read, HEX);
    delay(4000);
}
