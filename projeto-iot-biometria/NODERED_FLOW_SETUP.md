# Node-RED Flow Setup - Ponto Eletrônico IoT

## ✅ Status: Docker e Database Configurados!

### O que foi feito:
- ✅ Node-RED rodando com `node-red-contrib-postgresql` instalado
- ✅ PostgreSQL configurado com tabelas e dados de teste
- ✅ Fluxo Node-RED corrigido e pronto para importar

---

## 📥 Importando o Fluxo no Node-RED

### Passo 1: Acesse o Node-RED
Abra seu navegador em: **http://localhost:1880**

### Passo 2: Abra o Menu de Importação
1. Clique no **menu hamburger** (☰) no canto superior direito
2. Selecione **Import** (Importar)

### Passo 3: Importe o Arquivo
1. Clique em **select a file to import**
2. Navegue até: `/home/eduardo/projeto-iot/fluxo_nodered.json`
3. Clique em **Import**

### Passo 4: Configure a Conexão PostgreSQL
1. Dê um duplo clique em qualquer nó **postgresql** (azul)
2. Clique no ícone de **lápis** ao lado de "PontoEletronicoDB"
3. Verifique/ajuste as configurações:
   - **Host**: `postgres` (nome do container)
   - **Port**: `5432`
   - **Database**: `nodered_db`
   - **User**: `nodered`
   - **Password**: `YOUR_SECURE_PASSWORD` (a mesma do docker-compose.yml)
4. Clique em **Update** e depois **Done**

### Passo 5: Configure o MQTT Broker
1. Dê um duplo clique em qualquer nó **mqtt in** ou **mqtt out** (verde)
2. Clique no ícone de **lápis** ao lado do broker
3. Configure:
   - **Server**: `mqtt.janks.dev.br`
   - **Port**: `1883`
   - **Client ID**: `nodered_ponto_eletronico`
   - **Username**: Seu usuário MQTT
   - **Password**: Sua senha MQTT
4. Clique em **Update** e depois **Done**

### Passo 6: Deploy
Clique no botão vermelho **Deploy** no canto superior direito

---

## 🔄 Estrutura dos Fluxos

### Grupo 1: Validação de Acesso (RFID)
```
ESP32 → MQTT → Node-RED → PostgreSQL → MQTT → ESP32
```

**Funcionamento:**
1. ESP32 publica UID da TAG no tópico `ponto/rfid/validacao`
2. Node-RED recebe e busca no banco de dados
3. Se encontrado, retorna `{valido: true, nome: "João", id: 123}`
4. Se não encontrado, retorna `{valido: false}`
5. ESP32 exibe mensagem correspondente no ePaper

### Grupo 2: Registro de Ponto
```
ESP32 → MQTT → Node-RED → PostgreSQL
```

**Funcionamento:**
1. Após validação, usuário gira o encoder
2. ESP32 publica registro no tópico `ponto/registro`
3. Node-RED recebe e insere no banco `registros_ponto`
4. Dados ficam disponíveis para relatórios

### Grupo 3: Cadastro de Funcionário (API)
```
HTTP POST → Node-RED → PostgreSQL → MQTT → ESP32
```

**Funcionamento:**
1. Sistema web envia POST para `/api/funcionario`
2. Node-RED insere no banco `funcionarios`
3. Notifica ESP32 sobre novo cadastro
4. Retorna 201 Created

---

## 🧪 Testando o Sistema

### Teste 1: Validação de RFID (Simulação via MQTT)

Você pode testar sem o ESP32 usando o debug do Node-RED:

1. No Node-RED, adicione um nó **inject**
2. Configure para enviar:
   ```json
   {
     "uid": "C1:71:22:0D"
   }
   ```
3. Conecte ao tópico `ponto/rfid/validacao`
4. Adicione um nó **debug** no tópico `ponto/rfid/resposta`
5. Clique no botão do inject
6. Veja a resposta no painel de debug (deve retornar dados do João Silva)

### Teste 2: Registro de Ponto

1. Adicione um nó **inject** com:
   ```json
   {
     "funcionario_id": 1,
     "tipo": "ENTRADA",
     "timestamp": "1699632000",
     "metodo": "rfid"
   }
   ```
2. Conecte ao tópico `ponto/registro`
3. Click no botão
4. Verifique no banco:
   ```sql
   SELECT * FROM registros_ponto ORDER BY data_hora DESC LIMIT 5;
   ```

### Teste 3: Cadastro de Funcionário

