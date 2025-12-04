# 🐛 Guia de Debug do Node-RED

## Debug Nodes Adicionados

### Grupo 1: Validação de Acesso (RFID)
1. **🔍 MQTT Recebido** - Mostra a mensagem raw recebida do tópico `ponto/rfid/validacao`
2. **🔍 Query Preparada** - Mostra a query SQL e parâmetros antes de executar
3. **🔍 Resultado DB** - Mostra o resultado da consulta no PostgreSQL
4. **�� Resposta Final** - Mostra a resposta que será enviada ao ESP32

### Grupo 2: Registro de Ponto
1. **🔍 Registro Recebido** - Mostra a mensagem raw do tópico `ponto/registro`
2. **🔍 Insert Preparado** - Mostra o SQL INSERT e parâmetros antes de executar

## Como Usar

1. Importe o fluxo atualizado no Node-RED:
   ```bash
   # Abra http://localhost:1880
   # Menu > Import > Clipboard
   # Cole o conteúdo de fluxo_nodered.json
   # Click em Deploy
   ```

2. Abra o painel de Debug:
   - No Node-RED, clique na aba **"Debug"** no lado direito

3. Teste a validação RFID:
   - Aproxime uma TAG no ESP32
   - Observe a sequência no Debug:
     - 🔍 MQTT Recebido: {"uid":"XX:XX:XX:XX"}
     - 🔍 Query Preparada: topic = SELECT..., payload = ["XX:XX:XX:XX"]
     - 🔍 Resultado DB: [{id: 1, nome: "João", rfid_uid: "XX:XX:XX:XX"}]
     - 🔍 Resposta Final: {valido: true, nome: "João", id: 1}

4. Teste o registro de ponto:
   - Gire o encoder após validação
   - Observe no Debug:
     - 🔍 Registro Recebido: {"funcionario_id":1,"tipo":"ENTRADA","timestamp":"..."}
     - 🔍 Insert Preparado: topic = INSERT INTO..., payload = [1,"ENTRADA","rfid",...]

## Troubleshooting com Debug

### Problema: TAG não valida
- **Verificar**: 🔍 MQTT Recebido
  - Se vazio: ESP32 não está publicando
  - Se tem dados: Ir para próximo debug

- **Verificar**: 🔍 Query Preparada
  - Confirmar que msg.payload tem o UID correto
  - Confirmar formato: ["XX:XX:XX:XX"]

- **Verificar**: 🔍 Resultado DB
  - Se vazio [] : TAG não está cadastrada no banco
  - Se tem erro: Problema de conexão com PostgreSQL
  - Se tem dados: Ir para próximo debug

- **Verificar**: �� Resposta Final
  - Confirmar se {valido: true, nome: "...", id: ...}
  - Se não chegou aqui: Problema no switch node

### Problema: Ponto não registra
- **Verificar**: 🔍 Registro Recebido
  - Se vazio: ESP32 não está publicando no tópico correto
  - Se tem dados: Verificar se funcionario_id é válido

- **Verificar**: 🔍 Insert Preparado
  - Confirmar formato dos parâmetros
  - Timestamp deve ser número Unix (segundos)

- **Verificar**: Log Registro (já existia)
  - Se tem erro: Ver mensagem de erro do PostgreSQL
  - Se sucesso: Registro foi salvo

## Comandos Úteis

```bash
# Ver funcionários cadastrados
docker exec nodered_postgres psql -U nodered -d nodered_db \
  -c "SELECT * FROM funcionarios;"

# Ver últimos registros
docker exec nodered_postgres psql -U nodered -d nodered_db \
  -c "SELECT * FROM registros_ponto ORDER BY data_hora DESC LIMIT 10;"

# Ver logs do Node-RED
docker logs -f nodered_app

# Ver logs do ESP32
pio device monitor
```

## Filtros de Debug

No painel de Debug do Node-RED, você pode:
- **Filtrar por node**: Clique no nome do debug node
- **Limpar**: Clique no ícone de lixeira
- **Copiar**: Clique com botão direito > Copy value
- **Expandir JSON**: Clique na seta ao lado dos objetos

## Next Steps

Após identificar o problema com os debugs:
1. Anote qual debug node mostrou o erro
2. Compartilhe a saída completa (pode copiar do painel)
3. Verifique os logs do PostgreSQL se necessário
4. Verifique a saída serial do ESP32

---
🚀 **Pronto para debugar!**
