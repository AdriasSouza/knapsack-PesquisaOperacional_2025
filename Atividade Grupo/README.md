# Solução para o Problema da Mochila 0-1 com Busca Gulosa e Simulated Annealing

## Descrição

Este programa implementa duas meta-heurísticas para resolver o **Problema da Mochila 0-1 (0-1 Knapsack Problem)**:

1. **Busca Gulosa (Greedy Search)**: Seleciona itens com base na melhor razão lucro/peso
2. **Simulated Annealing (SA)**: Meta-heurística que utiliza perturbações aleatórias e critério probabilístico de aceitação

O programa processa automaticamente todas as instâncias de teste em um diretório, mede os tempos de execução de cada algoritmo e gera um arquivo CSV com os resultados.

## Como Compilar

Para compilar o programa, utilize o seguinte comando (requer C++17 para suporte a `<filesystem>`):

```bash
g++ -std=c++17 -O2 -o knapSA_solver knapSA_solver.cpp
```

### Pré-requisitos

- Compilador C++ com suporte a C++17 (g++ 8+ ou equivalente)
- Sistema operacional com suporte a filesystem (Linux, macOS, Windows com MinGW)

## Como Executar

O programa requer dois argumentos de linha de comando:

```bash
./knapSA_solver <caminho_para_diretorio_de_instancias> <nome_do_arquivo_de_saida.csv>
```

### Exemplo de Execução

```bash
./knapSA_solver ./problemInstances resultados.csv
```

Este comando:

- Processa todas as instâncias (arquivos `test.in`) encontradas no diretório `./problemInstances` (incluindo subdiretórios)
- Executa ambos os algoritmos em cada instância
- Salva os resultados no arquivo `resultados.csv`

## Formato do Arquivo de Saída

O arquivo CSV gerado contém as seguintes colunas:

- `instancia`: Caminho completo do arquivo de instância
- `lucro_guloso`: Lucro obtido pelo algoritmo guloso
- `lucro_sa`: Lucro obtido pelo Simulated Annealing
- `tempo_guloso_ms`: Tempo de execução do algoritmo guloso (em milissegundos)
- `tempo_sa_ms`: Tempo de execução do Simulated Annealing (em milissegundos)

### Exemplo de Saída CSV

```csv
instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms
./problemInstances/n_400_c_1000000_g_14_f_0.1_eps_0_s_100/test.in,1005410,1008800,2,45
./problemInstances/n_800_c_1000000_g_2_f_0.3_eps_0.01_s_300/test.in,545077,547200,4,89
```

## Descrição dos Algoritmos

### Busca Gulosa (Greedy Search)

A implementação da busca gulosa segue esta estratégia:

1. **Cálculo da Razão**: Para cada item, calcula a razão lucro/peso
2. **Ordenação**: Ordena todos os itens em ordem decrescente pela razão lucro/peso
3. **Seleção**: Percorre os itens ordenados e adiciona à mochila aqueles que couberem na capacidade restante
4. **Critério de Desempate**: Em caso de razões iguais, prioriza itens com maior lucro e, em segundo critério, menor peso

Este algoritmo é rápido (O(n log n)) e produz boas soluções para muitas instâncias, mas não garante otimalidade global.

### Simulated Annealing (SA)

A implementação do Simulated Annealing utiliza os seguintes parâmetros e características:

- **Solução Inicial**: Mochila vazia (todos os itens = false)
- **Temperatura Inicial**: 10.000,0
- **Temperatura Mínima**: 0,01
- **Taxa de Resfriamento**: 0,9995 (multiplicativa)
- **Operador de Vizinhança**: Bit flip - inverte aleatoriamente o estado de um item (incluído/excluído)
- **Critério de Aceitação**: Aceita soluções melhores deterministicamente; aceita soluções piores com probabilidade exp(Δ/T)

O algoritmo continua até que a temperatura atinja o valor mínimo, explorando o espaço de soluções de forma controlada e permitindo escapar de ótimos locais através da aceitação probabilística de soluções piores.

## Estrutura do Projeto

```text
Atividade Grupo/
├── knapSA_solver.cpp    # Código fonte principal
└── README.md           # Este arquivo de documentação
```

## Observações Técnicas

- O programa utiliza inteiros de 64 bits (`long long`) para evitar overflow em instâncias com valores grandes
- A geração de números aleatórios utiliza `std::mt19937` para melhor qualidade estatística
- O programa processa automaticamente todos os arquivos `test.in` encontrados recursivamente no diretório especificado
- Soluções inválidas (que excedem a capacidade) são tratadas com lucro -1 e penalizadas no SA
- A medição de tempo utiliza `std::chrono::high_resolution_clock` para maior precisão
