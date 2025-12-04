#include <SPI.h>
#include <MFRC522.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <RotaryEncoder.h>
#include <WiFi.h>
#include <MQTT.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "certificados.h"
#include <Adafruit_Fingerprint.h>

// Biometria
#define mySerial Serial1
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;

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
const char* TOPIC_BUSCA_BIO = "ponto/biometria/busca";
const char* TOPIC_RETORNO_BIO = "ponto/biometria/resposta";

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
#define ENCODER_CLK_PIN 47
#define ENCODER_DT_PIN 21


MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

GxEPD2_290_T94_V2 modeloTela(EPAPER_SS, EPAPER_DC, EPAPER_RST, EPAPER_BUSY);

GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);
U8G2_FOR_ADAFRUIT_GFX fontes;

RotaryEncoder encoder(ENCODER_DT_PIN, ENCODER_CLK_PIN);

WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1024);

String metodo;

// Máquina de Estados
enum Estado { 
  AGUARDANDO,      // Esperando aproximação de TAG
  VALIDANDO,       // Consultando servidor via MQTT
  AGUARDA_DIRECAO, // TAG válida, aguardando encoder
  REGISTRANDO,     // Enviando registro via MQTT
  CONFIRMADO,      // Ponto registrado com sucesso
  NEGADO,          // Acesso negado
  ERRO_CONEXAO,    // Erro de comunicação
  MODO_CADASTRO    // Novo estado para registrar TAGs
};

Estado estadoAtual = AGUARDANDO;

// Controle de tempo
unsigned long instanteAnterior = 0;
unsigned long instanteTimeout = 0;
const long DURACAO_MENSAGEM = 3000;  
const long TIMEOUT_ENCODER = 10000;  

// Dados do usuário
String uidLido = "";
String nomeFuncionario = "";
int funcionarioId = 0;
bool usuarioValido = false;

// Modo de operação
bool modoOffline = true;  // Inicia offline, muda para online se conectar ao MQTT

// Declaração de funções (protótipos)
void reconectarWiFi();
void recebeuMensagemMQTT(String topico, String conteudo);
void reconectarMQTT();
String lerRFID();
void mostrarTelaInicial();
void mostrarVerificando();
void mostrarBoasVindas(String nome);
void mostrarConfirmacao(String tipo, String horario);
void mostrarAcessoNegado();
void mostrarErroConexao();
void mostrarModoCadastro(int step);
void mostrarTagCadastrada(String uid);
void mostrarTagJaCadastrada(String funcionario);
void validarRFID(String uid);
void registrarPonto(String tipo);
void enrollAuto(int id);
void limpaTela();
void telaPrint1(String msg);
int getFingerprintIDez();
uint8_t getFingerprintEnroll(int id);

void limpaTela(){
  tela.fillScreen(GxEPD_WHITE);
}

void telaPrint1(String msg) {
  // Título
  fontes.setFont(u8g2_font_helvB14_te);
  fontes.setFontMode(1);
  fontes.setCursor(20, 50);
  fontes.print(msg);

  tela.display(true); // Partial update
}

void enrollAuto(int id){
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (! getFingerprintEnroll(id) );
}

uint8_t getFingerprintEnroll(int id) {
  limpaTela();
  int p = -1;
  mostrarModoCadastro(2);

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  limpaTela();
  mostrarModoCadastro(3);
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  limpaTela();
  telaPrint1("Aproxime o dedo");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Cadastrado!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) mqtt.publish("catraca", String(finger.fingerID));
  else return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

void reconectarWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando ao WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
      Serial.print(".");
      delay(500);
      tentativas++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println(" conectado!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println(" falhou!");
      modoOffline = true;
    }
  }
}

