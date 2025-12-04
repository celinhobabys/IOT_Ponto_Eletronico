## 1\. Conceitos Básicos de Arduino/ESP32

### Sensor de Luz

Leitura de um sensor analógico e mapeamento do valor para porcentagem.

```cpp
int leitura = analogRead(pino);
int porcentagemLuz = map(leitura, 0, 4095, 0, 100);
```

### Millis (Temporização)

Uso da função `millis()` para executar código em intervalos definidos sem usar `delay()`.

```cpp
unsigned long instanteAnterior = 0;

void loop() {
 unsigned long instanteAtual = millis();
 if (instanteAtual > instanteAnterior + 1000) {
 Serial.println("+1 segundo");
 instanteAnterior = instanteAtual;
 }
}
```

### LED

Configuração de um pino como saída e controle do estado (HIGH/LOW).

```cpp
void setup() {
 pinMode(pinoLED, OUTPUT);
}

digitalWrite(pinoLED, HIGH);
digitalWrite(pinoLED, LOW);
```

### Serial

Comunicação serial para debug e entrada/saída.

```cpp
void setup () {
 Serial.begin(115200); while(!Serial);
}

void loop() {
 if (Serial.available() > 0) {
 String texto = Serial.readStringUntil('\n');
 Serial.println(texto);
 }
}
```

### String

Manipulação de objetos `String`.

```cpp
// Criação e conversão
String texto1 = "Olá, mundo!";
int numero = 100 * 2;
String texto2 = String(numero);
int numero2 = texto2.toInt() + 42;

// Concatenação
String texto3 = "aaa" + texto2;

// Comparação
bool ehIgual = texto2 == texto3;
bool comecaComola = texto1.startsWith("Olá");

// Acesso e tamanho
char caracter = texto1[2]; // 'á'
int totalCaracteres = texto1.length(); // 11

// Substring
String trecho = texto1.substring(0, 3); // "Olá"
String trechoFinal = texto1.substring(5); // "mundo!"

// Substituição
String texto4 = "abc abc\n";
texto4.replace("ab", "AB"); // "ABC ABC"
```

-----

## 2\. Botões e Sensores (GFButton)

### LED RGB

Controle de um LED RGB.

```cpp
rgbLedWrite(RGB_BUILTIN, vermelho, verde, azul);
```

### Botão e Sensor de Movimento

Uso da biblioteca `GFButton` para lidar com eventos de pressionar (press) e soltar (release).

```cpp
#include <GFButton.h>

GFButton botao(1);
GFButton sensor(21); // Sensor de movimento

// Handlers para o botão
void botaoPressionado (GFButton& botaoDoEvento) {
 Serial.println("Botão foi pressionado!");
}
void botaoSolto (GFButton& botaoDoEvento) {
 Serial.println("Botão foi solto!");
}

// Handlers para o sensor de movimento
void movimento (GFButton& sensor) {
 Serial.println("Movimento detectado!");
}
void inercia (GFButton& sensor) {
 Serial.println("Inércia detectada!");
}

void setup () {
 Serial.begin(115200);

 // Configura botão
 botao.setPressHandler(botaoPressionado);
 botao.setReleaseHandler(botaoSolto);

 // Configura sensor
 sensor.setPressHandler(movimento);
 sensor.setReleaseHandler(inercia);
}

void loop() {
 botao.process();
 sensor.process();
}

// Verificação manual
bool estadoBotao = botao.isPressed();
bool estadoSensor = sensor.isPressed();
```

### Botão com Passagem de Parâmetro

Uso de uma função lambda para passar parâmetros para o handler.

```cpp
void minhaFuncao (int x) {
 Serial.println(x);
}

void setup () {
 Serial.begin(115200);
 botao.setPressHandler([] (GFButton &b) { minhaFuncao(42); });
}
```

-----

## 3\. Rede e Comunicação

### WiFi

Conexão e reconexão a uma rede WiFi.

```cpp
#include <WiFi.h>

void reconectarWiFi() {
 if (WiFi.status() != WL_CONNECTED) {
 WiFi.begin("NOME DA REDE", "SENHA DA REDE");
 Serial.print("Conectando ao WiFi...");
 while (WiFi.status() != WL_CONNECTED) {
 Serial.print(".");
 delay(1000);
 }
 Serial.print("conectado! \nEndereço IP: ");
 Serial.println(WiFi.localIP());
 }
}

void setup () {
 Serial.begin(115200); delay(500);
 reconectarWiFi();
}

void loop() {
 reconectarWiFi();
}
```

### MQTT

