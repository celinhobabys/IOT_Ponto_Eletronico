# ✨ Nova Funcionalidade: Cadastro de Funcionários

## 🎉 O Que Foi Adicionado

### 1. Novo Grupo no Node-RED Flow
**Grupo 4: "Cadastro via MQTT (ESP32)"**

Permite cadastrar novas TAGs diretamente no dispositivo ESP32, sem precisar acessar sistema web.

#### Fluxo de Dados:
```
ESP32 aproxima TAG
    ↓
MQTT: ponto/cadastro/registrar {"uid":"..."}
    ↓
Node-RED verifica se TAG existe
    ↓
    ├─ Existe? → Retorna erro
    └─ Não existe? → Cadastra no banco
         ↓
    Retorna confirmação
         ↓
ESP32 exibe resultado
```

---

## 🔧 Arquivos Modificados

### 1. `fluxo_nodered.json` ✅
- ✅ Adicionado Grupo 4 (Cadastro via MQTT)
- ✅ 7 novos nós:
  - `n4_1`: MQTT In (recebe UID)
  - `n4_2`: JSON Parse
  - `n4_3`: Verifica se TAG existe
  - `n4_4`: Busca no PostgreSQL
  - `n4_5`: Switch (existe/não existe)
  - `n4_6`: Cadastra TAG nova
  - `n4_7`: Retorna erro (TAG duplicada)
  - `n4_8`: Salva no banco
  - `n4_9`: MQTT Out (resposta)
  - `n4_10`: Formata resposta sucesso
  - `n4_11`: Debug log

### 2. `src/main.cpp` ✅
- ✅ Adicionado `TOPIC_CADASTRO_RESPOSTA`
- ✅ Subscribe no novo tópico
- ✅ Handler para resposta de cadastro
- ✅ Nova função `mostrarTagJaCadastrada()`
- ✅ Lógica completa no estado `MODO_CADASTRO`

### 3. Documentação Nova ✅
- ✅ `CADASTRO_FUNCIONARIOS.md` - Documentação técnica completa
- ✅ `GUIA_CADASTRO.md` - Guia prático de uso

---

## 📋 Tópicos MQTT

| Tópico | Direção | QoS | Descrição |
|--------|---------|-----|-----------|
| `ponto/cadastro/iniciar` | Admin → ESP32 | 0 | Ativa modo cadastro |
| `ponto/cadastro/registrar` | ESP32 → Node-RED | 2 | Envia UID para cadastrar |
| `ponto/cadastro/resposta` | Node-RED → ESP32 | 1 | Confirmação/erro de cadastro |

---

## 🎯 Casos de Uso

### Caso 1: Funcionário Novo Chegou
```
1. Admin ativa modo cadastro via MQTT
2. Funcionário aproxima TAG no ESP32
3. Sistema cadastra automaticamente
4. Funcionário já pode usar o ponto
5. RH completa dados depois via web
```

### Caso 2: TAG Extra/Reserva
```
1. Empresa comprou novas TAGs
2. Admin ativa modo cadastro
3. Aproxima cada TAG
4. Sistema valida e cadastra
5. TAGs ficam disponíveis para vincular depois
```

### Caso 3: Reposição de TAG Perdida
```
1. Funcionário perdeu TAG antiga
2. Admin dá nova TAG
3. Ativa modo cadastro no ESP32
4. Cadastra nova TAG
5. Vincula ao funcionário existente (via web)
```

---

## 🔄 Fluxo Completo de Uso

### Preparação (Uma Vez)
```bash
# 1. Containers rodando
docker-compose up -d

# 2. Banco configurado
./setup_database.sh

# 3. Node-RED com fluxo importado
# Acesse http://localhost:1880
# Importe fluxo_nodered.json
# Configure PostgreSQL e MQTT
# Deploy
```

### Uso Diário

**Via Web (Cadastro Completo):**
```bash
curl -X POST http://localhost:1880/api/funcionario \
  -H "Content-Type: application/json" \
  -d '{
    "nome": "Ana Silva",
    "cpf": "12345678901",
    "cargo": "Analista",
    "departamento": "TI",
    "rfid_uid": "AA:BB:CC:DD"
  }'
```

**Via ESP32 (Cadastro Rápido):**
```bash
# 1. Ativar modo
mosquitto_pub -h mqtt.janks.dev.br \
  -t "ponto/cadastro/iniciar" \
  -u USER -P PASS \
  -m "true"

# 2. Aproximar TAG (automático)
# ESP32 detecta e envia automaticamente

# 3. Completar dados depois (via web)
# Editar funcionário via interface administrativa
```

---

## 📊 Banco de Dados

### Registro Criado Automaticamente

Quando TAG é cadastrada via MQTT:

```sql
INSERT INTO funcionarios (nome, rfid_uid, cargo, ativo)
VALUES (
  'Funcionário AB:CD:EF',  -- Nome temporário
  'AB:CD:EF:12',           -- UID da TAG
  'A definir',             -- Cargo
  true                      -- Ativo
);
```

### Campos Opcionais (Preenchidos Depois)
- `cpf` → NULL (adicionar via web)
- `departamento` → NULL (adicionar via web)
- `digital_id` → NULL (biometria futura)

---

## ✅ Validações Implementadas

### 1. TAG Duplicada
```javascript
// Node-RED verifica antes de inserir
SELECT * FROM funcionarios WHERE rfid_uid = $1

// Se já existe:
{
  "sucesso": false,
  "mensagem": "TAG já cadastrada",
  "funcionario": "João Silva"
}
```

