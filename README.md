# Análise Heurística para o Problema da Mochila 0-1

## Contexto acadêmico

Este projeto foi desenvolvido como parte da avaliação da disciplina de **Pesquisa Operacional**, ministrada pelo **Prof. Dr. Olacir Rodrigues Castro Junior**, para o curso de **Bacharelado em Sistemas de Informação** da **Universidade Federal do Acre (UFAC)**.

## Sobre o projeto

Este repositório contém uma ferramenta de linha de comando em C++ para resolver o **Problema da Mochila 0-1 (0-1 Knapsack Problem)** e produzir resultados em **formato CSV**, facilitando a análise em lote.

- Conjunto de testes: foram utilizadas **3.240 instâncias de alta complexidade**, propostas no artigo “A new class of hard problem instances for the 0-1 knapsack problem”.
- Algoritmos implementados em C++:
  - **Busca Gulosa (Greedy Search)**: heurística que prioriza itens com a maior **razão lucro/peso**.
  - **Simulated Annealing (S.A.)**: meta-heurística com mutação do tipo **bit flip** para escapar de ótimos locais. Parâmetros padrão: temperatura inicial 10.000, taxa de resfriamento (alpha) 0,99, temperatura final 0,1.
- A aplicação foi estruturada para análise em lote: lê o caminho da instância via argumento e emite **uma linha CSV por execução**.
- A implementação atual também registra os **tempos de execução** (em milissegundos) do Greedy e do S.A., além do tempo total.

> Fonte dos datasets: [knapsackProblemInstances](https://github.com/JorikJooken/knapsackProblemInstances)

## Como executar

### Pré-requisitos

- **Compilador C++** (g++ ou compatível) com suporte a **C++11** ou superior.
- Ambiente Linux/macOS ou **WSL** no Windows (alternativamente, **PowerShell** + g++ no Windows).

### Execução em lote (Bash: Linux, macOS, WSL, Git Bash)

1. Conceda permissão de execução ao script:

```bash
chmod +x run_analysis.sh
```

2. Execute a análise:

```bash
./run_analysis.sh
```

#### O que o script faz

- Compila o código C++ (`knapSA_incompleto.cpp`) gerando o executável `knapSA`.
- Cria (ou limpa) o arquivo `resultados.csv` com o cabeçalho apropriado.
- Localiza todas as instâncias `test.in` dentro de `problemInstances/` e executa `./knapSA <instância>` para cada uma das **3.240** instâncias, exibindo o progresso no terminal.
- Anexa a saída (uma linha CSV por instância) ao arquivo `resultados.csv`.

### Execução em lote (Windows PowerShell)

1) Execute o script PowerShell:

```powershell
./run_analysis.ps1
```

O script compila, executa todas as instâncias e gera o mesmo `resultados.csv` com cabeçalho.

### Execução manual (uma instância)

Compilar e executar diretamente:

```bash
# Exemplo (Bash):
g++ -O2 -std=c++17 -Wall -Wextra -o knapSA knapSA_incompleto.cpp
./knapSA problemInstances/n_400_c_1000000_g_14_f_0.1_eps_0_s_100/test.in
```

```powershell
# Exemplo (PowerShell):
g++ -O2 -std=c++17 -Wall -Wextra -o knapSA knapSA_incompleto.cpp
./knapSA.exe "problemInstances/n_400_c_1000000_g_14_f_0.1_eps_0_s_100/test.in"
```

## Saída e formato do CSV

Ao final, todos os resultados estarão consolidados em `resultados.csv`.

- Estrutura base (requisito mínimo):

```text
instancia,lucro_guloso,lucro_sa
```

- Estrutura efetivamente produzida pela implementação atual (inclui tempos de execução em milissegundos):

```text
instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms,tempo_total_ms
```

Cada linha corresponde a uma instância processada. Exemplo:

```text
problemInstances/n_400_c_1000000_g_14_f_0.1_eps_0_s_100/test.in,1005410,1008800,3,47,50
```

Onde:

- `instancia`: caminho do arquivo da instância.
- `lucro_guloso`: lucro obtido pela heurística gulosa.
- `lucro_sa`: lucro da melhor solução encontrada pelo Simulated Annealing.
- `tempo_guloso_ms`: tempo gasto no Greedy (ms).
- `tempo_sa_ms`: tempo gasto no S.A. (ms).
- `tempo_total_ms`: soma dos tempos do Greedy e do S.A. (ms).

## Observações

- Os parâmetros do Simulated Annealing podem ser ajustados no código-fonte (temperatura inicial/final e taxa de resfriamento).
- Para reprodutibilidade, considere fixar a semente do gerador aleatório (atualmente usa `std::random_device`).
- Os scripts assumem que as instâncias estão no diretório `problemInstances/` e possuem arquivos chamados `test.in`.
