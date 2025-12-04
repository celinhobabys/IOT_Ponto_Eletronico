#!/bin/bash
# Script para configurar o banco de dados do Ponto Eletrônico
# Usage: ./setup_database.sh

echo "=================================="
echo "Ponto Eletrônico IoT - Setup DB"
echo "=================================="
echo ""

# Verifica se o PostgreSQL está rodando
if ! docker ps | grep -q nodered_postgres; then
    echo "❌ PostgreSQL não está rodando!"
    echo "Execute: docker-compose up -d"
    exit 1
fi

echo "✅ PostgreSQL está rodando"
echo ""

# Aguarda PostgreSQL estar pronto
echo "⏳ Aguardando PostgreSQL ficar pronto..."
sleep 3

# Executa o script SQL
echo "📝 Criando tabelas e dados iniciais..."
docker exec -i nodered_postgres psql -U nodered -d nodered_db < init_database.sql

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Banco de dados configurado com sucesso!"
    echo ""
    echo "📊 Para acessar o PostgreSQL:"
    echo "   docker exec -it nodered_postgres psql -U nodered -d nodered_db"
    echo ""
    echo "🔍 Comandos úteis:"
    echo "   \\dt              - Listar tabelas"
    echo "   \\dv              - Listar views"
    echo "   \\df              - Listar funções"
    echo "   \\d+ funcionarios - Descrever tabela funcionarios"
    echo "   \\q               - Sair"
    echo ""
    echo "🌐 Node-RED: http://localhost:1880"
    echo "   Importe o arquivo: fluxo_nodered.json"
    echo ""
else
    echo ""
    echo "❌ Erro ao configurar banco de dados"
    echo "Verifique os logs: docker logs nodered_postgres"
    exit 1
fi