Comunicação usando o protocolo MQTT. O Resumo 08 usa uma conexão insegura (porta 1883) e o Resumo 05 usa uma segura (porta 8883).

```cpp
// Bibliotecas comuns
#include <WiFi.h>
#include <MQTT.h>

// Para conexão insegura (Resumo08)
#include <WiFiClient.h>
WiFiClient conexao;

// Para conexão segura (Resumo05)
#include <WiFiClientSecure.h>
#include "certificados.h" // Resumo 08 também inclui, mas não usa
WiFiClientSecure conexaoSegura;

MQTTClient mqtt(1000);

void recebeuMensagem (String topico, String conteudo) {
 reconectarMQTT();
 Serial.println(topico + ": " + conteudo);
}

void reconectarMQTT() {
 if (!mqtt.connected()) {
 Serial.print("Conectando MQTT...");
 while(!mqtt.connected()) {
 mqtt.connect("SEU ID", "LOGIN", "SENHA");
 Serial.print(".");
 delay(1000);
 }
 Serial.println(" conectado!");
 mqtt.subscribe("topico1"); // qos=0
 mqtt.subscribe("topico2/+/parametro", 1); // qos=1
 }
}

void setup() {
 Serial.begin(115200);
 delay(500);
 reconectarWiFi();

 // Configuração segura (Resumo05)
 // conexaoSegura.setCACert(certificado1);
 // mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);

 // Configuração insegura (Resumo08)
 mqtt.begin("mqtt.janks.dev.br", 1883, conexao);

 mqtt.onMessage(recebeuMensagem);
 mqtt.setKeepAlive(10);
 mqtt.setWill("tópico da desconexão", "conteúdo");
}

void loop() {
 reconectarWiFi();
 reconectarMQTT();
 mqtt.loop();

 // Exemplo de envio
 mqtt.publish("topico", "conteúdo"); // retain=false, qos=0
 mqtt.publish("topico2/1234/parametro", "conteúdo 2", false, 1);
}
```

-----

## 4\. Dados e Armazenamento

### JSON (ArduinoJson)

Serialização e desserialização de dados JSON.

```cpp
#include <ArduinoJson.h>

// Serialização (Criar JSON)
JsonDocument dados;
dados["número"] = 12345;
dados["texto"] = "IoT";

String textoJson;
serializeJson(dados, textoJson);
serializeJson(dados, Serial);

// Desserialização (Ler JSON)
String texto_json2 = "[10, 20, 30]";
JsonDocument lista;
deserializeJson(lista, texto_json2);

// Acessar array
for (unsigned int i=0; i < lista.size(); i++) {
 int elemento = lista[i];
 Serial.println(elemento);
}
```

### LittleFS (Sistema de Arquivos)

Leitura e escrita de arquivos no sistema de arquivos flash (LittleFS).

```cpp
#include <LittleFS.h>

void setup() {
 if (!LittleFS.begin()) {
 Serial.println("LittleFS falhou!");
 while (true) {};
 }
}

// Escrita de Arquivo
File arquivo = LittleFS.open("/arquivo.txt", "w");
arquivo.println("IoT");
arquivo.print("PUC-Rio ");
arquivo.println(2020 + 4);
arquivo.close();

// Leitura de Arquivo
File arquivo = LittleFS.open("/arquivo.txt", "r");
if (!arquivo) {
 Serial.println("Arquivo falhou!");
 while (true) {};
}
String conteudo = arquivo.readString();
arquivo.close();

// Arquivo JSON
JsonDocument dados;
// ...
// deserializeJson(dados, arquivo); // leitura
// serializeJson(dados, arquivo); // escrita
```

Para enviar arquivos para o ESP32, deve-se usar a opção "Upload Filesystem Image" na IDE (PlatformIO).

### Web Server

Criação de um servidor web no ESP32.

```cpp
#include <WebServer.h>
#include <uri/UriBraces.h>

WebServer servidor(80);

// Handler para página simples
void pagina1 () {
 servidor.send(200, "text/html", "Bem-vindo!");
}

// Handler para página com HTML do LittleFS
void pagina2 () {
 File arquivo = LittleFS.open("/pagina.html", "r");
 if (!arquivo) {
 servidor.send(500, "text/html", "Erro no HTML");
 return;
 }
 String html = arquivo.readString();
 arquivo.close();
 html.replace("{{nome}}", "Jan"); // Substituição de template
 servidor.send(200, "text/html", html);
}

// Handler para página com parâmetros
void pagina3 () {
 String texto = servidor.pathArg(0);
 int numero = servidor.pathArg(1).toInt();
 servidor.send(200, "text/html", "Dados ok");
}

// Handler para dados POST
void tratarDados () {
 String email = servidor.arg("email");
 String mensagem = servidor.arg("mensagem");
 // ... faz alguma coisa ...
 // Redireciona
 servidor.sendHeader("Location", "/inicio");
 servidor.send(303);
}

void setup() {
 reconectarWiFi();

 // Rotas
 servidor.on("/inicio", HTTP_GET, pagina1);
 servidor.on("/contato", HTTP_GET, pagina2);
 servidor.on("/contato", HTTP_POST, tratarDados);
 servidor.on(UriBraces("/parametros/{}/{}"), HTTP_GET, pagina3);

 servidor.begin();
}

void loop() {
 reconectarWiFi();
 servidor.handleClient();
}
```

