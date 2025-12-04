-- Script de Inicialização do Banco de Dados
-- Projeto: Ponto Eletrônico IoT
-- Database: nodered_db (criado pelo docker-compose)

-- Conectar ao banco: psql -U nodered -d nodered_db

-- =============================================
-- TABELA: funcionarios
-- Armazena dados dos funcionários cadastrados
-- =============================================
CREATE TABLE IF NOT EXISTS funcionarios (
    id SERIAL PRIMARY KEY,
    nome VARCHAR(100) NOT NULL,
    cpf VARCHAR(11) UNIQUE,
    cargo VARCHAR(50),
    departamento VARCHAR(50),
    ativo BOOLEAN DEFAULT true,
    digital_id INTEGER UNIQUE,
    rfid_uid VARCHAR(50) UNIQUE,
    data_cadastro TIMESTAMP DEFAULT NOW(),
    
    CONSTRAINT chk_identificacao CHECK (
        digital_id IS NOT NULL OR rfid_uid IS NOT NULL
    )
);

-- Índices para melhorar performance de busca
CREATE INDEX IF NOT EXISTS idx_funcionarios_rfid ON funcionarios(rfid_uid);
CREATE INDEX IF NOT EXISTS idx_funcionarios_digital ON funcionarios(digital_id);
CREATE INDEX IF NOT EXISTS idx_funcionarios_ativo ON funcionarios(ativo);

-- =============================================
-- TABELA: registros_ponto
-- Armazena todos os registros de entrada/saída
-- =============================================
CREATE TABLE IF NOT EXISTS registros_ponto (
    id SERIAL PRIMARY KEY,
    funcionario_id INTEGER NOT NULL REFERENCES funcionarios(id) ON DELETE CASCADE,
    data_hora TIMESTAMP DEFAULT NOW(),
    tipo VARCHAR(10) NOT NULL CHECK (tipo IN ('ENTRADA', 'SAIDA', 'entrada', 'saida')),
    metodo VARCHAR(15) NOT NULL CHECK (metodo IN ('biometria', 'rfid', 'manual')),
    localizacao VARCHAR(50) DEFAULT 'Principal',
    observacao TEXT,
    
    CONSTRAINT chk_tipo_valido CHECK (tipo IN ('ENTRADA', 'SAIDA', 'entrada', 'saida'))
);

-- Índices para queries de relatórios
CREATE INDEX IF NOT EXISTS idx_registros_data ON registros_ponto(data_hora DESC);
CREATE INDEX IF NOT EXISTS idx_registros_funcionario ON registros_ponto(funcionario_id);
CREATE INDEX IF NOT EXISTS idx_registros_tipo ON registros_ponto(tipo);
CREATE INDEX IF NOT EXISTS idx_registros_data_func ON registros_ponto(data_hora, funcionario_id);

-- =============================================
-- VIEW: horas_trabalhadas_diarias
-- Calcula horas trabalhadas por funcionário por dia
-- =============================================
CREATE OR REPLACE VIEW horas_trabalhadas_diarias AS
SELECT 
    f.id as funcionario_id,
    f.nome,
    DATE(r.data_hora) as data,
    COUNT(CASE WHEN UPPER(r.tipo) = 'ENTRADA' THEN 1 END) as num_entradas,
    COUNT(CASE WHEN UPPER(r.tipo) = 'SAIDA' THEN 1 END) as num_saidas,
    MIN(CASE WHEN UPPER(r.tipo) = 'ENTRADA' THEN r.data_hora END) as primeira_entrada,
    MAX(CASE WHEN UPPER(r.tipo) = 'SAIDA' THEN r.data_hora END) as ultima_saida
FROM funcionarios f
LEFT JOIN registros_ponto r ON f.id = r.funcionario_id
WHERE f.ativo = true
GROUP BY f.id, f.nome, DATE(r.data_hora)
ORDER BY data DESC, f.nome;

-- =============================================
-- VIEW: ultimo_registro_por_funcionario
-- Mostra o último registro de cada funcionário
-- =============================================
CREATE OR REPLACE VIEW ultimo_registro_por_funcionario AS
SELECT DISTINCT ON (f.id)
    f.id as funcionario_id,
    f.nome,
    f.cargo,
    f.departamento,
    r.data_hora,
    r.tipo,
    r.metodo
