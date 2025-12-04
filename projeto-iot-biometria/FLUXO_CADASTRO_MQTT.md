# 🔄 Fluxo de Cadastro via MQTT (Grupo 4)

## Diagrama do Fluxo

```
ESP32                          Node-RED                        PostgreSQL
  │                               │                                │
  │  1. Entra em MODO_CADASTRO    │                                │
  │     (via MQTT: iniciar=true)  │                                │
  │                               │                                │
  │  2. Aproxima TAG RFID         │                                │
  │                               │                                │
  │  ──────────────────────────>  │                                │
  │   {"uid":"XX:XX:XX:XX"}       │                                │
  │   ponto/cadastro/registrar    │                                │
  │                               │                                │
  │                               │  3. Verifica se UID existe     │
  │                               │  ──────────────────────────>   │
  │                               │  SELECT * WHERE rfid_uid=$1    │
  │                               │                                │
  │                               │  <──────────────────────────   │
  │                               │  Resultado: [] ou [{...}]      │
  │                               │                                │
  │                               │  4a. Se NÃO existe:            │
  │                               │  ──────────────────────────>   │
  │                               │  INSERT INTO funcionarios      │
  │                               │  nome="Novo Func X"            │
  │                               │                                │
  │                               │  <──────────────────────────   │
  │                               │  RETURNING id, nome, uid       │
  │                               │                                │
  │  <──────────────────────────  │                                │
  │   {"sucesso":true,            │                                │
  │    "uid":"XX:XX:XX:XX",       │                                │
  │    "nome":"Novo Func 1234"}   │                                │
  │   ponto/cadastro/resposta     │                                │
  │                               │                                │
  │  5. Mostra "TAG CADASTRADA"   │                                │
  │                               │                                │
  │                               │  4b. Se JÁ existe:             │
  │  <──────────────────────────  │                                │
  │   {"sucesso":false,           │                                │
  │    "mensagem":"UID já...",    │                                │
  │    "funcionario":"João"}      │                                │
  │                               │                                │
  │  6. Mostra "TAG JÁ CADASTRADA"│                                │
```

## Estrutura do Grupo 4 no Node-RED

### Nós Principais:
1. **n4_1**: MQTT IN - Escuta `ponto/cadastro/registrar`
2. **n4_2**: JSON Parse - Converte string em objeto
3. **n4_3**: Function - Prepara query SELECT para verificar duplicata
4. **n4_4**: PostgreSQL - Executa SELECT
5. **n4_5**: Switch - Verifica se UID já existe
   - **Saída 1**: UID não existe → n4_6 (cadastrar)
   - **Saída 2**: UID existe → n4_8 (erro)
6. **n4_6**: Function - Gera nome automático "Novo Func X"
7. **n4_7**: PostgreSQL - INSERT com RETURNING
8. **n4_8**: Function - Monta resposta de erro
9. **n4_9**: Function - Formata resposta de sucesso
10. **n4_10**: MQTT OUT - Publica em `ponto/cadastro/resposta`

### Debug Nodes:
- **debug4_1**: 🔍 TAG Recebida - Mostra JSON recebido do ESP32
- **debug4_2**: 🔍 Query Verifica - Mostra SELECT antes de executar
- **debug4_3**: 🔍 Insert Novo Func - Mostra INSERT antes de executar

## Diferença do Grupo 3 (API HTTP)

| Aspecto | Grupo 3 (API) | Grupo 4 (MQTT) |
|---------|---------------|----------------|
| **Entrada** | HTTP POST /api/funcionario | MQTT ponto/cadastro/registrar |
| **Dados** | Nome, CPF, Cargo, etc. | Apenas UID |
| **Validação** | Nenhuma (insere direto) | Verifica duplicata |
| **Nome** | Fornecido pelo usuário | Gerado automaticamente |
| **Resposta** | HTTP 201 Created | MQTT ponto/cadastro/resposta |
| **Uso** | Interface web/admin | ESP32 diretamente |

## Como Testar

### 1. Importe o fluxo atualizado:
```bash
# Abra http://localhost:1880
# Menu > Import > Clipboard
# Cole o conteúdo de fluxo_nodered.json
# Deploy
```

### 2. Ative o modo cadastro via MQTTX:
```
Tópico: ponto/cadastro/iniciar
Payload: true
```

### 3. No ESP32:
- Aproxime uma TAG RFID nova
- Observe no Serial:
  ```
  --- NOVA TAG PARA CADASTRO ---
  UID: XX:XX:XX:XX
  MQTT >> ponto/cadastro/registrar: {"uid":"XX:XX:XX:XX"}
  ```

### 4. No Node-RED Debug:
Você verá:
```
🔍 TAG Recebida: {"uid":"XX:XX:XX:XX"}
🔍 Query Verifica: SELECT id, nome, rfid_uid FROM funcionarios WHERE rfid_uid = 'XX:XX:XX:XX'
🔍 Insert Novo Func: INSERT INTO funcionarios(nome, rfid_uid) VALUES('Novo Func 1234', 'XX:XX:XX:XX')
```

### 5. ESP32 recebe resposta:
```
MQTT << ponto/cadastro/resposta: {"sucesso":true,"uid":"XX:XX:XX:XX","nome":"Novo Func 1234","id":5}
CADASTRO OK: Novo Func 1234 (XX:XX:XX:XX)
Tela: TAG CADASTRADA - XX:XX:XX:XX
```

### 6. Teste a TAG cadastrada:
- Aproxime a mesma TAG novamente
- Deve validar corretamente agora!

## Verificação no Banco

```bash
# Ver funcionários cadastrados
docker exec nodered_postgres psql -U nodered -d nodered_db \
  -c "SELECT id, nome, rfid_uid, data_cadastro FROM funcionarios ORDER BY id DESC LIMIT 5;"

# Resultado esperado:
#  id |     nome      |   rfid_uid   |      data_cadastro      
# ----+---------------+--------------+-------------------------
#   5 | Novo Func 1234| XX:XX:XX:XX  | 2025-11-10 14:30:00
#   4 | João Silva    | C1:71:22:0D  | 2025-11-10 10:00:00
```

## Troubleshooting

### TAG mostra "cadastrada" mas não valida depois
**Causa**: INSERT não está sendo executado ou falhou
**Solução**:
1. Verifique debug4_3 - deve mostrar o INSERT
2. Verifique se há erro no PostgreSQL
3. Confirme que o nó n4_7 retorna RETURNING id, nome, rfid_uid

### TAG sempre mostra "já cadastrada" mesmo sendo nova
**Causa**: Switch (n4_5) invertido ou query errada
**Solução**:
1. Verifique debug4_2 - deve mostrar o SELECT
2. Confirme que resultado está vazio [] para TAG nova
3. Switch deve ter: Saída 1 = "empty", Saída 2 = "not empty"

### Resposta não chega no ESP32
**Causa**: MQTT OUT (n4_10) não está publicando
**Solução**:
1. Verifique se MQTT broker está conectado (bolinha verde)
2. Confirme tópico: `ponto/cadastro/resposta`
3. Teste publicar manualmente via MQTTX

---
✅ **Fluxo completo implementado!**

Agora o cadastro via MQTT funciona igual ao exemplo da documentação.