Use `curl` ou Postman:

```bash
curl -X POST http://localhost:1880/api/funcionario \
  -H "Content-Type: application/json" \
  -d '{
    "nome": "Carlos Oliveira",
    "cpf": "55566677788",
    "cargo": "Analista",
    "departamento": "RH",
    "rfid_uid": "1A:2B:3C:4D"
  }'
```

---

## 📊 Consultando Dados no PostgreSQL

### Via Docker:
```bash
docker exec -it nodered_postgres psql -U nodered -d nodered_db
```

### Queries Úteis:

#### Ver todos os funcionários:
```sql
SELECT * FROM funcionarios;
```

#### Ver registros de hoje:
```sql
SELECT 
    f.nome,
    r.tipo,
    r.data_hora,
    r.metodo
FROM registros_ponto r
JOIN funcionarios f ON r.funcionario_id = f.id
WHERE DATE(r.data_hora) = CURRENT_DATE
ORDER BY r.data_hora;
```

#### Horas trabalhadas por funcionário hoje:
```sql
SELECT * FROM horas_trabalhadas_diarias
WHERE data = CURRENT_DATE;
```

#### Verificar se funcionário está presente:
```sql
SELECT 
    f.nome,
    funcionario_presente(f.id) as esta_presente
FROM funcionarios f
WHERE f.ativo = true;
```

#### Calcular horas trabalhadas no mês:
```sql
SELECT 
    f.nome,
    calcular_horas_periodo(
        f.id,
        DATE_TRUNC('month', CURRENT_DATE),
        CURRENT_DATE
    ) as horas_mes
FROM funcionarios f
WHERE f.ativo = true;
```

---

## 🔧 Troubleshooting

### Problema: Nós PostgreSQL aparecem como "Unknown"
**Solução:** O fluxo já foi corrigido! Reimporte o arquivo `fluxo_nodered.json`

### Problema: Erro de conexão PostgreSQL
**Verificar:**
1. Container rodando: `docker ps`
2. Senha correta no nó de configuração
3. Host deve ser `postgres` (não `localhost`)

### Problema: MQTT não conecta
**Verificar:**
1. Credenciais corretas (usuário e senha)
2. Broker acessível: `ping mqtt.janks.dev.br`
3. Porta 1883 aberta

### Problema: Query SQL retorna erro
**Verificar:**
1. Tabelas criadas: Execute `setup_database.sh` novamente
2. Sintaxe SQL no Node-RED
3. Logs no Node-RED (painel Debug)

---

## 📱 Integrando com o ESP32

### No código do ESP32 (`main.cpp`):

Já está configurado! Apenas ajuste as credenciais:

```cpp
// WiFi
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// MQTT
const char* MQTT_BROKER = "mqtt.janks.dev.br";
const char* MQTT_USER = "SEU_USUARIO";
const char* MQTT_PASSWORD = "SUA_SENHA_MQTT";
```

### Fluxo Completo:
1. **TAG aproximada** → ESP32 lê UID
2. **Validação** → Publica em `ponto/rfid/validacao`
3. **Node-RED** → Busca no PostgreSQL
4. **Resposta** → Publica em `ponto/rfid/resposta`
5. **ESP32** → Exibe nome no ePaper
6. **Encoder girado** → Define ENTRADA/SAIDA
7. **Registro** → Publica em `ponto/registro`
8. **Node-RED** → Salva no PostgreSQL
9. **Confirmação** → ESP32 exibe no ePaper

---

## 📈 Próximos Passos

- [ ] Criar dashboard Grafana para visualização
- [ ] Desenvolver interface web administrativa
- [ ] Adicionar modo de cadastro de novas TAGs
- [ ] Implementar notificações (Telegram/Email)
- [ ] Criar relatórios automáticos
- [ ] Adicionar validação biométrica

---

## 🆘 Comandos Rápidos

```bash
# Ver logs Node-RED
docker logs nodered_app -f

# Ver logs PostgreSQL
docker logs nodered_postgres -f

# Reiniciar tudo
docker-compose restart

# Parar tudo
docker-compose down

# Iniciar tudo
docker-compose up -d

# Acessar PostgreSQL
docker exec -it nodered_postgres psql -U nodered -d nodered_db

# Backup do banco
docker exec nodered_postgres pg_dump -U nodered nodered_db > backup.sql

# Restaurar banco
docker exec -i nodered_postgres psql -U nodered -d nodered_db < backup.sql
```

---

**Sistema pronto para uso! 🚀**