void recebeuMensagemMQTT(String topico, String conteudo) {
  Serial.println("MQTT << " + topico + ": " + conteudo);
  
  if (topico == TOPIC_RESPOSTA) {
    JsonDocument doc;
    DeserializationError erro = deserializeJson(doc, conteudo);
    
    if (!erro) {
      usuarioValido = doc["valido"] | false;
      nomeFuncionario = doc["nome"] | String("Desconhecido");
      funcionarioId = doc["id"] | 0;
      
      Serial.println("Validação recebida: " + String(usuarioValido ? "VÁLIDO" : "INVÁLIDO"));
      
      if (usuarioValido) {
        estadoAtual = AGUARDA_DIRECAO;
      } else {
        estadoAtual = NEGADO;
        instanteAnterior = millis();
      }
    }
  } else if (topico == TOPIC_CADASTRO_INICIAR) {
    if (conteudo == "true") {
      estadoAtual = MODO_CADASTRO;
      mostrarModoCadastro(1);
      Serial.println("MODO CADASTRO ATIVADO");
    }
  } else if (topico == TOPIC_CADASTRO_RESPOSTA) {
    JsonDocument doc;
    DeserializationError erro = deserializeJson(doc, conteudo);

    if (!erro) {
      bool sucesso = doc["sucesso"] | false;
      String mensagem = doc["mensagem"] | String("");

      if (sucesso) {
        String uid = doc["uid"] | String("");
        String nome = doc["nome"] | String("");
        int id_user = doc["id"];
        mostrarTagCadastrada(uid);
        enrollAuto(id_user);
        Serial.println("CADASTRO OK: " + nome + " (" + uid + ")");
      } else {
        String funcionario = doc["funcionario"] | String("Desconhecido");
        Serial.println("CADASTRO FALHOU: " + mensagem);
        mostrarTagJaCadastrada(funcionario);
      }

      // Volta ao estado inicial após 3 segundos
      instanteAnterior = millis();
      estadoAtual = CONFIRMADO;
    }
  }
}

void reconectarMQTT() {
  if (!mqtt.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectando MQTT");
    
    int tentativas = 0;
    while (!mqtt.connected() && tentativas < 3) {
      if (mqtt.connect(MQTT_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.println(" conectado!");
        mqtt.subscribe(TOPIC_RESPOSTA);
        mqtt.subscribe(TOPIC_CADASTRO_INICIAR);
        mqtt.subscribe(TOPIC_CADASTRO_RESPOSTA);
        modoOffline = false;
        return;
      }
      Serial.print(".");
      delay(1000);
      tentativas++;
    }
    
    Serial.println(" falhou!");
    modoOffline = true;
  }
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

void mostrarTelaInicial() {
  Serial.println("[DEBUG] Iniciando atualização da tela inicial...");
  
  tela.fillScreen(GxEPD_WHITE);
  
  // Título
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(20, 50);
  fontes.print("PONTO ELETRONICO");
  
  // Instrução
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(40, 90);
  fontes.print("Aproxime sua TAG/Cartao");
  
  // Status de conexão
  fontes.setFont(u8g2_font_helvR10_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 120);
  if (modoOffline) {
    fontes.print("Modo: OFFLINE");
  } else {
    fontes.print("Modo: ONLINE");
  }
  
  tela.display(true); // Partial update
  
  Serial.println("[DEBUG] Tela inicial atualizada!");
}

void mostrarVerificando() {
  tela.fillScreen(GxEPD_WHITE);
  
  fontes.setFont(u8g2_font_helvB24_te);
  fontes.setFontMode(1);
  fontes.setCursor(50, 60);
  fontes.print("Verificando...");
  
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(80, 95);
  fontes.print("Aguarde");
  
  tela.display(true);
  Serial.println("Tela: VERIFICANDO");
}

void mostrarBoasVindas(String nome) {
  tela.fillScreen(GxEPD_WHITE);
  
  // Bem Vindo!
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 30);
  fontes.print("Bem-vindo(a),");
  
  // Nome do funcionário
  fontes.setFont(u8g2_font_helvB24_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 65);
  if (nome.length() > 12) {
    fontes.print(nome.substring(0, 12) + "...");
  } else {
    fontes.print(nome);
  }
  
  // Instruções
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 95);
  fontes.print("Gire a catraca:");
  
  fontes.setFont(u8g2_font_helvB14_te);
  fontes.setFontMode(1);
  fontes.setCursor(20, 115);
  fontes.print("-> Direita = ENTRADA");
  
  tela.display(true);
  Serial.println("Tela: BOAS-VINDAS - " + nome);
}

