#include "PCF8574.h"
#include "Wire.h"

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

//AsyncWebServer
AsyncWebServer server(1234);
AsyncWebSocket ws1("/ws");
AsyncWebSocket ws2("/wr");

//Buffor PWM
String buffor_data = "";
int value_pwm;
bool data_update = false;

int value_engine_a = 0;
int value_engine_b = 0;

//Konfiguracja ekspandera
PCF8574 PCF_01(0x20);

//Konfiguracja PWM
#define PWM_A 2
#define PWM_B 3

//Obsługa czasu
long current_time;
long last_time = 0.0;

//Konfiguracja WiFi (client mode)
const char* ssid = "JF";
const char* password = "wifiFJ11";

//Parametry silników
int max_value = 250;

//PWM Ferquency
#define FERQ 5000



void set_left_track(int a, int b){
    PCF_01.write(0, a);
    PCF_01.write(1, b);
}

void set_right_track(int c, int d){
    PCF_01.write(2, c);
    PCF_01.write(3, d);
}

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Połączono z WiFi.");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("Adres IP: ");
            Serial.println(WiFi.localIP());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Rozłączono z WiFi, ponawianie połączenia...");
            WiFi.reconnect();
            break;
        default:
            break;
    }
}



void onWSEvent1(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WB connect left");
        delay(100);
        ws1.textAll("led_01");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WB disconnect");
    } else if (type == WS_EVT_DATA) {
        buffor_data = "";
        for (size_t i = 0; i < len; i++) buffor_data += (char)data[i];
        int first_search = buffor_data.indexOf(" ");
        String message = buffor_data.substring(0, first_search);
        value_pwm = buffor_data.substring(first_search + 1, buffor_data.length()).toInt();
        Serial.println(message);
        Serial.println(value_engine_a);
        if(message.equals("left_pad")){
          value_pwm = map(value_pwm, -100, 100, -255, 255);
          value_engine_a = value_pwm;
        }
    }
}

void onWSEvent2(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WB connect right");
        delay(100);
        ws2.textAll("led_04");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WB disconnect");
    } else if (type == WS_EVT_DATA) {
        buffor_data = "";
        for (size_t i = 0; i < len; i++) buffor_data += (char)data[i];
        int first_search = buffor_data.indexOf(" ");
        String message = buffor_data.substring(0, first_search);
        value_pwm = buffor_data.substring(first_search + 1, buffor_data.length()).toInt();
         Serial.println(message);
        if(message.equals("right_pad")){
          value_pwm = map(value_pwm, -100, 100, -255, 255);
          value_engine_b = value_pwm;
        }
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(6, 7);
    PCF_01.begin();
    analogWriteFrequency(FERQ);
    analogWriteResolution(8);
    pinMode(PWM_A, OUTPUT);
    pinMode(PWM_B, OUTPUT);

    set_left_track(1, 0);
    set_right_track(0, 1);

    WiFi.onEvent(WiFiEvent);

    WiFi.begin(ssid, password);
    Serial.println("Łączenie z WiFi");

    if (!WiFi.status() != WL_CONNECTED) {
        Serial.println("Błąd Łączenia WiFi");
        delay(500);
    }

    // Inicjalizacja WebSocket
    ws1.onEvent(onWSEvent1);
    server.addHandler(&ws1);
    ws2.onEvent(onWSEvent2);
    server.addHandler(&ws2);
    server.begin();
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
}

void set_config_engines(int value_a, int value_b){

    if(value_a <= 0){
        set_left_track(0, 1);
    } else {
        set_left_track(1, 0);
    }

    if(value_b <= 0){
       set_right_track(1, 0);
    } else {
       set_right_track(0, 1);
    }

    if(value_a < 0){
        value_a *= (-1);
      
    }
    if(value_b < 0){
        value_b *= (-1);
    }

    analogWrite(PWM_A, value_a);
    analogWrite(PWM_B, value_b);
}

void loop() {
   
    current_time = millis();
    set_config_engines(value_engine_a, value_engine_b);
    if(current_time - last_time > 3000){
        last_time = current_time;
    }
  
}
