# Troubleshooting - ePaper Display

## Problemas Comuns

### 1. Tela não atualiza / Fica branca

**Possíveis causas:**
- Pinos mal conectados
- Cabo flat mal encaixado
- Alimentação insuficiente
- Modelo de display incorreto no código

**Verificações:**

#### a) Conferir Conexões (ESP32-S3 ↔ ePaper 2.9")
```
ePaper     →  ESP32-S3
-----------------------------
VCC        →  3.3V
GND        →  GND
DIN (MOSI) →  GPIO 11 (SPI MOSI)
CLK (SCK)  →  GPIO 12 (SPI SCK)
CS         →  GPIO 10
DC         →  GPIO 14
RST        →  GPIO 15
BUSY       →  GPIO 16
```

#### b) Verificar Modelo do Display
No código está configurado: `GxEPD2_290_T94_V2`

Se você tiver um modelo diferente, mude a linha:
```cpp
GxEPD2_290_T94_V2 modeloTela(EPAPER_SS, EPAPER_DC, EPAPER_RST, EPAPER_BUSY);
```

**Modelos comuns 2.9":**
- `GxEPD2_290` - Waveshare 2.9" padrão
- `GxEPD2_290_T5` - Waveshare 2.9" (T5)
- `GxEPD2_290_T94` - Waveshare 2.9" (T94)
- `GxEPD2_290_T94_V2` - Waveshare 2.9" (T94 V2)
- `GxEPD2_290_BS` - DEPG0290BS

#### c) Testar com código simples

Se o display não funciona, teste com este código mínimo:

```cpp
#include <GxEPD2_BW.h>

#define EPAPER_SS 10
#define EPAPER_DC 14
#define EPAPER_RST 15
#define EPAPER_BUSY 16

GxEPD2_290_T94_V2 display(EPAPER_SS, EPAPER_DC, EPAPER_RST, EPAPER_BUSY);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Teste ePaper");
  
  SPI.begin();
  display.init(115200);
  display.setRotation(1);
  display.setFullWindow();
  
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 50);
    display.print("TESTE OK!");
  } while (display.nextPage());
  
  Serial.println("Display atualizado!");
}

void loop() {
  delay(1000);
}
```

### 2. Monitor Serial não funciona

Para ver as mensagens de debug:

```bash
# Opção 1: PlatformIO
pio device monitor -p /dev/ttyACM0 -b 115200

# Opção 2: screen
screen /dev/ttyACM0 115200
# Para sair: Ctrl+A depois K e confirmar

# Opção 3: minicom
minicom -D /dev/ttyACM0 -b 115200
```

### 3. ePaper pisca mas não mostra conteúdo

**Causa:** Display está atualizando, mas o conteúdo pode estar sendo desenhado fora da área visível.

**Solução:**
- Verifique a rotação: `tela.setRotation(0)`, `tela.setRotation(1)`, `tela.setRotation(2)` ou `tela.setRotation(3)`
- Verifique as coordenadas do `setCursor()`

### 4. Erro de compilação relacionado ao GxEPD2

Se der erro de "GxEPD2_290_T94_V2 not found":

```bash
# Reinstalar a biblioteca
pio pkg uninstall -e projeto zinggjm/GxEPD2
pio pkg install -e projeto "zinggjm/GxEPD2@^1.6.5"
```

### 5. Display mostra "fantasmas" (imagens anteriores)

**Causa:** ePaper mantém a última imagem sem energia.

**Solução:**
- Use `display.fillScreen(GxEPD_WHITE)` antes de desenhar
- Para limpeza completa, use update parcial várias vezes

## Informações do Hardware

### ESP32-S3 DevKit-C1 - Pinos SPI Padrão
- **MOSI (DIN)**: GPIO 11
- **MISO**: GPIO 13  
- **SCK (CLK)**: GPIO 12
- **CS**: Definido pelo usuário (GPIO 10 no projeto)

### Consumo de Energia
- ePaper 2.9": ~30mA durante atualização
- Em repouso: < 0.01mA
- Certifique-se que a fonte USB fornece pelo menos 500mA

## Logs para Debug

Quando abrir o monitor serial, você deve ver:

```
PONTO ELETRONICO
[INFO] Configurando SPI...
[OK] SPI configurado
[INFO] Inicializando leitor RFID...
[OK] Leitor RFID iniciado
[INFO] Inicializando display ePaper...
  - CS (SS): GPIO 10
  - DC: GPIO 14
  - RST: GPIO 15
  - BUSY: GPIO 16
[OK] Display ePaper iniciado
[INFO] Testando ePaper...
[INFO] Teste concluído. Carregando tela inicial...
[DEBUG] Iniciando atualização da tela inicial...
[DEBUG] Tela inicial atualizada!
=================================
Sistema pronto! Aproxime uma TAG.
=================================
```

Se não ver essas mensagens, o ESP32 pode não estar bootando corretamente.
