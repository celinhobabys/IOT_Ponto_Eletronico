# 🎯 Ponto Eletrônico IoT - Status do Projeto

## ✅ Concluído e Funcionando

### 🐳 Docker Setup
- ✅ **Node-RED** rodando em `localhost:1880`
- ✅ **PostgreSQL 16** rodando internamente
- ✅ Pacote `node-red-contrib-postgresql` instalado
- ✅ Volumes persistentes configurados
- ✅ Rede interna funcionando

### 🗄️ Banco de Dados
- ✅ Tabelas criadas (`funcionarios`, `registros_ponto`)
- ✅ Índices otimizados
- ✅ Views para relatórios
- ✅ Funções úteis (calcular horas, verificar presença)
- ✅ Dados de teste inseridos (3 funcionários, 4 registros)

### 🔄 Node-RED Flows
- ✅ Arquivo `fluxo_nodered.json` corrigido
- ✅ Nós PostgreSQL configurados corretamente
- ✅ 3 grupos de fluxos:
  - Validação de Acesso (RFID)
  - Registro de Ponto
  - Cadastro de Funcionário (API)

### 📟 ESP32 Firmware
- ✅ Código `main.cpp` completo
- ✅ Máquina de estados implementada
- ✅ Suporte RFID + ePaper + Encoder
- ✅ Modo Online (MQTT) e Offline
- ✅ Integração com Node-RED

---

## 📁 Arquivos do Projeto

```
projeto-iot/
├── src/
│   └── main.cpp                    # ✅ Firmware ESP32
├── docker-compose.yml              # ✅ Configuração Docker
├── Dockerfile                      # ✅ Imagem Node-RED customizada
├── fluxo_nodered.json             # ✅ Flows Node-RED (CORRIGIDO)
├── init_database.sql              # ✅ Script de inicialização DB
├── setup_database.sh              # ✅ Script de setup automatizado
├── NODERED_POSTGRES_SETUP.md      # ✅ Guia Docker/PostgreSQL
├── NODERED_FLOW_SETUP.md          # ✅ Guia Node-RED Flows
├── DOCUMENTACAO.md                # ✅ Documentação técnica
├── projeto.md                     # ✅ Planejamento do projeto
└── resumo.md                      # ✅ Referências técnicas
```

---

## 🚀 Como Usar (Quick Start)

### 1️⃣ Iniciar Docker
```bash
cd /home/eduardo/projeto-iot
docker-compose up -d
```

### 2️⃣ Configurar Banco de Dados
```bash
./setup_database.sh
```

### 3️⃣ Importar Fluxo Node-RED
1. Acesse: http://localhost:1880
2. Menu (☰) → Import
3. Selecione: `fluxo_nodered.json`
4. Configure PostgreSQL e MQTT
5. Deploy

### 4️⃣ Testar
```bash
# Acessar PostgreSQL
docker exec -it nodered_postgres psql -U nodered -d nodered_db

# Ver funcionários
SELECT * FROM funcionarios;

# Ver registros
SELECT * FROM registros_ponto;
```

### 5️⃣ Programar ESP32
1. Abra `src/main.cpp` no PlatformIO
2. Ajuste credenciais WiFi e MQTT
3. Upload para o ESP32
4. Aproxime a TAG `C1:71:22:0D` (João Silva)

---

## 🔌 Conexões

### MQTT Tópicos
| Tópico | Direção | Descrição |
|--------|---------|-----------|
| `ponto/rfid/validacao` | ESP32 → Node-RED | Solicita validação de UID |
| `ponto/rfid/resposta` | Node-RED → ESP32 | Retorna dados do funcionário |
| `ponto/registro` | ESP32 → Node-RED | Envia registro de ponto |
| `ponto/cadastro/iniciar` | Node-RED → ESP32 | Ativa modo cadastro |
| `ponto/cadastro/registrar` | ESP32 → Node-RED | Envia nova TAG |

### APIs HTTP
| Endpoint | Método | Descrição |
|----------|--------|-----------|
| `/api/funcionario` | POST | Cadastra novo funcionário |

---

## 📊 Dados de Teste

### Funcionários Cadastrados:
| ID | Nome | Cargo | RFID UID |
|----|------|-------|----------|
| 1 | João Silva | Desenvolvedor | C1:71:22:0D |
| 2 | Maria Santos | Designer | A2:B3:C4:D5 |
| 3 | Pedro Costa | Gerente | E6:F7:G8:H9 |

### TAG Master (Modo Offline):
- **UID**: `C1:71:22:0D`
- **Funcionário**: João Silva

---

## 🎨 Interface do Sistema