### 2. Formato UID
- UID deve estar no formato: `AA:BB:CC:DD`
- Case insensitive (convertido para uppercase)
- Armazenado com `:` separadores

### 3. Constraint do Banco
```sql
-- Garante UID único
ALTER TABLE funcionarios 
ADD CONSTRAINT uk_rfid_uid UNIQUE (rfid_uid);
```

---

## 🎨 Interface Visual (ESP32)

### Estados da Tela

**1. Normal → Modo Cadastro**
```
PONTO ELETRONICO          MODO DE CADASTRO
Aproxime sua TAG     →    Aproxime a nova TAG
Modo: ONLINE
```

**2. TAG Nova → Confirmação**
```
MODO DE CADASTRO          TAG CADASTRADA!
Aproxime a nova TAG  →    UID: AB:CD:EF:12
```

**3. TAG Existente → Erro**
```
MODO DE CADASTRO          TAG JA CADASTRADA!
Aproxime a nova TAG  →    Funcionario:
                          João Silva
```

**4. Volta Automático (3s)**
```
TAG CADASTRADA!           PONTO ELETRONICO
UID: AB:CD:EF:12     →    Aproxime sua TAG
                          Modo: ONLINE
```

---

## 🔐 Segurança

### Implementado:
- ✅ Validação de TAG duplicada
- ✅ Constraint UNIQUE no banco
- ✅ QoS adequados (2 para cadastro, 1 para resposta)
- ✅ Timeout automático (estado volta após 3s)

### A Implementar:
- [ ] Senha para ativar modo cadastro
- [ ] Timeout de inatividade (2 minutos)
- [ ] Limite de cadastros por sessão
- [ ] Log de auditoria (quem cadastrou, quando)
- [ ] Confirmação de admin via segundo canal

---

## 🧪 Testes Realizados

### Teste 1: Cadastro TAG Nova ✅
```bash
mosquitto_pub -t "ponto/cadastro/registrar" \
  -m '{"uid":"FF:EE:DD:CC"}'

# Resultado: Cadastrada com sucesso
# Banco: 1 nova linha em funcionarios
```

### Teste 2: TAG Duplicada ✅
```bash
mosquitto_pub -t "ponto/cadastro/registrar" \
  -m '{"uid":"C1:71:22:0D"}'  # TAG do João

# Resultado: Erro "TAG já cadastrada"
# Banco: Nenhuma alteração
```

### Teste 3: Fluxo Completo ✅
```
1. Ativar modo → ESP32 exibe "MODO DE CADASTRO"
2. Aproximar TAG → ESP32 envia MQTT
3. Node-RED processa → Verifica no banco
4. Resposta → ESP32 exibe resultado
5. 3 segundos → Volta à tela inicial
```

---

## 📈 Estatísticas

### Consultas Úteis

**Cadastros de hoje:**
```sql
SELECT COUNT(*) FROM funcionarios 
WHERE DATE(data_cadastro) = CURRENT_DATE;
```

**Últimas 10 TAGs cadastradas:**
```sql
SELECT nome, rfid_uid, 
       TO_CHAR(data_cadastro, 'DD/MM HH24:MI') as quando
FROM funcionarios 
ORDER BY data_cadastro DESC 
LIMIT 10;
```

**TAGs pendentes de completar dados:**
```sql
SELECT id, nome, rfid_uid 
FROM funcionarios 
WHERE cargo = 'A definir' 
   OR cpf IS NULL;
```

---

## 🚀 Próximos Passos

### Features Sugeridas:

1. **Editar Funcionário (API)**
   ```
   PUT /api/funcionario/:id
   {
     "nome": "Nome Completo",
     "cpf": "12345678901",
     "cargo": "Cargo Real",
     "departamento": "Departamento"
   }
   ```

2. **Listar Funcionários (API)**
   ```
   GET /api/funcionarios
   GET /api/funcionario/:id
   GET /api/funcionario?cpf=12345678901
   ```

3. **Desativar/Deletar (API)**
   ```
   PATCH /api/funcionario/:id/desativar
   DELETE /api/funcionario/:id
   ```

4. **Interface Web**
   - Dashboard administrativo
   - Tabela de funcionários
   - Formulário de edição
   - Histórico de cadastros

5. **Melhorias ESP32**
   - Senha para modo cadastro
   - Timeout automático
   - Contador de cadastros na sessão
   - Feedback sonoro (buzzer)

---

## 📚 Documentação

| Arquivo | Descrição |
|---------|-----------|
| `CADASTRO_FUNCIONARIOS.md` | Documentação técnica detalhada |
| `GUIA_CADASTRO.md` | Guia prático com exemplos |
| `fluxo_nodered.json` | Flow completo (Grupo 4) |
| `src/main.cpp` | Firmware ESP32 atualizado |

---

## 🎓 Aprendizados

### Conceitos Aplicados:
- ✅ MQTT bidirecionai (publish + subscribe)
- ✅ State machine (estados do ESP32)
- ✅ PostgreSQL transactions
- ✅ Node-RED flow context (`flow.set/get`)
- ✅ JSON serialization/deserialization
- ✅ Error handling
- ✅ Database constraints

---

## ✨ Resultado Final

### Antes:
- ❌ Cadastro manual via SQL
- ❌ UID da TAG anotado à mão
- ❌ Erro humano possível
- ❌ Processo demorado

### Depois:
- ✅ Cadastro automático via ESP32
- ✅ UID capturado automaticamente
- ✅ Validação em tempo real
- ✅ Processo instantâneo
- ✅ Backup via web (API)

---

**Sistema de cadastro completo e testado! 🎉**

*Última atualização: 10/11/2025*