void mostrarConfirmacao(String tipo, String horario) {
  tela.fillScreen(GxEPD_WHITE);
  fontes.setFontMode(1);
  // Título
  fontes.setFont(u8g2_font_helvB24_te);
  fontes.setFontMode(1);
  fontes.setCursor(30, 40);
  fontes.print("REGISTRADO!");
  
  // Tipo
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 75);
  fontes.print("Tipo: ");
  fontes.print(tipo);
  
  // Horário
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 105);
  fontes.print("Horario: ");
  fontes.print(horario);
  
  tela.display(true);
  Serial.println("Tela: CONFIRMACAO - " + tipo);
}

void mostrarAcessoNegado() {
  tela.fillScreen(GxEPD_WHITE);
  
  // Mensagem de erro
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(30, 50);
  fontes.print("ACESSO NEGADO");
  
  // Instrução
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(30, 90);
  fontes.print("Procure a administracao");
  
  tela.display(true);
  Serial.println("Tela: ACESSO NEGADO");
}

void mostrarErroConexao() {
  tela.fillScreen(GxEPD_WHITE);
  
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(40, 50);
  fontes.print("ERRO DE CONEXAO");
  
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(50, 90);
  fontes.print("Tente novamente");
  
  tela.display(true);
  Serial.println("Tela: ERRO CONEXAO");
}

void mostrarModoCadastro(int step) {
  switch (step){
  case 1:
    tela.fillScreen(GxEPD_WHITE);
    
    fontes.setFont(u8g2_font_helvB18_te);
    fontes.setFontMode(1);
    fontes.setCursor(20, 50);
    fontes.print("MODO DE CADASTRO");
    
    fontes.setFont(u8g2_font_helvR14_te);
    fontes.setFontMode(1);
    fontes.setCursor(40, 90);
    fontes.print("Aproxime a nova TAG");
    break;
  case 2:
    tela.fillScreen(GxEPD_WHITE);
    
    fontes.setFont(u8g2_font_helvB18_te);
    fontes.setFontMode(1);
    fontes.setCursor(10, 50);
    fontes.print("COLETA DE BIOMETRIA");
    
    fontes.setFont(u8g2_font_helvR14_te);
    fontes.setFontMode(1);
    fontes.setCursor(40, 90);
    fontes.print("Aproxime o dedo do leitor");
    break;
  case 3:
    tela.fillScreen(GxEPD_WHITE);
    
    fontes.setFont(u8g2_font_helvB18_te);
    fontes.setFontMode(1);
    fontes.setCursor(10, 50);
    fontes.print("COLETA DE BIOMETRIA");
    
    fontes.setFont(u8g2_font_helvR14_te);
    fontes.setFontMode(1);
    fontes.setCursor(40, 90);
    fontes.print("Remova o dedo do leitor");
    break;
  default:
    break;
  }
  tela.display(true); // Partial update
  
  Serial.println("Tela: MODO CADASTRO");
}

void mostrarTagCadastrada(String uid) {
  tela.fillScreen(GxEPD_WHITE);
  
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(20, 50);
  fontes.print("TAG CADASTRADA!");
  
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 90);
  fontes.print("UID: " + uid);
  
  tela.display(true);
  
  Serial.println("Tela: TAG CADASTRADA - " + uid);
}

void mostrarTagJaCadastrada(String funcionario) {
  tela.fillScreen(GxEPD_WHITE);
  
  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 40);
  fontes.print("TAG JA CADASTRADA!");
  
  fontes.setFont(u8g2_font_helvR14_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 75);
  fontes.print("Funcionario:");
  
  fontes.setFont(u8g2_font_helvB14_te);
  fontes.setFontMode(1);
  fontes.setCursor(10, 100);
  if (funcionario.length() > 20) {
    fontes.print(funcionario.substring(0, 20) + "...");
  } else {
    fontes.print(funcionario);
  }
  
  tela.display(true);
  
  Serial.println("Tela: TAG JA CADASTRADA - " + funcionario);
}