-----

## 5\. Sensores Avançados

### Sensor BME680 (Temperatura, Umidade, Pressão)

Leitura de múltiplos dados ambientais.

```cpp
#include <Adafruit_BME680.h>

Adafruit_BME680 sensorBME;

void setup() {
 Serial.begin(115200); delay(500);

 if (!sensorBME.begin()) {
 Serial.println("Erro no sensor BME");
 while (true);
 }

 // Configuração de amostragem
 sensorBME.setTemperatureOversampling(BME680_OS_8X);
 sensorBME.setHumidityOversampling(BME680_OS_2X);
 sensorBME.setPressureOversampling(BME680_OS_4X);
 sensorBME.setIIRFilterSize(BME680_FILTER_SIZE_3);
 sensorBME.setGasHeater(320, 150); // 320°C por 150ms
}

// Medição dos Dados
sensorBME.performReading();

float temperatura = sensorBME.temperature; // °C
float pressao = sensorBME.pressure / 100.0; // hPa
float altitude = sensorBME.readAltitude(1013.25); // m
float umidade = sensorBME.humidity; // %
float resistencia_gas = sensorBME.gas_resistance / 1000.0; // KΩ
```

### RFID-RC522

Leitura de tags RFID.

```cpp
#include <SPI.h>
#include <MFRC522.h>

MFRC522 rfid(46, 17); // Pinos SS e RST
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

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
 if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
 String id = lerRFID();
 Serial.println("UID: " + id);

 String texto = lerTextoDoBloco(6);
 Serial.println("Bloco 6: " + texto);
 }
}
```

### Célula de Carga (Balança)

Leitura de peso usando o módulo HX711.

```cpp
#include <HX711.h>

HX711 balanca;

void setup() {
 Serial.begin(115200); delay(500);
 balanca.begin(6, 7); // Pinos DT e SCK

 // Cálculo da escala: escala = medição / peso
 // Ex: medição = 132600 , peso = 300g -> escala = 442
 balanca.set_scale(SEU_FATOR_DE_ESCALA);

 balanca.tare(5); // Define a tara com 5 medições
}

// Leitura do Peso
float pesoMedido = balanca.get_units(1); // 1 medição
float pesoMedio = balanca.get_units(5); // média de 5 medições
```

-----

## 6\. Displays e Códigos

### Display ePaper

Desenho de formas e textos em um display ePaper.

```cpp
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

U8G2_FOR_ADAFRUIT_GFX fontes;

// Definição do modelo de tela
GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);

void setup() {
 tela.init();
 tela.setRotation(3);
 tela.fillScreen(GxEPD_WHITE);

 // Configuração das fontes
 fontes.begin(tela);
 fontes.setForegroundColor(GxEPD_BLACK);

 tela.display(true);
}

// Desenhos
tela.drawLine(x1, y1, x2, y2, cor);
tela.fillCircle(x, y, raio, cor);
tela.drawCircle(x, y, raio, cor);
tela.fillRect(x, y, comprimento, altura, cor);
tela.drawRect(x, y, comprimento, altura, cor);
tela.fillTriangle(x1, y1, x2, y2, x3, y3, cor);
tela.drawTriangle(x1, y1, x2, y2, x3, y3, cor);

// Textos
fontes.setFont(u8g2_font_helvB24_te);
fontes.setFontMode(1);
fontes.setCursor(x, y);
fontes.print("Meu texto");

// Atualiza a tela (SEMPRE CHAMAR NO FINAL!)
tela.display(true);
```

### Barcode e QR Code

Geração e desenho de códigos no display ePaper e leitura via serial.

