# ✅ Migração para Banco de Dados Remoto - Concluída

## 🔄 Mudanças Aplicadas

### 1. **Configuração do Banco de Dados**
- **Host:** `postgresql.janks.dev.br`
- **Porta:** `5432`
- **Database:** `projeto` 
- **Usuário:** `iot`
- **Senha:** `pepcon-garton`

### 2. **Estrutura da Tabela**
A tabela `funcionarios` no banco remoto já existia com estrutura:
- `id` (integer, PRIMARY KEY)
- `nome` (text, NOT NULL)
- `rfid` (text) ← **Nota: é `rfid`, não `rfid_uid`**

### 3. **Arquivos Atualizados**

#### `fluxo_nodered.json`
- ✅ Host alterado: `postgres` → `postgresql.janks.dev.br`
- ✅ Database alterado: `nodered_db` → `projeto`
- ✅ Usuário alterado: `nodered` → `iot`
- ✅ Senha alterada: `YOUR_SECURE_PASSWORD` → `pepcon-garton`
- ✅ Todas as queries: `rfid_uid` → `rfid` (9 substituições)

#### `docker-compose.yml`
- ✅ Removido container `postgres` local
- ✅ Removido volume `postgres_data`
- ✅ Removido `depends_on: postgres`

### 4. **Scripts Criados**

#### `init_database_remoto.sql`
Script SQL para criar tabelas e schemas **SEM deletar dados existentes**:
- `CREATE TABLE IF NOT EXISTS funcionarios`
- `CREATE TABLE IF NOT EXISTS registros_ponto`
- Views, funções e índices
- Dados de teste apenas se tabela vazia

#### `setup_database_remoto.sh`
Script bash para executar inicialização:
- Testa conexão
- Lista tabelas existentes
- Executa SQL
- Mostra resumo

## 📝 Próximos Passos

### 1. Reimportar Fluxo no Node-RED
```
1. Abra http://localhost:1880
2. Menu (☰) → Import → Clipboard
3. Cole o conteúdo de fluxo_nodered.json
4. Click em Deploy 🚀
```

### 2. Teste a Conexão
Aproxime uma TAG RFID e verifique os debugs:

**🔍 debug1_1** - Deve mostrar:
```json
{"uid":"BA:69:8F:1A"}
```

**🔍 debug1_2** - Deve mostrar:
```json
{
  "topic": "SELECT id, nome, rfid FROM funcionarios WHERE rfid = $1",
  "payload": ["BA:69:8F:1A"]
}
```

**🔍 debug1_3** - Deve mostrar resultado do banco:
```json
{
  "payload": [
    {
      "id": 1,
      "nome": "João Silva",
      "rfid": "BA:69:8F:1A"
    }
  ]
}
```

### 3. Cadastrar Nova TAG
Para cadastrar nova TAG, execute:

```bash
PGPASSWORD="pepcon-garton" psql -h postgresql.janks.dev.br -p 5432 -U iot -d projeto -c "
INSERT INTO funcionarios (nome, rfid) 
VALUES ('Novo Funcionário', 'XX:XX:XX:XX');
"
```

Ou use o modo cadastro via MQTT conforme documentado.

## 🔍 Verificações

### Ver Funcionários Cadastrados:
```bash
PGPASSWORD="pepcon-garton" psql -h postgresql.janks.dev.br -p 5432 -U iot -d projeto -c "
SELECT * FROM funcionarios ORDER BY id;
"
```

### Ver Últimos Registros de Ponto:
```bash
PGPASSWORD="pepcon-garton" psql -h postgresql.janks.dev.br -p 5432 -U iot -d projeto -c "
SELECT 
    f.nome AS funcionario,
    rp.data_hora,
    rp.tipo,
    rp.metodo
FROM registros_ponto rp
JOIN funcionarios f ON rp.funcionario_id = f.id
ORDER BY rp.data_hora DESC
LIMIT 10;
"
```

### Teste de Conexão do Node-RED:
Veja os logs:
```bash
docker logs -f nodered_app
```

## ⚠️ Notas Importantes

1. **Não há mais container PostgreSQL local**
   - O banco `nodered_postgres` foi removido
   - Todos os dados agora são salvos no banco remoto do professor

2. **Estrutura de coluna diferente**
   - Banco remoto usa `rfid` (sem `_uid`)
   - Campos como `cpf`, `cargo`, `departamento` não existem
   - Apenas: `id`, `nome`, `rfid`

3. **Dados existentes preservados**
   - O script `init_database_remoto.sql` usa `IF NOT EXISTS`
   - Não deleta dados que já estão no banco
   - Tabelas do professor continuam intactas

4. **Backup local**
   - Se precisar dos dados locais, eles ainda estão no volume Docker
   - Para exportar: `docker run --rm -v projeto-iot_postgres_data:/data alpine tar -czf - /data > backup.tar.gz`

## 🚀 Status

✅ Configuração concluída
✅ Node-RED reiniciado
✅ Fluxo atualizado
✅ Pronto para importar e testar!

Próximo teste: Aproxime uma TAG RFID e veja os debugs no Node-RED!
