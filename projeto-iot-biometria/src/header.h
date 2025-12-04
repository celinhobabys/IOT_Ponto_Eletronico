#include <SPI.h>
#include <MFRC522.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <RotaryEncoder.h>
#include <WiFi.h>
#include <MQTT.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
//#include "certificados.h"
#include "biometria_functions.h"


// WiFi
const char* WIFI_SSID = "Projeto";           // <-- TROCAR
const char* WIFI_PASSWORD = "2022-11-07";       // <-- TROCAR

// MQTT
const char* MQTT_BROKER = "mqtt.janks.dev.br"; // <-- TROCAR
const int MQTT_PORT = 8883;
const char* MQTT_ID = "ponto_eletronico_01";   // <-- TROCAR
const char* MQTT_USER = "aula";         // <-- TROCAR
const char* MQTT_PASSWORD = "zowmad-tavQez";  // <-- TROCAR

// Tópicos MQTT
const char* TOPIC_VALIDACAO = "ponto/rfid/validacao";
const char* TOPIC_RESPOSTA = "ponto/rfid/resposta";
const char* TOPIC_REGISTRO = "ponto/registro";
const char* TOPIC_CADASTRO_INICIAR = "ponto/cadastro/iniciar";
const char* TOPIC_CADASTRO_REGISTRAR = "ponto/cadastro/registrar";
const char* TOPIC_CADASTRO_RESPOSTA = "ponto/cadastro/resposta";

// Tag RFID para teste (formato com : como no lerRFID())
const char* TAG_MESTRE = "C1:71:22:0D";        

// Pinos RFID MFRC522
#define RFID_SS_PIN 46
#define RFID_RST_PIN 17

// Pinos ePaper 
#define EPAPER_SS 10
#define EPAPER_DC 14
#define EPAPER_RST 15
#define EPAPER_BUSY 16

// Pinos Encoder 
#define ENCODER_CLK_PIN 4
#define ENCODER_DT_PIN 5


MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

GxEPD2_290_T94_V2 modeloTela(EPAPER_SS, EPAPER_DC, EPAPER_RST, EPAPER_BUSY);

GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);
U8G2_FOR_ADAFRUIT_GFX fontes;

RotaryEncoder encoder(ENCODER_DT_PIN, ENCODER_CLK_PIN);

WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1024);

// biometria
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;
int getFingerprintIDez();
uint8_t getFingerprintEnroll(int id);
void enroll();
void enrollAuto(int id);
uint8_t readnumber(void);
void reconectarMQTT();
void reconectarWiFi();
void msgMqtt(String topico, String conteudo);
void enroll();
uint8_t getFingerprintEnroll(int id);
int getFingerprintIDez();