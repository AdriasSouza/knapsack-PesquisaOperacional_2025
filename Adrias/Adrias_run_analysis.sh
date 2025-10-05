#!/usr/bin/env bash
set -euo pipefail

# Compila o executável com otimização
if ! g++ Adrias_knapSA.cpp -o knapSA -O3 -std=c++11; then
  echo "Falha na compilação" >&2
  exit 1
fi

# Cria/limpa arquivo de resultados e adiciona cabeçalho (6 colunas)
echo "instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms,tempo_total_ms" > resultados.csv

# Encontra todas as instâncias e executa em lote
# Usa IFS nulo e read -r para lidar com espaços no caminho
while IFS= read -r -d '' file; do
  echo "Processando: $file"
  ./knapSA "$file" >> resultados.csv
done < <(find problemInstances -type f -name 'test.in' -print0 | sort -z)

echo "Análise concluída. Resultados salvos em resultados.csv"
