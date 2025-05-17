#include "PCF8574.h"
#include "Wire.h"

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"

//AsyncWebServer
AsyncWebServer server(1234);
AsyncWebSocket ws("/ws");

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
const char* password = ".";

//Parametry silników
int max_value = 255;

//PWM Ferquency
#define FERQ 5000

void set_start_config_track() {
    PCF_01.write(0, HIGH);
    PCF_01.write(1, LOW);
    PCF_01.write(2, LOW);
    PCF_01.write(3, HIGH);
}

void change_direction_track(int first_pin, int sec_pin) {
    PCF_01.toggle(first_pin);
    PCF_01.toggle(sec_pin);
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

void onWSEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WB connect");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WB disconnect");
    } else if (type == WS_EVT_DATA) {
        buffor_data = "";
        for (size_t i = 0; i < len; i++) buffor_data += (char)data[i];
        Serial.println("Wiadomość: ");
        value_pwm = buffor_data.toInt();
        Serial.println(value_pwm);
        value_engine_a += value_pwm;
        value_engine_b += value_pwm;
        data_update = true;
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
    set_start_config_track();

    WiFi.onEvent(WiFiEvent);

    WiFi.begin(ssid, password);
    Serial.println("Łączenie z WiFi");

    if (!WiFi.status() != WL_CONNECTED) {
        Serial.println("Błąd Łączenia WiFi");
        delay(500);
    }

    // Inicjalizacja WebSocket
    ws.onEvent(onWSEvent);
    server.addHandler(&ws);
    server.begin();
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
}

void loop() {
    current_time = millis();
    if (data_update) {
        Serial.println("Przypisane");
        Serial.println(value_engine_a);
        Serial.println(value_engine_b);
        analogWrite(PWM_A, value_engine_a);
        analogWrite(PWM_B, value_engine_b);
        data_update = false;
    }
    if(current_time - last_time > 3000 && !data_update && value_engine_a > 0 && value_engine_b > 0){
        last_time = current_time;
        value_engine_a -= 10;
        value_engine_b -= 10;
        analogWrite(PWM_A, value_engine_a);
        analogWrite(PWM_B, value_engine_b);
    }
  
}
