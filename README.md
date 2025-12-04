# PROJETO PONTO ELETRÔNICO IoT

O trabalho apresenta um sistema de ponto eletrônico inteligente, suportado por tecnologias de IoT. A solução integra métodos de autenticação via biometria e RFID para validar usuários. O objetivo é entregar um dispositivo totalmente conectado aos sistema geral, que não apenas registre o fluxo de entrada e saída dos colaboradores, mas também disponibilize esses dados para monitoramento e análise.

---

### Modos de Operação

**MODO OFFLINE (sem WiFi/MQTT):**
- Valida com TAG mestre configurada
- Registra apenas no Serial Monitor
- Ideal para testes

**MODO ONLINE (com WiFi/MQTT):**
- Valida com servidor Node-RED
- Envia registros para banco de dados
- Modo de produção

---

## Tecnologias Utilizadas

### Hardware
- **ESP32** - Microcontrolador principal
- **MFRC522** - Leitor RFID 13.56MHz
- **GxEPD2_290_T94_V2** - Display e-Paper 2.9"
- **Encoder Rotativo** - Detecção de direção
- **Buzzer Ativo** - alerta sonoro

### Software
- **Arduino Framework** - Desenvolvimento
- **MQTT** - Comunicação IoT
- **JSON** - Serialização de dados
- **WiFi** - Conectividade
- **Tecnologias Web** - Interface do painel

### Backend
- **Node-RED** - Processamento de fluxos
- **PostgreSQL** - Banco de dados
- **Django** - Backend do painel web

---

## Como Funciona

**Conceitual:**
```mermaid
sequenceDiagram
    participant U as Usuário
    participant R as RFID
    participant E as ESP32
    participant D as Display ePaper
    participant EN as Encoder
    participant S as Servidor MQTT
    
    U->>R: Aproxima TAG
    R->>E: Lê UID
    E->>E: Valida (local)
    alt Modo Online
        E->>S: Envia validação
        S->>E: Responde válido/inválido
    end
    E->>D: Exibe boas-vindas
    U->>EN: Gira encoder
    EN->>E: Detecta direção
    alt DIREITA
        E->>E: Tipo = ENTRADA
    else ESQUERDA
        E->>E: Tipo = SAÍDA
    end
    E->>S: Registra ponto (se online)
    E->>D: Exibe confirmação
    Note over E,D: Aguarda 3 segundos
    E->>D: Volta ao início
```
**Hardware:**

<img width="800" height="801" alt="Schematic_ProjetoIOT_Ponto_2025-12-03" src="https://github.com/user-attachments/assets/d8a42857-aa3e-44b3-8d48-f6d8d5855960" />
