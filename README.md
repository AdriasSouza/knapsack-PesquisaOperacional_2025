# Análise Heurística para o Problema da Mochila 0-1

## Contexto acadêmico

Este projeto foi desenvolvido como parte da avaliação da disciplina de **Pesquisa Operacional**, ministrada pelo **Prof. Dr. Olacir Rodrigues Castro Junior**, para o curso de **Bacharelado em Sistemas de Informação** da **Universidade Federal do Acre (UFAC)**.

## Sobre o projeto

Este repositório contém uma ferramenta de linha de comando em C++ para resolver o **Problema da Mochila 0-1 (0-1 Knapsack Problem)** e produzir resultados em **formato CSV**, facilitando a análise em lote. A implementação de referência está em `Adrias/Adrias_knapSA.cpp`.

- Conjunto de testes: foram utilizadas **3.240 instâncias de alta complexidade**, propostas no artigo “A new class of hard problem instances for the 0-1 knapsack problem”.
- Algoritmos implementados em C++:
  - **Busca Gulosa (Greedy)**: ordena por razão **lucro/peso** (decrescente) e seleciona enquanto houver capacidade. Desempate por maior lucro e, depois, menor peso.
  - **Simulated Annealing (SA)**: busca estocástica com aceitação por Metropolis. Nesta versão:
    - A solução inicial do SA é **zerada** (não utiliza a gulosa como base).
    - A avaliação usa **score penalizado**: score = lucro − coef_penal × excessoDePeso.
    - O coeficiente de penalidade é derivado da média lucro/peso dos itens (multiplicada por 10.0), podendo ser ajustado no código.
    - Operador de vizinhança: **bit flip** de 1 bit, com 10% de chance de flipar **2 bits distintos**.
    - Parâmetros padrão: temperatura inicial 10.000, alpha 0,99, temperatura final 0,1; RNG `std::mt19937` sem semente fixa (usa `random_device`).
- A aplicação lê o caminho da instância via argumento e emite **uma linha CSV por execução**.
- A implementação registra os **tempos de execução** (ms) do Greedy e do SA, além do tempo total.

> Fonte dos datasets: [knapsackProblemInstances](https://github.com/JorikJooken/knapsackProblemInstances)

## Como executar

### Pré-requisitos

- **Compilador C++** (g++ ou compatível) com suporte a **C++11** ou superior.
- Ambiente Linux/macOS ou **WSL** no Windows (alternativamente, **PowerShell** + g++ no Windows).

### Execução em lote (Bash: Linux, macOS, WSL, Git Bash)

1. Conceda permissão de execução ao script:

```bash
chmod +x Adrias/Adrias_run_analysis.sh
```

1. Execute a análise:

```bash
./Adrias/Adrias_run_analysis.sh
```

#### O que o script faz

- Compila `Adrias/Adrias_knapSA.cpp` (otimizações) e gera `knapSA` no diretório raiz.
- Cria (ou limpa) o arquivo `resultados.csv` com o cabeçalho apropriado (6 colunas).
- Localiza todas as instâncias `test.in` em `problemInstances/` e executa `./knapSA <instância>` para cada uma, exibindo progresso.
- Anexa a saída (uma linha CSV por instância) ao arquivo `resultados.csv`.

### Execução em lote (Windows PowerShell)

1) Execute o script PowerShell:

```powershell
./Adrias/Adrias_run_analysis.ps1
```

O script compila, executa todas as instâncias e gera o `resultados.csv` (6 colunas) no diretório raiz.

### Execução manual (uma instância)

Compilar e executar diretamente:

```bash
# Exemplo (Bash):
g++ -O2 -std=c++17 -Wall -Wextra -o knapSA Adrias/Adrias_knapSA.cpp
./knapSA problemInstances/n_400_c_1000000_g_14_f_0.1_eps_0_s_100/test.in
```

```powershell
# Exemplo (PowerShell):
g++ -O2 -std=c++17 -Wall -Wextra -o knapSA Adrias/Adrias_knapSA.cpp
./knapSA.exe "problemInstances/n_400_c_1000000_g_14_f_0.1_eps_0_s_100/test.in"
```

## Saída e formato do CSV

Ao final, todos os resultados estarão consolidados em `resultados.csv`.

- Estrutura produzida pela implementação atual (inclui tempos de execução em milissegundos):

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

- Parâmetros do SA podem ser ajustados no código-fonte (`initialTemp`, `finalTemp`, `alpha`) e no cálculo do **coeficiente de penalidade** (média lucro/peso × 10.0 por padrão).
- Para reprodutibilidade, considere fixar a semente do gerador (atualmente usa `std::random_device`).
- Os scripts assumem que as instâncias estão no diretório `problemInstances/` e possuem arquivos chamados `test.in`.

### Scripts auxiliares

- Há também um script simples para o código de partida (`knapSA_incompleto.cpp`): `run_knapSA_incompleto.ps1`.
  - Ele compila, executa e extrai os lucros de Greedy e SA da saída textual, gerando `resultados_incompleto.csv`.
  - Use apenas se precisar comparar com a versão incompleta; a versão principal para análise em lote é `Adrias/Adrias_knapSA.cpp` com os scripts de `Adrias/`.
