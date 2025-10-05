#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>
#include <algorithm>
#include <numeric>
#include <cstring>
#include <cstdlib>

namespace fs = std::filesystem;

// Estrutura para representar um item com suas propriedades
struct Item {
    int id;
    long long profit;
    long long weight;
    double ratio; // razão lucro/peso
    
    Item(int i, long long p, long long w) : id(i), profit(p), weight(w) {
        ratio = (w > 0) ? static_cast<double>(p) / static_cast<double>(w) : 0.0;
    }
};

// Variáveis globais para os dados da instância
std::vector<std::vector<long long>> itens;
int size = -1;
long long maxWeight = -1;

// Gerador de números aleatórios global
std::mt19937 rng(std::random_device{}());

/**
 * Lê um arquivo de instância do problema da mochila
 * Formato: primeira linha = número de itens, linhas seguintes = id lucro peso, última linha = capacidade
 */
bool readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir arquivo: " << fileName << std::endl;
        return false;
    }
    
    std::string line;
    int lineNum = 0;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Remove carriage return se presente
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        std::istringstream iss(line);
        
        if (lineNum == 0) {
            // Primeira linha: número de itens
            iss >> size;
            itens.clear();
            itens.resize(size, std::vector<long long>(2));
        }
        else if (lineNum <= size) {
            // Linhas dos itens: id lucro peso
            int id;
            long long profit, weight;
            iss >> id >> profit >> weight;
            
            if (id != lineNum - 1) {
                std::cerr << "Erro: ID inválido no arquivo. Esperado: " << (lineNum-1) << ", encontrado: " << id << std::endl;
                return false;
            }
            
            itens[id][0] = profit; // lucro
            itens[id][1] = weight; // peso
        }
        else if (lineNum == size + 1) {
            // Última linha: capacidade da mochila
            iss >> maxWeight;
        }
        
        lineNum++;
    }
    
    file.close();
    return true;
}

/**
 * Calcula o lucro total de uma solução
 * Retorna -1 se a solução for inválida (excede capacidade)
 */
long long calculateSolProfit(const std::vector<bool>& sol) {
    long long profit = 0;
    long long weight = 0;
    
    for (int i = 0; i < size; i++) {
        if (sol[i]) {
            profit += itens[i][0]; // lucro
            weight += itens[i][1]; // peso
        }
    }
    
    return (weight > maxWeight) ? -1 : profit;
}

/**
 * Implementação da Busca Gulosa
 * Ordena itens por razão lucro/peso (decrescente) e seleciona enquanto couber
 */
long long greedy_search() {
    // Cria vetor de itens com razões calculadas
    std::vector<Item> items;
    items.reserve(size);
    
    for (int i = 0; i < size; i++) {
        items.emplace_back(i, itens[i][0], itens[i][1]);
    }
    
    // Ordena por razão lucro/peso (decrescente)
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        if (a.ratio == b.ratio) {
            // Critério de desempate: maior lucro primeiro, depois menor peso
            if (a.profit == b.profit) {
                return a.weight < b.weight;
            }
            return a.profit > b.profit;
        }
        return a.ratio > b.ratio;
    });
    
    // Constrói a solução gulosa
    std::vector<bool> solution(size, false);
    long long remainingCapacity = maxWeight;
    
    for (const auto& item : items) {
        if (item.weight <= remainingCapacity) {
            solution[item.id] = true;
            remainingCapacity -= item.weight;
        }
    }
    
    return calculateSolProfit(solution);
}

/**
 * Função de mutação para o Simulated Annealing
 * Realiza bit flip: inverte o valor de um índice aleatório
 */
void tweak(std::vector<bool>& sol) {
    if (size <= 0) return;
    
    std::uniform_int_distribution<int> dist(0, size - 1);
    int idx = dist(rng);
    sol[idx] = !sol[idx];
}

/**
 * Implementação do Simulated Annealing
 * Inicia com solução vazia e busca melhorias através de perturbações
 */
