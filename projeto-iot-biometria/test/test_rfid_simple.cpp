
#include <SPI.h>
#include <MFRC522.h>

MFRC522 rfid(46, 17); // Pinos SS e RST
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
unsigned long instanteAnterior = 0;


void setup() {
 Serial.begin(115200); delay(500);
 SPI.begin();
 rfid.PCD_Init();
}

String lerRFID() {
 String id = "";
 for (byte i = 0; i < rfid.uid.size; i++) {
 if (i > 0) {
 id += ":";
 }
 if (rfid.uid.uidByte[i] < 0x10) {
 id += "0";
 }
 id += String(rfid.uid.uidByte[i], HEX);
 }
 id.toUpperCase();
 return id;
}

String lerTextoDoBloco (byte bloco) {
 byte tamanhoDados = 18;
 char dados[tamanhoDados];

 MFRC522::StatusCode status = rfid.PCD_Authenticate(
 MFRC522::PICC_CMD_MF_AUTH_KEY_A,
 bloco, &chaveA, &(rfid.uid)
 );

 if (status != MFRC522::STATUS_OK) { return ""; }

 status = rfid.MIFARE_Read(bloco, (byte*)dados, &tamanhoDados);
 if (status != MFRC522::STATUS_OK) { return ""; }

 dados[tamanhoDados - 2] = '\0'; // Fim da string
 rfid.PICC_HaltA();
 rfid.PCD_StopCrypto1();
 return String(dados);
}

void loop() {
 unsigned long agora = millis();
 
 // Print a cada 1 segundo
 if (agora - instanteAnterior > 1000) {
   Serial.println("[DEBUG] Aguardando TAG... (loop ativo)");
   instanteAnterior = agora;
 }

 if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
   String id = lerRFID();
   Serial.println("\n--- TAG DETECTADA ---");
   Serial.println("UID: " + id);

   String texto = lerTextoDoBloco(6);
   Serial.println("Bloco 6: " + texto);
   Serial.println("---------------------\n");
   
   delay(1000); // Evita leituras múltiplas
 }
}