```cpp
// Bibliotecas adicionais
#include <BarcodeGFX.h>
#include <QRCodeGFX.h>

// Objetos (além dos de ePaper)
BarcodeGFX codigoBarras(tela);
QRCodeGFX qrcode(tela);

// Desenho do Código de Barras
codigoBarras.setScale(2);
codigoBarras.draw("7896065880069", x, y, altura);
tela.display(true);

// Desenho do QR Code
qrcode.setScale(2);
qrcode.draw("https://google.com", x, y);
tela.display(true);

// --- Setup Leitor ---
void setup() {
 // Serial para debug
 Serial.begin(115200); delay(500);
 // Serial1 (UART) para o leitor nos pinos 47 e 48
 Serial1.begin(9600, SERIAL_8N1, 47, 48);

 // Envia comandos de configuração para o leitor
 Serial1.println("~M00910001.");
 delay(100);
 Serial1.println("~M00210001.");
 delay(100);
 Serial1.println("~M00B00014.");
}

// --- Loop Leitor (Resposta) ---
void loop() {
 if (Serial1.available() > 0) {
 String texto = Serial1.readStringUntil('\n'); // Cuidado: 'Seriall' no doc original
 texto.trim();

 if (texto.length() > 5) {
 Serial.println("Resposta do leitor: " + texto);
 long long code = strtoll(texto.c_str(), nullptr, 10);
 }
 }
}
```

-----

## 7\. Node-RED

Fluxos de processamento de dados.

 * **Nós (Nodes):**
 * `inject`: Inicia um fluxo (ex: com `payload: "texto"`).
 * `debug` (depurar): Exibe o `msg.payload` ou o objeto `msg` completo.
 * `mqtt in/out`: Subscreve e publica em tópicos MQTT.
 * `change`: Modifica propriedades da mensagem (ex: `msg.payload`).
 * `switch`: Roteia mensagens com base em condições (ex: `payload < 5`).
 * `split`: Divide um array em mensagens individuais (ex: `[10, 20]` -\> `10`, `20`).
 * `join`: Agrupa mensagens de volta em um array.
 * `template`: Formata uma string usando o modelo "Mustache" (ex: `"Valor = {{payload}}"`).
 * **Integração Telegram:**
 * Usa nós `sender` e `receiver`.
 * A mensagem é formatada como JSON, definindo `chatId` e `type`.

-----

## 8\. Banco de Dados e Visualização

### PostgreSQL

Comandos SQL para inserção e busca de dados.

 * **Inserção de Dados**
 ```sql
 INSERT INTO provas (id, nome, inicio) VALUES
 (1, 'P1 de Programação', '2024-04-22 17:00:00'),
 (2, 'P2 de Programação', '2024-05-17 17:00:00');
 ```
 * **Busca de Dados**
 ```sql
 SELECT * FROM questoes;

 SELECT numero, pontos, enunciado
 FROM questoes
 WHERE pontos >= 2.0 AND id = 1
 ORDER BY id_prova ASC, numero ASC;
 ```
 * **Integração Node-RED**
 * Parâmetros são passados via `msg.queryParameters` (ou `msg.payload` ).
 * A query usa placeholders, como `$minimo`.```sql
 SELECT *
 FROM questoes
 WHERE pontos >= $minimo;
 ```

### Timescale (Banco de Dados Time-Series)

Consultas SQL otimizadas para janelamento de tempo.

 * **Busca com Janelamento**
 Agrega dados em intervalos de 1 hora.
 ```sql
 SELECT
 time_bucket('1 hour', data_hora) AS time,
 AVG(luz) AS media_luz,
 SUM(movimento) AS soma_movimento
 FROM dados
 WHERE data_hora > NOW() - INTERVAL '1 day'
 GROUP BY time
 ORDER BY time ASC;
 ```
 * **Outras Funções de Janelamento**
 * `first(coluna, tempo)`: Pega o primeiro valor da janela.
 * `last(coluna, tempo)`: Pega o último valor da janela.
 * `time_bucket_gapfill(...)`: Preenche intervalos de tempo vazios.

### Grafana (Visualização)

Integração de consultas com o dashboard do Grafana.

 * **Filtro de Tempo do Grafana**
 Usa a variável global `$_timeFilter` para aplicar o seletor de tempo do dashboard.
 ```sql
 SELECT
 time_bucket('1 minute', data_hora) AS time,
 AVG(luz) AS media_luz
 FROM dados
 WHERE $_timeFilter(data_hora)
 GROUP BY time
 ORDER BY time ASC;
 ```
 * **Variáveis de Template**
 Permite criar filtros dinâmicos no Grafana.
 * Define uma variável (ex: `Name`: `nomeCliente` , `Label`: `Nome do Cliente` , `Type`: `Text box` ).
 * Usa a variável na query SQL com a sintaxe `'$nomeVariavel'`.```sql
 SELECT saldo
 FROM clientes
 WHERE nome = '$nomeCliente'
 ```