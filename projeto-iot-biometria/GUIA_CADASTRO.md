# 🎯 Guia Rápido: Cadastro de Funcionários

## 🚀 Quick Start

### Cenário 1: Cadastro via Web (Completo)

```bash
# Cadastrar funcionário com todos os dados
curl -X POST http://localhost:1880/api/funcionario \
  -H "Content-Type: application/json" \
  -d '{
    "nome": "Ana Paula Silva",
    "cpf": "12345678901",
    "cargo": "Gerente de Projetos",
    "departamento": "TI",
    "rfid_uid": "A1:B2:C3:D4"
  }'
```

**✅ Vantagens:**
- Todos os dados cadastrados de uma vez
- Ideal para RH/administrativo
- Validação completa

---

### Cenário 2: Cadastro Rápido no Local (ESP32)

**Passo 1:** Ativar modo cadastro
```bash
# Via mosquitto_pub
mosquitto_pub -h mqtt.janks.dev.br \
  -t "ponto/cadastro/iniciar" \
  -u SEU_USUARIO -P SUA_SENHA \
  -m "true"
```

**Passo 2:** Aproximar TAG no ESP32
- ESP32 detecta TAG automaticamente
- Envia para Node-RED
- Cadastra com dados básicos

**Passo 3:** Editar dados depois (via web)
```bash
# TODO: Implementar endpoint PUT
curl -X PUT http://localhost:1880/api/funcionario/4 \
  -H "Content-Type: application/json" \
  -d '{
    "nome": "Ana Paula Silva",
    "cpf": "12345678901",
    "cargo": "Gerente de Projetos",
    "departamento": "TI"
  }'
```

**✅ Vantagens:**
- Cadastro imediato no local
- Não precisa anotar UID manualmente
- Funcionário já pode usar TAG
- Dados completados depois

---

## 📋 Fluxo Completo

### 1️⃣ Preparação (Uma Vez)

```bash
# Iniciar Docker
cd ~/projeto-iot
docker-compose up -d

# Configurar banco
./setup_database.sh

# Importar fluxo Node-RED
# Acesse http://localhost:1880
# Menu → Import → fluxo_nodered.json
```

### 2️⃣ Cadastro Regular (Diário)

**Option A: Via Web**
```javascript
// Exemplo JavaScript (frontend web)
fetch('http://localhost:1880/api/funcionario', {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify({
    nome: 'João Santos',
    cpf: '98765432100',
    cargo: 'Técnico',
    departamento: 'Manutenção',
    rfid_uid: 'FF:EE:DD:CC'
  })
})
.then(res => res.json())
.then(data => console.log('Cadastrado!', data));
```

**Option B: Via ESP32**
```cpp
// No ESP32, admin ativa modo:
// 1. Envia MQTT: ponto/cadastro/iniciar = "true"
// 2. ESP32 entra em MODO_CADASTRO
// 3. Aproxima TAG
// 4. ESP32 envia: ponto/cadastro/registrar
// 5. Node-RED cadastra
// 6. ESP32 recebe confirmação
```

---

## 🎨 Interface ESP32

### Fluxo Visual no ePaper

**1. Estado Normal**
```
┌───────────────────────────┐
│  PONTO ELETRONICO         │
│                           │
│  Aproxime sua TAG/Cartao  │
│                           │
│  Modo: ONLINE             │
└───────────────────────────┘
```

**2. Admin Ativa Modo Cadastro (via MQTT)**
```
mosquitto_pub -t "ponto/cadastro/iniciar" -m "true"
```

↓

```
┌───────────────────────────┐
│  MODO DE CADASTRO         │
│                           │
│  Aproxime a nova TAG      │
└───────────────────────────┘
```

**3. Funcionário Aproxima TAG Nova**

↓

**Caso 1: TAG Nova (Sucesso)**
```
┌───────────────────────────┐
│  TAG CADASTRADA!          │
│                           │
│  UID: AB:CD:EF:12         │
└───────────────────────────┘
```

**Caso 2: TAG Já Existe (Erro)**
```
┌───────────────────────────┐
│  TAG JA CADASTRADA!       │
│                           │
│  Funcionario:             │
│  João Silva               │
└───────────────────────────┘
```

