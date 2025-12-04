#!/bin/bash

# ========================================
# Script para configurar banco de dados remoto
# Banco: postgresql.janks.dev.br
# ========================================

echo "🔧 Configurando banco de dados remoto do professor..."
echo ""

# Variáveis de conexão
DB_HOST="postgresql.janks.dev.br"
DB_PORT="5432"
DB_NAME="projeto"
DB_USER="iot"
DB_PASS="pepcon-garton"

# Exporta a senha para evitar prompt
export PGPASSWORD="$DB_PASS"

echo "📡 Testando conexão com o banco de dados..."
if psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "SELECT 1;" > /dev/null 2>&1; then
    echo "✅ Conexão estabelecida com sucesso!"
else
    echo "❌ Erro ao conectar no banco de dados!"
    echo ""
    echo "Verifique:"
    echo "  1. Se você tem acesso à internet"
    echo "  2. Se o servidor postgresql.janks.dev.br está acessível"
    echo "  3. Se as credenciais estão corretas"
    exit 1
fi

echo ""
echo "📊 Verificando tabelas existentes..."
EXISTING_TABLES=$(psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -t -c "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'public';")
echo "   Tabelas existentes no schema public: $EXISTING_TABLES"

echo ""
echo "🗂️  Criando/Atualizando schema do Ponto Eletrônico..."
psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -f init_database_remoto.sql

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Schema criado/atualizado com sucesso!"
    echo ""
    echo "📋 Resumo das tabelas do Ponto Eletrônico:"
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            schemaname,
            tablename,
            COALESCE(pg_size_pretty(pg_total_relation_size(schemaname||'.'||tablename)), '0 bytes') as size
        FROM pg_tables 
        WHERE schemaname = 'public' 
        AND tablename IN ('funcionarios', 'registros_ponto')
        ORDER BY tablename;
    "
    
    echo ""
    echo "👥 Funcionários cadastrados:"
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT id, nome, rfid_uid, data_cadastro 
        FROM funcionarios 
        ORDER BY id 
        LIMIT 10;
    "
    
    echo ""
    echo "📝 Últimos registros de ponto:"
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            f.nome AS funcionario,
            rp.data_hora,
            rp.tipo,
            rp.metodo
        FROM registros_ponto rp
        JOIN funcionarios f ON rp.funcionario_id = f.id
        ORDER BY rp.data_hora DESC
        LIMIT 5;
    "
    
    echo ""
    echo "🎉 Configuração concluída!"
    echo ""
    echo "📌 Próximos passos:"
    echo "   1. Reinicie o Node-RED: docker-compose restart"
    echo "   2. Reimporte o fluxo atualizado no Node-RED"
    echo "   3. Faça o Deploy do fluxo"
    echo "   4. Teste a validação RFID"
else
    echo ""
    echo "❌ Erro ao executar script SQL!"
    exit 1
fi

# Limpa a senha da variável de ambiente
unset PGPASSWORD

echo ""
echo "🔗 Informações de conexão:"
echo "   Host: $DB_HOST"
echo "   Porta: $DB_PORT"
echo "   Database: $DB_NAME"
echo "   Usuário: $DB_USER"
