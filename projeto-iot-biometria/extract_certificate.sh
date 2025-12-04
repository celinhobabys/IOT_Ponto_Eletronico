#!/bin/bash

echo "🔐 Extraindo certificado em formato PEM..."

# Extrai o certificado1 (Let's Encrypt) do arquivo certificados.h
# Pega tudo entre R"=EOF=( e )=EOF=" para o certificado1
sed -n '/const char certificado1\[\]/,/)=EOF=";/p' src/certificados.h | \
  sed '1d;$d' | \
  sed 's/^[[:space:]]*//' > ca-cert.pem

echo "✅ Certificado extraído em: ca-cert.pem"
echo ""
echo "Conteúdo do certificado:"
head -3 ca-cert.pem
echo "..."
tail -3 ca-cert.pem
echo ""
echo "Total de linhas: $(wc -l < ca-cert.pem)"