**4. Volta Automático (3 segundos)**
```
┌───────────────────────────┐
│  PONTO ELETRONICO         │
│                           │
│  Aproxime sua TAG/Cartao  │
└───────────────────────────┘
```

---

## 📱 Dashboard Admin (Node-RED)

### Criar Painel de Controle

Adicione ao Node-RED:

```json
[
  {
    "type": "ui_button",
    "name": "Ativar Modo Cadastro",
    "topic": "ponto/cadastro/iniciar",
    "payload": "true"
  },
  {
    "type": "ui_text",
    "name": "Status ESP32",
    "label": "Modo Atual"
  },
  {
    "type": "ui_table",
    "name": "Últimos Cadastros",
    "columns": ["ID", "Nome", "UID", "Data"]
  }
]
```

### Query para Últimos Cadastros
```sql
SELECT 
    id,
    nome,
    rfid_uid as uid,
    TO_CHAR(data_cadastro, 'DD/MM/YY HH24:MI') as data
FROM funcionarios
ORDER BY data_cadastro DESC
LIMIT 10;
```

---

## 🔍 Verificações e Testes

### Teste Completo Passo a Passo

```bash
# 1. Verificar containers rodando
docker-compose ps
# Deve mostrar nodered_app e nodered_postgres UP

# 2. Verificar banco de dados
docker exec -it nodered_postgres psql -U nodered -d nodered_db
# \dt  (deve mostrar funcionarios e registros_ponto)
# \q

# 3. Teste cadastro via API
curl -X POST http://localhost:1880/api/funcionario \
  -H "Content-Type: application/json" \
  -d '{
    "nome": "Teste Usuario",
    "cpf": "11111111111",
    "cargo": "Teste",
    "departamento": "Teste",
    "rfid_uid": "AA:BB:CC:DD"
  }'
# Deve retornar 201

# 4. Verificar no banco
docker exec -it nodered_postgres psql -U nodered -d nodered_db \
  -c "SELECT * FROM funcionarios WHERE rfid_uid = 'AA:BB:CC:DD';"

# 5. Testar cadastro MQTT (simular ESP32)
mosquitto_pub -h mqtt.janks.dev.br \
  -t "ponto/cadastro/iniciar" \
  -u SEU_USUARIO -P SUA_SENHA \
  -m "true"

# 6. Simular TAG nova
mosquitto_pub -h mqtt.janks.dev.br \
  -t "ponto/cadastro/registrar" \
  -u SEU_USUARIO -P SUA_SENHA \
  -m '{"uid":"11:22:33:44"}'

# 7. Verificar resposta (inscrever no tópico)
mosquitto_sub -h mqtt.janks.dev.br \
  -t "ponto/cadastro/resposta" \
  -u SEU_USUARIO -P SUA_SENHA
```

---

## 📊 Consultas Úteis

### Ver Todos Funcionários
```sql
SELECT id, nome, cargo, rfid_uid, ativo 
FROM funcionarios 
ORDER BY nome;
```

### Funcionários Cadastrados Hoje
```sql
SELECT id, nome, rfid_uid, 
       TO_CHAR(data_cadastro, 'HH24:MI:SS') as hora
FROM funcionarios
WHERE DATE(data_cadastro) = CURRENT_DATE
ORDER BY data_cadastro DESC;
```

### Funcionários com Dados Incompletos
```sql
SELECT id, nome, rfid_uid, cargo
FROM funcionarios
WHERE cpf IS NULL 
   OR cargo = 'A definir'
   OR departamento IS NULL
ORDER BY data_cadastro DESC;
```

### Estatísticas
```sql
SELECT 
    COUNT(*) as total,
    COUNT(*) FILTER (WHERE ativo = true) as ativos,
    COUNT(*) FILTER (WHERE cpf IS NULL) as sem_cpf,
    COUNT(*) FILTER (WHERE DATE(data_cadastro) = CURRENT_DATE) as cadastros_hoje
FROM funcionarios;
```

---

## 🛡️ Segurança

### Melhorias Recomendadas