void validarBiometria(int id){
  Serial.println("Enviando validacao via MQTT...");
  
  JsonDocument doc;
  doc["id"] = id;
  
  String payload;
  serializeJson(doc, payload);
  
  if (mqtt.publish(TOPIC_BUSCA_BIO, payload)) {
    Serial.println("MQTT >> " + String(TOPIC_BUSCA_BIO) + ": " + payload);
  // Aguarda resposta (será processada em recebeuMensagemMQTT)
  } else {
    Serial.println("ERRO: Falha ao publicar no MQTT");
    estadoAtual = ERRO_CONEXAO;
    instanteAnterior = millis();
  }
}

void validarRFID(String uid) {
  if (modoOffline) {
    // Modo offline: valida localmente com TAG_MESTRE
    Serial.println("Validando localmente...");
    
    if (uid == TAG_MESTRE) {
      usuarioValido = true;
      nomeFuncionario = "Usuario Teste";
      funcionarioId = 999;
      estadoAtual = AGUARDA_DIRECAO;
      Serial.println("TAG VALIDA (offline)");
    } else {
      usuarioValido = false;
      estadoAtual = NEGADO;
      instanteAnterior = millis();
      Serial.println("TAG INVALIDA (offline)");
    }
  } else {
    // Modo online: envia para servidor via MQTT
    Serial.println("Enviando validacao via MQTT...");
    
    JsonDocument doc;
    doc["uid"] = uid;
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqtt.publish(TOPIC_VALIDACAO, payload)) {
      Serial.println("MQTT >> " + String(TOPIC_VALIDACAO) + ": " + payload);
      // Aguarda resposta (será processada em recebeuMensagemMQTT)
    } else {
      Serial.println("ERRO: Falha ao publicar no MQTT");
      estadoAtual = ERRO_CONEXAO;
      instanteAnterior = millis();
    }
  }
}

void registrarPonto(String tipo) {
  unsigned long agora = millis() / 1000;
  String horario = String(agora);
  
  if (!modoOffline && mqtt.connected()) {
    JsonDocument doc;
    doc["funcionario_id"] = funcionarioId;
    doc["tipo"] = tipo;
    // Usar schema para autoadd o timestamp no servidor
    // doc["timestamp"] = horario;
    // TODO: Consertar
    doc["metodo"] = metodo;
    
    String payload;
    serializeJson(doc, payload);
    
    if (mqtt.publish(TOPIC_REGISTRO, payload)) {
      Serial.println("MQTT >> " + String(TOPIC_REGISTRO) + ": " + payload);
    } else {
      Serial.println("ERRO: Falha ao publicar registro");
    }
  } else {
    Serial.println("REGISTRO OFFLINE: " + nomeFuncionario + " - " + tipo + " - " + horario);
  }
  
  mostrarConfirmacao(tipo, horario);
  estadoAtual = CONFIRMADO;
  instanteAnterior = millis();
}

