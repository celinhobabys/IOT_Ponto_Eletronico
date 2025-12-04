# 📋 META 1 - GUIA DE TESTE

## PONTO ELETRÔNICO - RFID + ePaper + Encoder

**Data**: 03/11/2025  
**Objetivo**: Testar integração RFID, display ePaper e encoder rotativo

---

## 🎯 O QUE O CÓDIGO FAZ

### Funcionalidades Implementadas

1. **Leitura de RFID**
   - Detecta aproximação de tags/cartões RFID
   - Lê o UID (identificador único) da tag
   - Formata o UID no padrão: "AB CD EF 12" (hexadecimal com espaços)

2. **Display ePaper - 6 Telas**
   - **Tela Inicial**: "Aproxime sua TAG/Cartão" + status de conexão
   - **Verificando**: Mensagem de aguarde durante validação
   - **Boas-vindas**: Mostra nome do funcionário + instruções do encoder
   - **Confirmação**: Exibe tipo de registro (ENTRADA/SAÍDA) e horário
   - **Acesso Negado**: Mensagem de erro para tags não cadastradas
   - **Erro de Conexão**: Falha de comunicação MQTT

3. **Encoder Rotativo - Detecção de Direção**
   - **Girar para DIREITA** (sentido horário) = **ENTRADA**
   - **Girar para ESQUERDA** (sentido anti-horário) = **SAÍDA**
   - Timeout de 10 segundos se não girar

4. **Máquina de Estados**
   ```mermaid
   stateDiagram-v2
       [*] --> AGUARDANDO
       AGUARDANDO --> VALIDANDO
       VALIDANDO --> AGUARDA_DIRECAO
       VALIDANDO --> NEGADO
       VALIDANDO --> ERRO_CONEXAO
       AGUARDA_DIRECAO --> CONFIRMADO
       CONFIRMADO --> AGUARDANDO
       NEGADO --> AGUARDANDO
       ERRO_CONEXAO --> AGUARDANDO
   ```

5. **Dois Modos de Operação**
   - **MODO OFFLINE**: Valida localmente com TAG mestre (sem WiFi/MQTT)
   - **MODO ONLINE**: Valida via MQTT com servidor Node-RED

---

## ⚙️ CONFIGURAÇÃO ANTES DE TESTAR

### 1. Edite o arquivo `main.cpp`

Localize a seção **CONFIGURAÇÕES - AJUSTE AQUI!** (linha ~30) e modifique:

```cpp
// WiFi (apenas se testar modo online)
const char* WIFI_SSID = "Nome_Da_Sua_Rede";
const char* WIFI_PASSWORD = "Senha_Do_WiFi";

// MQTT (apenas se testar modo online)
const char* MQTT_BROKER = "mqtt.janks.dev.br";
const char* MQTT_USER = "seu_usuario";
const char* MQTT_PASSWORD = "sua_senha";

// TAG MESTRE PARA TESTE OFFLINE
const char* TAG_MESTRE = "AB CD EF 12";  // <-- TROCAR!
```

### 2. Descubra o UID da sua TAG RFID

**Opção A - Usando este código:**
1. Compile e suba o código atual
2. Aproxime sua TAG do leitor
3. Veja no **Serial Monitor** a linha: `UID: XX YY ZZ WW`
4. Copie exatamente como aparece (com espaços)
5. Cole em `TAG_MESTRE`

**Opção B - Usando exemplo básico:**
1. Abra: `Arquivo → Exemplos → MFRC522 → DumpInfo`
2. Suba para o ESP32
3. Aproxime a TAG
4. Copie o UID mostrado

### 3. Verifique a Pinagem

Confirme se os pinos no código batem com sua montagem física:

| Componente | Pino | Conectado ao ESP32 |
|------------|------|--------------------|
| **RFID MFRC522** | | |
| SDA (SS) | 46 | GPIO 46 |
| RST | 17 | GPIO 17 |
| SCK | SCK | Padrão SPI |
| MOSI | MOSI | Padrão SPI |
| MISO | MISO | Padrão SPI |
| **ePaper 2.9"** | | |
| SS | 10 | GPIO 10 |
| DC | 14 | GPIO 14 |
| RST | 15 | GPIO 15 |
| BUSY | 16 | GPIO 16 |
| **Encoder** | | |
| CLK | 34 | GPIO 34 |
| DT | 35 | GPIO 35 |

⚠️ **Se seus pinos forem diferentes, edite as defines no código!**

---

## 📝 BIBLIOTECAS NECESSÁRIAS

Instale via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

1. **MFRC522** (by GithubCommunity) - para RFID
2. **GxEPD2** (by Jean-Marc Zingg) - para ePaper
3. **U8g2_for_Adafruit_GFX** (by olikraus) - para fontes
4. **ESP32Encoder** (by Kevin Harrington) - para encoder
5. **MQTT** (by Joel Gaehwiler) - para comunicação
6. **ArduinoJson** (by Benoit Blanchon) - para JSON

