#include "PCF8574.h"
#include "Wire.h"

#include "WiFi.h"
#include "AsyncUDP.h"

//Konfiguracja ekspandera
PCF8574 PCF_01(0x20);

//Konfiguracja PWM
#define PWM_A 2
#define PWM_B 3

//Konfiguracja WiFi
const char* ssid = "vehicle_wifi_sumo";
const char* password = "AtSumo01";
const int channel = 6;
//Samo urządzenie + diagnostyka
const int max_users = 2; 
const bool hidden_ssid = false;
const int expower = 20; //dBm;

IPAddress local_IP(192, 168, 5, 5);
IPAddress gateway(255, 255, 255, 0);
IPAddress sub(255, 255, 255, 0);

//Ustawienia UDP
const int udp_channel_A_port = 1234;
const int udp_channel_B_port = 5678;

AsyncUDP udp_channel_A;
AsyncUDP udp_channel_B;

String udp_message_a;
String udp_message_b;

//Parametry silników
int max_value = 255;
int value_engine_a = 0;
int value_engine_b = 0;

//PWM Ferquency
#define FERQ 5000

void set_start_config_track(){
    PCF_01.write(0, LOW);
    PCF_01.write(1, HIGH);
    PCF_01.write(2, HIGH);
    PCF_01.write(3, LOW);
}



void change_direction_track(int first_pin, int sec_pin){
   PCF_01.toggle(first_pin);
   PCF_01.toggle(sec_pin);
}


void setup() {
    Serial.begin(115200);
    Wire.begin(6, 7);
    PCF_01.begin();
    analogWriteFrequency(FERQ);
    analogWriteResolution(8);
    pinMode(PWM_A, OUTPUT);
    pinMode(PWM_B, OUTPUT);
    set_start_config_track();
    WiFi.softAP(ssid, password, channel, hidden_ssid, max_users);
    WiFi.softAPConfig(local_IP, gateway, sub);
    WiFi.setTxPower((wifi_power_t)expower);
    WiFi.onEvent(WiFiEvent);

    //Obsługa pakietów
    if(udp_channel_A.listen(udp_channel_A_port)){
        Serial.println("Kanał A jest aktywny");
        udp_channel_A.onPacket([](AsyncUDPPacket packet){
            udp_message_a = String((char*)packet.data());
            value_engine_a = udp_message_a.toInt();
        });
    }

    if(udp_channel_B.listen(udp_channel_B_port)){
        Serial.println("Kanał B jest aktywny");
        udp_channel_B.onPacket([](AsyncUDPPacket packet){
            udp_message_b = String((char*)packet.data());
            value_engine_b = udp_message_a.toInt();
        });
    }
}

void loop() {
    
}

void WiFiEvent(WiFiEvent_t event){
    switch(event){
      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("Nawiązano połączenie");
      break;
    }
    
}
