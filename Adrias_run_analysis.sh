#!/usr/bin/env bash
set -euo pipefail

# Compilar o executável com otimizações
# Observação: usamos C++11 conforme pedido, mas o código compila também com C++17
if ! g++ knapSA_incompleto.cpp -o knapSA -O3 -std=c++11; then
  echo "Falha na compilação" >&2
  exit 1
fi

# Criar/limpar arquivo de resultados e adicionar cabeçalho
echo "instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms,tempo_total_ms,init,alpha,final,seed" > resultados.csv

# Encontrar todas as instâncias e executar em lote
# Usa IFS nulo e read -r para lidar com espaços no caminho
while IFS= read -r -d '' file; do
  echo "Processando: $file"
  ./knapSA "$file" >> resultados.csv
done < <(find problemInstances -type f -name 'test.in' -print0 | sort -z)

echo "Análise concluída. Resultados salvos em resultados.csv"