---

## 🧪 ROTEIRO DE TESTE

### TESTE 1: Modo Offline (Sem WiFi/MQTT)

**Objetivo**: Testar hardware básico sem rede

1. **Deixe WiFi/MQTT com credenciais inválidas** (para forçar modo offline)
2. **Configure TAG_MESTRE** com o UID da sua tag
3. **Compile e suba** o código
4. **Abra Serial Monitor** (115200 baud)

**Resultado esperado:**
```
=================================
PONTO ELETRONICO - META 1
=================================

[OK] Leitor RFID iniciado
[OK] Display ePaper iniciado
[OK] Encoder iniciado

[INFO] Tentando conectar ao WiFi...
Conectando ao WiFi........ falhou!

[AVISO] Operando em MODO OFFLINE
TAG mestre configurada: XX YY ZZ WW

=================================
Sistema pronto! Aproxime uma TAG.
=================================
```

5. **Observe o display ePaper**: deve mostrar "PONTO ELETRÔNICO" e "Modo: OFFLINE"

#### Passo a Passo:

**A) Teste TAG Válida + Encoder ENTRADA**
1. Aproxime a TAG configurada como mestre
2. Serial mostra: `UID: XX YY ZZ WW` e `TAG VALIDA (offline)`
3. Display mostra: "Bem-vindo(a), Usuario Teste" e instruções do encoder
4. **Gire o encoder para DIREITA** (sentido horário)
5. Serial mostra: `Encoder girou para DIREITA -> ENTRADA`
6. Display mostra: "REGISTRADO! Tipo: ENTRADA"
7. Após 3 segundos, volta à tela inicial

**B) Teste TAG Válida + Encoder SAÍDA**
1. Aproxime a TAG novamente
2. Display mostra boas-vindas
3. **Gire o encoder para ESQUERDA** (sentido anti-horário)
4. Serial mostra: `Encoder girou para ESQUERDA -> SAIDA`
5. Display mostra: "REGISTRADO! Tipo: SAIDA"

**C) Teste TAG Inválida**
1. Aproxime uma TAG diferente (ou crie uma falsa)
2. Serial mostra: `TAG INVALIDA (offline)`
3. Display mostra: "ACESSO NEGADO"
4. Após 3 segundos, volta à tela inicial

**D) Teste Timeout do Encoder**
1. Aproxime a TAG válida
2. Display mostra boas-vindas
3. **NÃO gire o encoder** por 10 segundos
4. Serial mostra: `TIMEOUT aguardando encoder`
5. Display volta à tela inicial

---

### TESTE 2: Modo Online (Com WiFi/MQTT)

**Objetivo**: Testar comunicação com servidor

⚠️ **Pré-requisito**: Node-RED configurado com fluxos de validação

1. **Configure WiFi e MQTT** com credenciais válidas
2. **Compile e suba** o código
3. **Abra Serial Monitor**

**Resultado esperado:**
```
[OK] Leitor RFID iniciado
[OK] Display ePaper iniciado
[OK] Encoder iniciado

[INFO] Tentando conectar ao WiFi...
Conectando ao WiFi.. conectado!
IP: 192.168.1.100

[INFO] Tentando conectar ao MQTT...
Conectando MQTT conectado!

[OK] Operando em MODO ONLINE

=================================
Sistema pronto! Aproxime uma TAG.
=================================
```

4. Display mostra: "Modo: ONLINE"

#### Passo a Passo:

**A) Teste Validação via MQTT**
1. Aproxime uma TAG
2. Serial mostra:
   ```
   --- TAG DETECTADA ---
   UID: XX YY ZZ WW
   Enviando validacao via MQTT...
   MQTT >> ponto/rfid/validacao: {"uid":"XX YY ZZ WW"}
   ```
3. Aguarda resposta do Node-RED
4. Serial mostra:
   ```
   MQTT << ponto/rfid/resposta: {"valido":true,"nome":"João Silva","id":123}
   Validação recebida: VÁLIDO
   ```
5. Display mostra: "Bem-vindo(a), João Silva"

**B) Teste Registro via MQTT**
1. Após validação, gire o encoder
2. Serial mostra:
   ```
   Encoder girou para DIREITA -> ENTRADA
   MQTT >> ponto/registro: {"funcionario_id":123,"tipo":"ENTRADA","timestamp":"12345","metodo":"rfid"}
   ```
3. Display mostra confirmação

---

## 🐛 TROUBLESHOOTING

### Problema: Display ePaper não atualiza

**Possíveis causas:**
- Pinagem errada (verifique SS, DC, RST, BUSY)
- Alimentação insuficiente
- Cabo flat mal conectado

**Solução:**
1. Confira os pinos no código vs hardware
2. Meça tensão: deve ter 3.3V estável
3. Teste com exemplo da biblioteca GxEPD2

---

### Problema: RFID não detecta tags

**Possíveis causas:**
- Pinagem errada (SS ou RST)
- TAG muito longe (< 2cm ideal)
- SPI não inicializado

