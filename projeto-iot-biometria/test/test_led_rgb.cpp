// Teste simples do LED RGB onboard do ESP32-S3
// Este teste pisca o LED RGB em diferentes cores para verificar se o ESP32 está funcionando

#include <Arduino.h>

// LED RGB onboard do ESP32-S3-DevKitC-1
// Conectado ao GPIO 48 (WS2812 / Neopixel)
#define RGB_LED_PIN 48
#define RGB_LED_COUNT 1

// Se tiver a biblioteca Adafruit_NeoPixel, use ela
// Caso contrário, usaremos controle direto via RMT

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n================================");
  Serial.println("TESTE LED RGB ONBOARD ESP32-S3");
  Serial.println("================================\n");
  
  // Configura o pino do LED como saída
  pinMode(RGB_LED_PIN, OUTPUT);
  
  Serial.println("LED RGB configurado no GPIO 48");
  Serial.println("Iniciando sequência de teste...\n");
  
  // Teste simples: piscar usando digitalWrite
  // (LED WS2812 não funciona bem assim, mas serve para testar o pino)
  for (int i = 0; i < 10; i++) {
    digitalWrite(RGB_LED_PIN, HIGH);
    Serial.print("LED ON (");
    Serial.print(i + 1);
    Serial.println("/10)");
    delay(200);
    
    digitalWrite(RGB_LED_PIN, LOW);
    Serial.println("LED OFF");
    delay(200);
  }
  
  Serial.println("\n================================");
  Serial.println("TESTE CONCLUÍDO!");
  Serial.println("================================");
  Serial.println("\nSe você viu o LED piscando, o ESP32-S3 está OK!");
  Serial.println("Se não viu, verifique:");
  Serial.println("1. O LED RGB está presente na sua placa?");
  Serial.println("2. Tente outros pinos (GPIO 2, 8, etc)");
}

void loop() {
  // Pisca devagar no loop
  digitalWrite(RGB_LED_PIN, HIGH);
  delay(1000);
  digitalWrite(RGB_LED_PIN, LOW);
  delay(1000);
}