void setup() {
  // Inicializa Serial
  Serial.begin(115200);
  delay(500);
  Serial.println("PONTO ELETRONICO");


  // Inicializa SPI
  Serial.println("[INFO] Configurando SPI...");
  SPI.begin();  // Usa pinos padrão do ESP32-S3
  Serial.println("[OK] SPI configurado");
  
  // Inicializa RFID
  Serial.println("[INFO] Inicializando leitor RFID...");
  rfid.PCD_Init();
  Serial.println("[OK] Leitor RFID iniciado");

  // Inicializa ePaper
  Serial.println("[INFO] Inicializando display ePaper...");
  Serial.print("  - CS (SS): GPIO ");
  Serial.println(EPAPER_SS);
  Serial.print("  - DC: GPIO ");
  Serial.println(EPAPER_DC);
  Serial.print("  - RST: GPIO ");
  Serial.println(EPAPER_RST);
  Serial.print("  - BUSY: GPIO ");
  Serial.println(EPAPER_BUSY);
  
  tela.init(115200, true, 2, false); // Serial debug, initial, reset_duration, pulldown_rst_mode
  tela.setRotation(3);
  tela.setFullWindow();
  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);
  Serial.println("[OK] Display ePaper iniciado");

  // Inicializa Encoder
  Serial.println("[OK] Encoder iniciado");

  // Tenta conectar WiFi
  Serial.println("\n[INFO] Tentando conectar ao WiFi...");
  reconectarWiFi();
  
  // Configura certificado para conexão segura
  Serial.println("[INFO] Configurando certificado SSL/TLS...");
  conexaoSegura.setCACert(certificado1);
  Serial.println("[OK] Certificado configurado");
  
  // Tenta conectar MQTT
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[INFO] Tentando conectar ao MQTT...");
    mqtt.begin(MQTT_BROKER, MQTT_PORT, conexaoSegura);
    mqtt.onMessage(recebeuMensagemMQTT);
    reconectarMQTT();
  }
  
  // Exibe modo de operação
  if (modoOffline) {
    Serial.println("\n[AVISO] Operando em MODO OFFLINE");
    Serial.println("TAG mestre configurada: " + String(TAG_MESTRE));
  } else {
    Serial.println("\n[OK] Operando em MODO ONLINE");
  }

  // Teste rápido do ePaper
  Serial.println("\n[INFO] Testando ePaper...");
  tela.fillScreen(GxEPD_WHITE);
  tela.display(false); // Full update
  delay(1000);
  
  Serial.println("[INFO] Teste concluído. Carregando tela inicial...");
  
  // Sensor de dedinho 123
  Serial1.begin(57600, SERIAL_8N1, 44, 43);
  finger.begin(57600);
  delay(5);
  while(!finger.verifyPassword()){
    Serial.println("Did not find fingerprint sensor :(");
    delay(1000);
  }

  Serial.println("Found fingerprint sensor!");
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data.");
  }
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }

  // Mostra tela inicial
  delay(500);
  mostrarTelaInicial();
  estadoAtual = AGUARDANDO;
  
  Serial.println("Sistema pronto! Aproxime uma TAG.");

}



