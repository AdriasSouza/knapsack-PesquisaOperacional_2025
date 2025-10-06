#!/bin/bash

SOURCE_FILE="knapSA.cpp"
# Nome do executável que será gerado
EXECUTABLE="knapSA"
# Diretório principal que contém todas as instâncias do problema
BASE_DIR="problemInstances"
# Arquivo CSV de saída onde os resultados serão salvos
CSV_FILE="results.csv"
# Diretório para armazenar os arquivos de saída de cada execução
RESULTS_DIR="individual_results"

# --- Compilação ---
echo "Compilando o arquivo C++: $SOURCE_FILE..."
# -O3 para otimização e -std=c++11 para garantir compatibilidade
g++ -std=c++11 -O3 "$SOURCE_FILE" -o "$EXECUTABLE"

# Verifica se a compilação foi bem-sucedida
if [ $? -ne 0 ]; then
    echo "Erro na compilação. Abortando."
    exit 1
fi
echo "Compilação concluída com sucesso. Executável '$EXECUTABLE' criado."

# --- Execução Paralela ---
# Cria o diretório de resultados se ele não existir
mkdir -p "$RESULTS_DIR"

echo "Encontrando todas as instâncias em '$BASE_DIR' e executando em paralelo..."

# 1. 'find' localiza todos os arquivos 'test.in' dentro de 'BASE_DIR'.
# 2. O pipe '|' envia essa lista para o 'parallel'.
# 3. 'parallel' executa o nosso programa para cada arquivo de entrada.
#    - {} é um placeholder para o caminho do arquivo de entrada.
#    - > redireciona a saída do programa para um arquivo de texto único no diretório de resultados.
#    - O nome do arquivo de saída é gerado a partir do nome da pasta da instância.
find "$BASE_DIR" -name "test.in" | parallel --bar "./$EXECUTABLE {} > $RESULTS_DIR/\$(basename \$(dirname {})).txt"

echo "Execução paralela concluída. Todas as saídas estão em '$RESULTS_DIR'."

# --- Coleta de Resultados ---
echo "Agregando resultados no arquivo CSV: $CSV_FILE..."

# Cria o cabeçalho do arquivo CSV
echo "instance_name,greedy_profit,sa_profit" > "$CSV_FILE"

# Itera sobre cada arquivo de resultado no diretório de saídas
for result_file in "$RESULTS_DIR"/*.txt; do
    # Extrai o nome da instância a partir do nome do arquivo (removendo .txt)
    instance_name=$(basename "$result_file" .txt)

    # Extrai o valor do Greedy usando grep e awk
    # grep 'Greedy:' -> encontra a linha que contém "Greedy:"
    # awk '{print $2}' -> pega a segunda coluna (o número)
    greedy_profit=$(grep 'Greedy:' "$result_file" | awk '{print $2}')

    # Extrai o valor do S.A. da mesma forma
    sa_profit=$(grep 'S.A:' "$result_file" | awk '{print $2}')

    # Adiciona a linha de dados ao arquivo CSV
    echo "$instance_name,$greedy_profit,$sa_profit" >> "$CSV_FILE"
done

echo "Processo finalizado! Os resultados foram salvos em $CSV_FILE."