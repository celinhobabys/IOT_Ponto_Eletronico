-- ========================================
-- Script de Inicialização do Banco de Dados
-- Ponto Eletrônico IoT
-- 
-- IMPORTANTE: Este script usa CREATE TABLE IF NOT EXISTS
-- para não deletar dados existentes!
-- ========================================

-- Criar tabela de funcionários (se não existir)
CREATE TABLE IF NOT EXISTS funcionarios (
    id SERIAL PRIMARY KEY,
    nome VARCHAR(255) NOT NULL,
    cpf VARCHAR(14),
    cargo VARCHAR(100),
    departamento VARCHAR(100),
    rfid_uid VARCHAR(50) UNIQUE NOT NULL,
    data_cadastro TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Criar índice no rfid_uid (se não existir)
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_indexes 
        WHERE indexname = 'idx_funcionarios_rfid'
    ) THEN
        CREATE INDEX idx_funcionarios_rfid ON funcionarios(rfid_uid);
    END IF;
END $$;

-- Criar tabela de registros de ponto (se não existir)
CREATE TABLE IF NOT EXISTS registros_ponto (
    id SERIAL PRIMARY KEY,
    funcionario_id INTEGER REFERENCES funcionarios(id) ON DELETE CASCADE,
    data_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    tipo VARCHAR(10) CHECK (tipo IN ('ENTRADA', 'SAIDA')),
    metodo VARCHAR(50) DEFAULT 'rfid',
    observacao TEXT
);

-- Criar índices (se não existirem)
DO $$
BEGIN
    IF NOT EXISTS (
        SELECT 1 FROM pg_indexes 
        WHERE indexname = 'idx_registros_funcionario'
    ) THEN
        CREATE INDEX idx_registros_funcionario ON registros_ponto(funcionario_id);
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM pg_indexes 
        WHERE indexname = 'idx_registros_data'
    ) THEN
        CREATE INDEX idx_registros_data ON registros_ponto(data_hora);
    END IF;

    IF NOT EXISTS (
        SELECT 1 FROM pg_indexes 
        WHERE indexname = 'idx_registros_tipo'
    ) THEN
        CREATE INDEX idx_registros_tipo ON registros_ponto(tipo);
    END IF;
END $$;

-- Criar view de horas trabalhadas diárias (ou substituir se existir)
CREATE OR REPLACE VIEW horas_trabalhadas_diarias AS
SELECT 
    f.id AS funcionario_id,
    f.nome AS funcionario_nome,
    DATE(rp.data_hora) AS data,
    MIN(CASE WHEN rp.tipo = 'ENTRADA' THEN rp.data_hora END) AS primeira_entrada,
    MAX(CASE WHEN rp.tipo = 'SAIDA' THEN rp.data_hora END) AS ultima_saida,
    COUNT(CASE WHEN rp.tipo = 'ENTRADA' THEN 1 END) AS total_entradas,
    COUNT(CASE WHEN rp.tipo = 'SAIDA' THEN 1 END) AS total_saidas,
    EXTRACT(EPOCH FROM (
        MAX(CASE WHEN rp.tipo = 'SAIDA' THEN rp.data_hora END) - 
        MIN(CASE WHEN rp.tipo = 'ENTRADA' THEN rp.data_hora END)
    )) / 3600.0 AS horas_trabalhadas
FROM funcionarios f
LEFT JOIN registros_ponto rp ON f.id = rp.funcionario_id
GROUP BY f.id, f.nome, DATE(rp.data_hora)
ORDER BY data DESC, f.nome;

-- Criar view do último registro por funcionário (ou substituir se existir)
CREATE OR REPLACE VIEW ultimo_registro_por_funcionario AS
SELECT DISTINCT ON (f.id)
    f.id AS funcionario_id,
    f.nome AS funcionario_nome,
    rp.data_hora AS ultimo_registro,
    rp.tipo AS tipo_ultimo_registro,
    CASE 
        WHEN rp.tipo = 'ENTRADA' THEN 'Presente'
        WHEN rp.tipo = 'SAIDA' THEN 'Ausente'
        ELSE 'Sem registro'
    END AS status_atual
FROM funcionarios f
LEFT JOIN registros_ponto rp ON f.id = rp.funcionario_id
ORDER BY f.id, rp.data_hora DESC;

