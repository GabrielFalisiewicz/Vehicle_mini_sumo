#include "PCF8574.h"
#include "Wire.h"

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

//AsyncWebServer
AsyncWebServer server(80);
AsyncWebSocket ws("/vs");

//Buffor PWM
String buffor_data = "";
char engine;
int value_pwm;
bool data_update = false;

int value_engine_a = 0;
int value_engine_b = 0;

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
const int max_users = 3; 
const bool hidden_ssid = false;
const int expower = 20; //dBm;

IPAddress local_IP(192, 168, 5, 5);
IPAddress gateway(255, 255, 255, 0);
IPAddress sub(255, 255, 255, 0);

//Parametry silników
int max_value = 255;

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
    
    ws.onEvent(onWSEvent);
    server.addHandler(&ws);
    server.begin();
}

void set_engine(int Pin){
     if(value_pwm > max_value){
          value_pwm = max_value;
      }
      if(Pin == PWM_A){
        value_engine_a = value_pwm;
      }
      if(Pin == PWM_B){
        value_engine_b = value_pwm;
      }
      analogWrite(Pin, value_pwm);
      data_update = false;
}

void loop() {
    if(data_update){
        switch(engine){
          case 'A':
          set_engine(PWM_A);
          break;
          case 'B':
          set_engine(PWM_B);
          break;
        }
    } else {
       if(value_engine_a > 0){
          value_engine_a -= 30;
          analogWrite(PWM_A, value_engine_a);
       }
       if(value_engine_b > 0){
         value_engine_b -= 30;
         analogWrite(PWM_B, value_engine_b);
       }
    }
}

void WiFiEvent(WiFiEvent_t event){
    switch(event){
      case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("Nawiązano połączenie");
      break;
    }
   
}

void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
      Serial.println("WB connect");
    } else if(type == WS_EVT_DISCONNECT){
      Serial.println("WB disconnect");
    } else if(type == WS_EVT_DATA){
      for(size_t i = 0; i < len; i++) buffor_data += (char)data[i];
      Serial.println("Wiadomosc: ");
      Serial.println(buffor_data);
      sscanf(buffor_data.c_str(), "%c %d", &engine, &value_pwm);
      data_update = true;
    }
}