// LOOP PRINCIPAL (Máquina de Estados)
void loop() {
  // Processa o encoder
  encoder.tick();

  // Mantém conexões ativas (se online)
  if (!modoOffline) {
    reconectarWiFi();
    reconectarMQTT();
    mqtt.loop();
  }

  unsigned long agora = millis();
  
  switch (estadoAtual) {
    
    // ESTADO: AGUARDANDO
    case AGUARDANDO: {
      // Verifica se tem nova TAG RFID
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        uidLido = lerRFID();
        metodo = "RFID";
        Serial.println("\n--- TAG DETECTADA ---");
        Serial.println("UID: " + uidLido);
        
        // Vai para validação
        mostrarVerificando();
        estadoAtual = VALIDANDO;
        instanteAnterior = agora;
        validarRFID(uidLido);
      }
      if (agora > instanteAnterior + 1000) {
        int x = getFingerprintIDez();
        if(x != -1) {
          estadoAtual = AGUARDA_DIRECAO;
          metodo = "Biometria";
          funcionarioId = x;
        }
        instanteAnterior = agora;
      }

      break;
    }
    
    // ESTADO: VALIDANDO
    case VALIDANDO: {
      if (agora - instanteAnterior > 5000) {
        Serial.println("TIMEOUT na validacao");
        estadoAtual = ERRO_CONEXAO;
        instanteAnterior = agora;
      }
      break;
    }
    
    // ESTADO: AGUARDA_DIRECAO
    case AGUARDA_DIRECAO: {
      // Mostra tela de boas-vindas (apenas uma vez)
      static bool telaExibida = false;
      if (!telaExibida) {
        mostrarBoasVindas(nomeFuncionario);
        encoder.setPosition(0); // Reseta a posição do encoder
        instanteTimeout = agora;
        telaExibida = true;
      }
      
      // Lê a direção do encoder
      RotaryEncoder::Direction direcao = encoder.getDirection();
      
      if (direcao != RotaryEncoder::Direction::NOROTATION) {
        String tipo = "";
        
        if (direcao == RotaryEncoder::Direction::CLOCKWISE) {
          tipo = "ENTRADA";
          Serial.println("Encoder girou para DIREITA -> ENTRADA");
        } else if (direcao == RotaryEncoder::Direction::COUNTERCLOCKWISE) {
          tipo = "SAIDA";
          Serial.println("Encoder girou para ESQUERDA -> SAIDA");
        }
        
        // Registra o ponto
        registrarPonto(tipo);
        telaExibida = false;  // Reset para próxima vez
      }
      
      // Timeout: volta ao início após 10 segundos sem movimento
      if (agora - instanteTimeout > TIMEOUT_ENCODER) {
        Serial.println("TIMEOUT aguardando encoder");
        mostrarTelaInicial();
        estadoAtual = AGUARDANDO;
        telaExibida = false;
      }
      break;
    }
    

    // ESTADO: REGISTRANDO
    case REGISTRANDO: {
      // Este estado é transitório, vai direto para CONFIRMADO
      estadoAtual = CONFIRMADO;
      break;
    }
    

    // ESTADO: CONFIRMADO
    case CONFIRMADO: {
      // Aguarda 3 segundos e volta ao início
      if (agora - instanteAnterior > DURACAO_MENSAGEM) {
        mostrarTelaInicial();
        estadoAtual = AGUARDANDO;
      }
      break;
    }
    

    // ESTADO: NEGADO
    case NEGADO: {
      // Mostra tela de acesso negado (apenas uma vez)
      static bool telaNegadoExibida = false;
      if (!telaNegadoExibida) {
        mostrarAcessoNegado();
        telaNegadoExibida = true;
      }
      
      // Aguarda 3 segundos e volta ao início
      if (agora - instanteAnterior > DURACAO_MENSAGEM) {
        mostrarTelaInicial();
        estadoAtual = AGUARDANDO;
        telaNegadoExibida = false;
      }
      break;
    }
    

    // ESTADO: ERRO_CONEXAO
    case MODO_CADASTRO: {
      static bool telaCadastroExibida = false;
      if (!telaCadastroExibida) {
        mostrarModoCadastro(1);
        telaCadastroExibida = true;
      }

      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        uidLido = lerRFID();
        Serial.println("\n--- NOVA TAG PARA CADASTRO ---");
        Serial.println("UID: " + uidLido);

        if (!modoOffline && mqtt.connected()) {
          JsonDocument doc;
          doc["uid"] = uidLido;
          
          String payload;
          serializeJson(doc, payload);
          
          if (mqtt.publish(TOPIC_CADASTRO_REGISTRAR, payload)) {
            Serial.println("MQTT >> " + String(TOPIC_CADASTRO_REGISTRAR) + ": " + payload);
            mostrarTagCadastrada(uidLido);
          } else {
            Serial.println("ERRO: Falha ao publicar tag para cadastro");
            mostrarErroConexao();
          }
        } else {
          Serial.println("ERRO: Sem conexão para cadastrar");
          mostrarErroConexao();
        }

        delay(DURACAO_MENSAGEM);

        mostrarTelaInicial();
        estadoAtual = AGUARDANDO;
        telaCadastroExibida = false;
      }
      break;
    }

    case ERRO_CONEXAO: {
      // Mostra tela de erro
      static bool telaErroExibida = false;
      if (!telaErroExibida) {
        mostrarErroConexao();
        telaErroExibida = true;
      }
      
      // Aguarda 3 segundos e volta ao início
      if (agora - instanteAnterior > DURACAO_MENSAGEM) {
        mostrarTelaInicial();
        estadoAtual = AGUARDANDO;
        telaErroExibida = false;
      }
      break;
    }
  }
}