-- Criar função para verificar se funcionário está presente (ou substituir se existir)
CREATE OR REPLACE FUNCTION funcionario_presente(func_id INTEGER)
RETURNS BOOLEAN AS $$
DECLARE
    ultimo_tipo VARCHAR(10);
BEGIN
    SELECT tipo INTO ultimo_tipo
    FROM registros_ponto
    WHERE funcionario_id = func_id
    ORDER BY data_hora DESC
    LIMIT 1;
    
    RETURN ultimo_tipo = 'ENTRADA';
END;
$$ LANGUAGE plpgsql;

-- Criar função para calcular horas no período (ou substituir se existir)
CREATE OR REPLACE FUNCTION calcular_horas_periodo(
    func_id INTEGER,
    data_inicio DATE,
    data_fim DATE
)
RETURNS TABLE (
    data DATE,
    horas_trabalhadas NUMERIC
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        DATE(rp.data_hora) AS data,
        COALESCE(
            EXTRACT(EPOCH FROM (
                MAX(CASE WHEN rp.tipo = 'SAIDA' THEN rp.data_hora END) - 
                MIN(CASE WHEN rp.tipo = 'ENTRADA' THEN rp.data_hora END)
            )) / 3600.0,
            0
        )::NUMERIC(10,2) AS horas_trabalhadas
    FROM registros_ponto rp
    WHERE rp.funcionario_id = func_id
      AND DATE(rp.data_hora) BETWEEN data_inicio AND data_fim
    GROUP BY DATE(rp.data_hora)
    ORDER BY DATE(rp.data_hora);
END;
$$ LANGUAGE plpgsql;

-- Inserir dados de teste APENAS se a tabela estiver vazia
DO $$
BEGIN
    IF NOT EXISTS (SELECT 1 FROM funcionarios LIMIT 1) THEN
        -- Funcionários de teste
        INSERT INTO funcionarios (nome, cpf, cargo, departamento, rfid_uid) VALUES
        ('João Silva', '123.456.789-00', 'Desenvolvedor', 'TI', 'C1:71:22:0D'),
        ('Maria Santos', '987.654.321-00', 'Designer', 'Marketing', 'A2:B3:C4:D5'),
        ('Pedro Costa', '456.789.123-00', 'Analista', 'Operações', 'E6:F7:G8:H9');

        RAISE NOTICE 'Dados de teste inseridos com sucesso!';
    ELSE
        RAISE NOTICE 'Tabela funcionarios já contém dados. Pulando inserção de dados de teste.';
    END IF;
END $$;

-- Inserir registros de ponto de exemplo APENAS se não existirem
DO $$
DECLARE
    joao_id INTEGER;
    maria_id INTEGER;
BEGIN
    IF NOT EXISTS (SELECT 1 FROM registros_ponto LIMIT 1) THEN
        -- Pega os IDs (podem ser diferentes se já existirem outros funcionários)
        SELECT id INTO joao_id FROM funcionarios WHERE rfid_uid = 'C1:71:22:0D';
        SELECT id INTO maria_id FROM funcionarios WHERE rfid_uid = 'A2:B3:C4:D5';

        IF joao_id IS NOT NULL AND maria_id IS NOT NULL THEN
            INSERT INTO registros_ponto (funcionario_id, data_hora, tipo, metodo) VALUES
            (joao_id, CURRENT_TIMESTAMP - INTERVAL '2 hours', 'ENTRADA', 'rfid'),
            (joao_id, CURRENT_TIMESTAMP - INTERVAL '30 minutes', 'SAIDA', 'rfid'),
            (maria_id, CURRENT_TIMESTAMP - INTERVAL '1 hour', 'ENTRADA', 'rfid'),
            (maria_id, CURRENT_TIMESTAMP - INTERVAL '15 minutes', 'SAIDA', 'rfid');

            RAISE NOTICE 'Registros de ponto de exemplo inseridos com sucesso!';
        END IF;
    ELSE
        RAISE NOTICE 'Tabela registros_ponto já contém dados. Pulando inserção de exemplos.';
    END IF;
END $$;

-- Resumo final
SELECT 'Inicialização concluída!' AS status;
SELECT COUNT(*) AS total_funcionarios FROM funcionarios;
SELECT COUNT(*) AS total_registros FROM registros_ponto;