### ePaper Display (ESP32)
```
╔═══════════════════════════════╗
║   PONTO ELETRONICO           ║
║                               ║
║   Aproxime sua TAG/Cartao    ║
║                               ║
║   Modo: ONLINE                ║
╚═══════════════════════════════╝
```

↓ *TAG detectada*

```
╔═══════════════════════════════╗
║   Bem-vindo(a),               ║
║   João Silva                  ║
║                               ║
║   Gire a catraca:             ║
║   -> Direita = ENTRADA        ║
╚═══════════════════════════════╝
```

↓ *Encoder girado*

```
╔═══════════════════════════════╗
║   REGISTRADO!                 ║
║                               ║
║   Tipo: ENTRADA               ║
║   Horario: 1699632145         ║
║                               ║
╚═══════════════════════════════╝
```

---

## 🔍 Monitoramento

### Logs em Tempo Real
```bash
# Node-RED
docker logs nodered_app -f

# PostgreSQL
docker logs nodered_postgres -f

# ESP32 (via Serial)
pio device monitor
```

### Verificar Status
```bash
# Containers
docker-compose ps

# Conexões
docker exec nodered_app ping postgres
docker exec nodered_app ping mqtt.janks.dev.br
```

---

## 🛠️ Troubleshooting

### ❌ Node-RED: Nós PostgreSQL aparecem como "unknown"
**✅ Solução:** Já corrigido! Reimporte `fluxo_nodered.json`

### ❌ PostgreSQL: Conexão recusada
**✅ Verificar:**
- Host = `postgres` (não `localhost`)
- Senha = igual ao `docker-compose.yml`
- Porta = `5432`

### ❌ MQTT: Não conecta
**✅ Verificar:**
- Credenciais corretas
- Broker acessível: `ping mqtt.janks.dev.br`
- Porta 1883 aberta

### ❌ ESP32: TAG não detectada
**✅ Verificar:**
- Leitor RFID conectado corretamente
- Pinos: SS=46, RST=17
- Serial: Mensagem de inicialização

---

## 📈 Próximas Etapas

### Funcionalidades Pendentes:
- [ ] Modo de cadastro de novas TAGs via MQTT
- [ ] Interface web administrativa
- [ ] Dashboard Grafana
- [ ] Sensor biométrico
- [ ] Notificações Telegram
- [ ] Relatórios automáticos
- [ ] Backup automático do banco

### Hardware Pendente:
- [ ] PCB customizada
- [ ] Case 3D
- [ ] Montagem final

---

## 📚 Documentação

| Arquivo | Descrição |
|---------|-----------|
| `DOCUMENTACAO.md` | Documentação técnica detalhada do código |
| `projeto.md` | Planejamento completo do projeto |
| `resumo.md` | Referências rápidas (MQTT, PostgreSQL, etc) |
| `NODERED_POSTGRES_SETUP.md` | Setup Docker e PostgreSQL |
| `NODERED_FLOW_SETUP.md` | Setup Node-RED Flows |

---

## 🎓 Tecnologias Utilizadas

### Hardware
- ESP32-S3
- MFRC522 (RFID)
- ePaper 2.9" (GxEPD2)
- Encoder Rotativo

### Software
- **Linguagem**: C++ (Arduino Framework)
- **Build**: PlatformIO
- **Containerização**: Docker + Docker Compose
- **Broker MQTT**: Mosquitto
- **Backend**: Node-RED
- **Database**: PostgreSQL 16
- **Visualização**: Grafana (futuro)

### Bibliotecas ESP32
- WiFi.h
- MQTT.h (lwmqtt)
- MFRC522.h
- GxEPD2_BW.h
- U8g2_for_Adafruit_GFX.h
- RotaryEncoder.h
- ArduinoJson.h

---

## ✨ Destaques do Sistema

### 🎯 Máquina de Estados Robusta
```cpp
enum Estado {
    AGUARDANDO,      // Idle
    VALIDANDO,       // Verificando TAG
    AGUARDA_DIRECAO, // Aguardando encoder
    REGISTRANDO,     // Salvando ponto
    CONFIRMADO,      // Sucesso
    NEGADO,          // Acesso negado
    ERRO_CONEXAO,    // Falha MQTT
    MODO_CADASTRO    // Cadastrar nova TAG
};
```

### 🌐 Modo Híbrido (Online/Offline)
- **Online**: Validação via MQTT + PostgreSQL
- **Offline**: Validação local com TAG_MESTRE
- **Failover**: Automático se perder conexão

### 📊 Views PostgreSQL Otimizadas
- `horas_trabalhadas_diarias` - Relatório de horas
- `ultimo_registro_por_funcionario` - Status atual
- Funções: `funcionario_presente()`, `calcular_horas_periodo()`