**1. Proteger Modo Cadastro**
```cpp
// No ESP32, adicionar senha
void recebeuMensagemMQTT(String topico, String conteudo) {
  if (topico == TOPIC_CADASTRO_INICIAR) {
    JsonDocument doc;
    deserializeJson(doc, conteudo);
    
    String senha = doc["senha"];
    if (senha == "SENHA_SECRETA_123") {
      estadoAtual = MODO_CADASTRO;
      mostrarModoCadastro();
    } else {
      Serial.println("ERRO: Senha incorreta");
      mostrarAcessoNegado();
    }
  }
}
```

**2. Timeout Automático**
```cpp
// Sair do modo cadastro após 2 minutos sem uso
case MODO_CADASTRO: {
  if (millis() - instanteTimeout > 120000) {  // 2 minutos
    Serial.println("TIMEOUT modo cadastro");
    estadoAtual = AGUARDANDO;
    mostrarTelaInicial();
  }
  // ... resto do código
  break;
}
```

**3. Limitar Cadastros**
```cpp
// Máximo 10 cadastros por sessão
int cadastrosNaSessao = 0;
const int MAX_CADASTROS = 10;

case MODO_CADASTRO: {
  if (cadastrosNaSessao >= MAX_CADASTROS) {
    Serial.println("LIMITE de cadastros atingido");
    estadoAtual = AGUARDANDO;
    mostrarTelaInicial();
    cadastrosNaSessao = 0;
  }
  // ...
}
```

---

## ✅ Checklist de Produção

Antes de usar em produção:

- [ ] Alterar senha do PostgreSQL no docker-compose.yml
- [ ] Configurar credenciais MQTT corretas
- [ ] Adicionar senha para modo cadastro
- [ ] Implementar timeout automático
- [ ] Criar backup automático do banco
- [ ] Testar recuperação de falhas
- [ ] Documentar TAGs master para emergência
- [ ] Treinar usuários administrativos
- [ ] Criar manual de operação
- [ ] Configurar logs e monitoramento

---

## 🎓 Exemplos de Uso

### Exemplo 1: Cadastro em Lote (Script)

```bash
#!/bin/bash
# cadastro_lote.sh - Cadastrar vários funcionários

FUNCIONARIOS=(
  "Maria Santos:11111111111:Analista:RH:AA:BB:CC:DD"
  "Pedro Costa:22222222222:Técnico:TI:EE:FF:00:11"
  "Ana Silva:33333333333:Gerente:Vendas:22:33:44:55"
)

for func in "${FUNCIONARIOS[@]}"; do
  IFS=':' read -r nome cpf cargo dept uid <<< "$func"
  
  curl -X POST http://localhost:1880/api/funcionario \
    -H "Content-Type: application/json" \
    -d "{
      \"nome\": \"$nome\",
      \"cpf\": \"$cpf\",
      \"cargo\": \"$cargo\",
      \"departamento\": \"$dept\",
      \"rfid_uid\": \"$uid\"
    }"
  
  echo "Cadastrado: $nome"
  sleep 1
done
```

### Exemplo 2: Web Form (HTML/JavaScript)

```html
<!DOCTYPE html>
<html>
<head>
  <title>Cadastro de Funcionário</title>
</head>
<body>
  <h1>Cadastrar Novo Funcionário</h1>
  <form id="cadastroForm">
    <input type="text" id="nome" placeholder="Nome Completo" required>
    <input type="text" id="cpf" placeholder="CPF" required>
    <input type="text" id="cargo" placeholder="Cargo" required>
    <input type="text" id="departamento" placeholder="Departamento" required>
    <input type="text" id="rfid_uid" placeholder="UID da TAG" required>
    <button type="submit">Cadastrar</button>
  </form>

  <script>
    document.getElementById('cadastroForm').onsubmit = async (e) => {
      e.preventDefault();
      
      const data = {
        nome: document.getElementById('nome').value,
        cpf: document.getElementById('cpf').value,
        cargo: document.getElementById('cargo').value,
        departamento: document.getElementById('departamento').value,
        rfid_uid: document.getElementById('rfid_uid').value
      };
      
      const response = await fetch('http://localhost:1880/api/funcionario', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
      });
      
      if (response.ok) {
        alert('Funcionário cadastrado com sucesso!');
        e.target.reset();
      } else {
        alert('Erro ao cadastrar');
      }
    };
  </script>
</body>
</html>
```

---

**Sistema completo de cadastro funcionando! 🎉**