long long simulated_annealing() {
    // Parâmetros do SA
    double temperature = 10000.0;    // temperatura inicial
    double min_temp = 0.01;          // temperatura mínima
    double alpha = 0.9995;           // taxa de resfriamento
    
    // Inicialização: solução vazia (mochila vazia)
    std::vector<bool> currentSol(size, false);
    std::vector<bool> bestSol(size, false);
    std::vector<bool> neighborSol(size);
    
    long long currentProfit = 0;  // solução vazia tem lucro 0
    long long bestProfit = 0;     // melhor solução inicial também é 0
    
    // Distribuição uniforme para critério de aceitação
    std::uniform_real_distribution<double> uniform(0.0, 1.0);
    
    // Loop principal do Simulated Annealing
    while (temperature > min_temp) {
        // Gera solução vizinha
        neighborSol = currentSol;
        tweak(neighborSol);
        
        long long neighborProfit = calculateSolProfit(neighborSol);
        
        // Se a solução vizinha é inválida, trata como lucro muito baixo
        long long evalNeighbor = (neighborProfit < 0) ? -1000000000LL : neighborProfit;
        long long evalCurrent = (currentProfit < 0) ? -1000000000LL : currentProfit;
        
        long long delta = evalNeighbor - evalCurrent;
        
        // Critério de aceitação
        bool accept = false;
        if (delta >= 0) {
            // Solução melhor ou igual: aceita
            accept = true;
        } else {
            // Solução pior: aceita com probabilidade exp(delta/temperature)
            double acceptProb = std::exp(static_cast<double>(delta) / temperature);
            accept = (uniform(rng) < acceptProb);
        }
        
        if (accept) {
            currentSol = neighborSol;
            currentProfit = neighborProfit;
        }
        
        // Atualiza a melhor solução encontrada (apenas se for válida)
        if (currentProfit > bestProfit && currentProfit >= 0) {
            bestProfit = currentProfit;
            bestSol = currentSol;
        }
        
        // Resfriamento
        temperature *= alpha;
    }
    
    return bestProfit;
}

int main(int argc, char* argv[]) {
    // Verifica argumentos de linha de comando
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <diretorio_de_instancias> <arquivo_saida.csv>" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " ./problemInstances resultados.csv" << std::endl;
        return 1;
    }
    
    std::string instancesDir = argv[1];
    std::string outputFile = argv[2];
    
    // Verifica se o diretório existe
    if (!fs::exists(instancesDir) || !fs::is_directory(instancesDir)) {
        std::cerr << "Erro: Diretório não encontrado: " << instancesDir << std::endl;
        return 1;
    }
    
    // Abre arquivo de saída CSV
    std::ofstream csvFile(outputFile);
    if (!csvFile.is_open()) {
        std::cerr << "Erro ao criar arquivo de saída: " << outputFile << std::endl;
        return 1;
    }
    
    // Escreve cabeçalho do CSV
    csvFile << "instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms" << std::endl;
    
    // Coleta todos os arquivos de instância
    std::vector<fs::path> instanceFiles;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(instancesDir)) {
            if (entry.is_regular_file() && entry.path().filename() == "test.in") {
                instanceFiles.push_back(entry.path());
            }
        }
    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Erro ao acessar diretório: " << ex.what() << std::endl;
        return 1;
    }
    
    // Ordena os arquivos para processamento consistente
    std::sort(instanceFiles.begin(), instanceFiles.end());
    
    std::cout << "Encontradas " << instanceFiles.size() << " instâncias para processar." << std::endl;
    
    // Processa cada instância
    for (size_t i = 0; i < instanceFiles.size(); i++) {
        const auto& instancePath = instanceFiles[i];
        
        std::cout << "Processando (" << (i+1) << "/" << instanceFiles.size() << "): " 
                  << instancePath << std::endl;
        
        // Lê a instância
        if (!readFile(instancePath.string())) {
            std::cerr << "Erro ao ler instância: " << instancePath << std::endl;
            continue;
        }
        
        // Executa Busca Gulosa e mede tempo
        auto start = std::chrono::high_resolution_clock::now();
        long long greedyProfit = greedy_search();
        auto end = std::chrono::high_resolution_clock::now();
        auto greedyTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Executa Simulated Annealing e mede tempo
        start = std::chrono::high_resolution_clock::now();
        long long saProfit = simulated_annealing();
        end = std::chrono::high_resolution_clock::now();
        auto saTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Escreve resultado no CSV
        csvFile << instancePath.string() << "," 
                << greedyProfit << "," 
                << saProfit << "," 
                << greedyTime.count() << "," 
                << saTime.count() << std::endl;
        
        // Libera memória (os vetores são automaticamente limpos na próxima iteração)
        itens.clear();
    }
    
    csvFile.close();
    std::cout << "Processamento concluído. Resultados salvos em: " << outputFile << std::endl;
    
    return 0;
}