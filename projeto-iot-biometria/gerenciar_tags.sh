#!/bin/bash

# ========================================
# Script para cadastrar TAG RFID no banco remoto
# ========================================

echo "🏷️  Cadastro de TAG RFID"
echo ""

# Variáveis de conexão
DB_HOST="postgresql.janks.dev.br"
DB_PORT="5432"
DB_NAME="projeto"
DB_USER="iot"
DB_PASS="pepcon-garton"

export PGPASSWORD="$DB_PASS"

# Função para listar funcionários
listar_funcionarios() {
    echo "👥 Funcionários cadastrados:"
    echo ""
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        SELECT 
            id,
            nome,
            COALESCE(rfid, '(sem TAG)') as rfid
        FROM funcionarios
        ORDER BY id;
    "
}

# Função para cadastrar novo funcionário
cadastrar_funcionario() {
    echo ""
    read -p "Nome do funcionário: " nome
    read -p "UID da TAG RFID (formato XX:XX:XX:XX): " rfid
    
    if [ -z "$nome" ] || [ -z "$rfid" ]; then
        echo "❌ Nome e RFID são obrigatórios!"
        return 1
    fi
    
    # Verifica se TAG já existe
    EXISTE=$(psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) FROM funcionarios WHERE rfid = '$rfid';
    " | xargs)
    
    if [ "$EXISTE" != "0" ]; then
        echo "❌ Esta TAG já está cadastrada!"
        psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
            SELECT id, nome, rfid FROM funcionarios WHERE rfid = '$rfid';
        "
        return 1
    fi
    
    # Insere novo funcionário
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        INSERT INTO funcionarios (nome, rfid) VALUES ('$nome', '$rfid');
    "
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Funcionário cadastrado com sucesso!"
        echo ""
        psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
            SELECT id, nome, rfid FROM funcionarios WHERE rfid = '$rfid';
        "
    else
        echo "❌ Erro ao cadastrar funcionário!"
    fi
}

# Função para atualizar TAG de funcionário existente
atualizar_tag() {
    echo ""
    listar_funcionarios
    echo ""
    read -p "ID do funcionário: " func_id
    read -p "Nova TAG RFID (formato XX:XX:XX:XX): " rfid
    
    if [ -z "$func_id" ] || [ -z "$rfid" ]; then
        echo "❌ ID e RFID são obrigatórios!"
        return 1
    fi
    
    # Verifica se TAG já está em uso
    EXISTE=$(psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -t -c "
        SELECT COUNT(*) FROM funcionarios WHERE rfid = '$rfid' AND id != $func_id;
    " | xargs)
    
    if [ "$EXISTE" != "0" ]; then
        echo "❌ Esta TAG já está cadastrada para outro funcionário!"
        return 1
    fi
    
    # Atualiza TAG
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
        UPDATE funcionarios SET rfid = '$rfid' WHERE id = $func_id;
    "
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ TAG atualizada com sucesso!"
        echo ""
        psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
            SELECT id, nome, rfid FROM funcionarios WHERE id = $func_id;
        "
    else
        echo "❌ Erro ao atualizar TAG!"
    fi
}

# Menu principal
while true; do
    echo ""
    echo "=========================================="
    echo "  Gerenciamento de TAGs RFID"
    echo "=========================================="
    echo ""
    echo "1) Listar funcionários"
    echo "2) Cadastrar novo funcionário com TAG"
    echo "3) Atualizar TAG de funcionário existente"
    echo "4) Ver últimos registros de ponto"
    echo "5) Sair"
    echo ""
    read -p "Escolha uma opção: " opcao
    
    case $opcao in
        1)
            listar_funcionarios
            ;;
        2)
            cadastrar_funcionario
            ;;
        3)
            atualizar_tag
            ;;
        4)
            echo ""
            echo "📝 Últimos registros de ponto:"
            echo ""
            psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "
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
            ;;
        5)
            echo "Até logo!"
            unset PGPASSWORD
            exit 0
            ;;
        *)
            echo "❌ Opção inválida!"
            ;;
    esac
done