FROM funcionarios f
LEFT JOIN registros_ponto r ON f.id = r.funcionario_id
WHERE f.ativo = true
ORDER BY f.id, r.data_hora DESC;

-- =============================================
-- DADOS DE TESTE
-- Funcionários e registros para testar o sistema
-- =============================================

-- Inserir funcionários de teste
INSERT INTO funcionarios (nome, cpf, cargo, departamento, rfid_uid) VALUES
    ('João Silva', '12345678901', 'Desenvolvedor', 'TI', 'C1:71:22:0D'),
    ('Maria Santos', '98765432109', 'Designer', 'Marketing', 'A2:B3:C4:D5'),
    ('Pedro Costa', '11122233344', 'Gerente', 'TI', 'E6:F7:G8:H9')
ON CONFLICT (rfid_uid) DO NOTHING;

-- Inserir registros de teste de hoje
INSERT INTO registros_ponto (funcionario_id, tipo, metodo, data_hora) VALUES
    (1, 'ENTRADA', 'rfid', NOW() - INTERVAL '8 hours'),
    (1, 'SAIDA', 'rfid', NOW() - INTERVAL '4 hours'),
    (2, 'ENTRADA', 'rfid', NOW() - INTERVAL '7 hours 30 minutes'),
    (3, 'ENTRADA', 'rfid', NOW() - INTERVAL '8 hours 15 minutes')
ON CONFLICT DO NOTHING;

-- =============================================
-- FUNÇÕES ÚTEIS
-- =============================================

-- Função para verificar se funcionário está presente
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
    
    RETURN (ultimo_tipo = 'ENTRADA');
END;
$$ LANGUAGE plpgsql;

-- Função para calcular horas trabalhadas em um período
CREATE OR REPLACE FUNCTION calcular_horas_periodo(
    func_id INTEGER,
    data_inicio TIMESTAMP,
    data_fim TIMESTAMP
) 
RETURNS NUMERIC AS $$
DECLARE
    total_segundos BIGINT := 0;
    ultima_entrada TIMESTAMP;
    reg RECORD;
BEGIN
    FOR reg IN 
        SELECT data_hora, tipo 
        FROM registros_ponto 
        WHERE funcionario_id = func_id 
          AND data_hora BETWEEN data_inicio AND data_fim
        ORDER BY data_hora
    LOOP
        IF UPPER(reg.tipo) = 'ENTRADA' THEN
            ultima_entrada := reg.data_hora;
        ELSIF UPPER(reg.tipo) = 'SAIDA' AND ultima_entrada IS NOT NULL THEN
            total_segundos := total_segundos + 
                EXTRACT(EPOCH FROM (reg.data_hora - ultima_entrada))::BIGINT;
            ultima_entrada := NULL;
        END IF;
    END LOOP;
    
    -- Retorna horas (com 2 casas decimais)
    RETURN ROUND((total_segundos::NUMERIC / 3600), 2);
END;
$$ LANGUAGE plpgsql;

-- =============================================
-- VERIFICAÇÃO FINAL
-- =============================================

-- Mostrar funcionários cadastrados
SELECT 'Funcionários cadastrados:' as info;
SELECT id, nome, cargo, rfid_uid FROM funcionarios WHERE ativo = true;

-- Mostrar últimos registros
SELECT 'Últimos 5 registros:' as info;
SELECT 
    r.id,
    f.nome,
    r.tipo,
    r.metodo,
    TO_CHAR(r.data_hora, 'DD/MM/YYYY HH24:MI:SS') as data_hora
FROM registros_ponto r
JOIN funcionarios f ON r.funcionario_id = f.id
ORDER BY r.data_hora DESC
LIMIT 5;

-- Estatísticas
SELECT 'Estatísticas:' as info;
SELECT 
    COUNT(DISTINCT id) as total_funcionarios,
    (SELECT COUNT(*) FROM registros_ponto WHERE DATE(data_hora) = CURRENT_DATE) as registros_hoje,
    (SELECT COUNT(*) FROM registros_ponto) as total_registros
FROM funcionarios
WHERE ativo = true;