**Solução:**
1. Confira pinos 46 (SS) e 17 (RST)
2. Aproxime TAG bem próxima do leitor
3. Adicione `delay(100)` antes de `rfid.PICC_IsNewCardPresent()`
4. Teste com exemplo DumpInfo da biblioteca

---

### Problema: Encoder não detecta rotação

**Possíveis causas:**
- Pinagem invertida (CLK e DT trocados)
- Encoder defeituoso
- Pull-ups não ativados

**Solução:**
1. Troque CLK e DT no código
2. Teste encoder em circuito isolado
3. Verifique se linha `ESP32Encoder::useInternalWeakPullResistors = UP;` está presente

---

### Problema: WiFi não conecta

**Solução:**
1. Verifique SSID e senha (case-sensitive!)
2. Confirme que rede é 2.4GHz (ESP32 não suporta 5GHz)
3. Aproxime ESP32 do roteador
4. Teste primeiro sem MQTT

---

### Problema: MQTT não conecta

**Solução:**
1. Confirme que WiFi está conectado
2. Verifique usuário e senha MQTT
3. Teste broker com cliente externo (MQTT Explorer, mosquitto_pub)
4. Confirme porta (1883 para insegura, 8883 para TLS)

---

## 📊 MONITORAMENTO SERIAL

Durante os testes, o Serial Monitor mostrará:

```
=================================
PONTO ELETRONICO - META 1
=================================

[OK] Leitor RFID iniciado
[OK] Display ePaper iniciado
[OK] Encoder iniciado

[AVISO] Operando em MODO OFFLINE

=================================
Sistema pronto! Aproxime uma TAG.
=================================

--- TAG DETECTADA ---
UID: 04 A2 3C 12
Validando localmente...
TAG VALIDA (offline)
Tela: BOAS-VINDAS - Usuario Teste

Encoder girou para DIREITA -> ENTRADA
REGISTRO OFFLINE: Usuario Teste - ENTRADA - 45678
Tela: CONFIRMACAO - ENTRADA
Tela: INICIAL
```

---

## ✅ CHECKLIST DE VALIDAÇÃO

Marque os testes concluídos:

### Hardware
- [ ] RFID detecta tags (UID aparece no Serial)
- [ ] Display ePaper atualiza corretamente
- [ ] Encoder detecta rotação para direita (ENTRADA)
- [ ] Encoder detecta rotação para esquerda (SAÍDA)

### Fluxo Completo (Offline)
- [ ] TAG válida → Mostra boas-vindas
- [ ] TAG inválida → Mostra acesso negado
- [ ] Encoder DIREITA → Registra ENTRADA
- [ ] Encoder ESQUERDA → Registra SAÍDA
- [ ] Timeout encoder → Volta ao início
- [ ] Confirmação exibida por 3 segundos

### Fluxo Completo (Online)
- [ ] WiFi conecta
- [ ] MQTT conecta
- [ ] Envia validação via MQTT
- [ ] Recebe resposta do servidor
- [ ] Envia registro via MQTT
- [ ] Display mostra "Modo: ONLINE"

---

## 🎓 DICAS PARA A AULA

1. **Prepare antes:**
   - Configure TAG_MESTRE com UID real
   - Teste em casa antes da aula
   - Leve cabo USB extra

2. **Se algo falhar:**
   - Foque no modo OFFLINE primeiro
   - Use Serial Monitor para debug
   - Documente erros para corrigir depois

3. **Demonstração sugerida:**
   - Mostre tela inicial
   - Aproxime TAG válida → boas-vindas
   - Gire encoder → confirmação
   - Aproxime TAG inválida → negado
   - Explique estados no Serial Monitor

4. **Perguntas esperadas:**
   - "Como cadastrar novos usuários?" → Resposta: Node-RED + Banco (Meta 2)
   - "E se não tiver internet?" → Resposta: Modo offline funciona
   - "Onde ficam os dados?" → Resposta: Serial agora, banco depois

---

## 📚 PRÓXIMOS PASSOS (Depois da META 1)

Após validar esta meta, implementar:

1. **META 2**: Integração com Node-RED
   - Fluxo de validação no servidor
   - Cadastro de usuários no PostgreSQL
   - Registro de pontos no banco

2. **META 3**: Site administrativo
   - Interface web para CRUD de funcionários
   - Visualização de registros
   - Dashboard com Grafana

3. **META 4**: Biometria
   - Adicionar sensor de digital
   - Integrar com validação atual

---

## 🆘 SUPORTE

**Problemas durante o teste?**

1. Anote mensagens de erro do Serial Monitor
2. Tire foto do display ePaper
3. Documente qual teste falhou
4. Verifique pinagem 3x antes de pedir ajuda

**Arquivos importantes:**
- `main.cpp` - Código principal
- `projeto.md` - Documentação completa do projeto
- `resumo.md` - Referências técnicas

---

**Boa sorte nos testes! 🚀**
