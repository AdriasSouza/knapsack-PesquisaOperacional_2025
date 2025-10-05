#include <stdio.h> // print, fgets function
#include <stdlib.h> // exit, atoi function
#include <string.h> //strlen function
#include <math.h> // sqrt function
#include <algorithm>    // std::min
#include <limits.h> // INT_MAX
#include <float.h> //DBL_MAX
#include <time.h> 
#include <vector>   // armazenamento/ordenação de itens
#include <random>   // números aleatórios (tweak/SA)
#include <cmath>    // exp() para aceitação no SA
#include <chrono>   // medição de tempo
//n_400_c_1000000_g_14_f_0.1_eps_0_s_100
//n - number of itens
//c - capacity
//g - groups of itens
//f - influences the number of items in the different groups (fraction of items in the last group)
//eps - influences in the calculation of profit and weight
//s - numbers sampled between 1 and s

long long calculateSolProfit(const bool *sol);//calculate the profit of a given solution - invalid solutions get -1
double calculatePenalizedScore(const bool *sol, double penaltyCoef);
void tweak(bool* sol, int &idx1, int &idx2, bool &twoFlips);
void readFile(const char* fileName);

// itens[i] = {lucro, peso}; size = número de itens; maxWeight = capacidade
long long **itens; int size=-1; long long maxWeight=-1; //[profit, weight]

// RNG global para reprodutibilidade
static std::mt19937 rng;

int main(const int argc, const char **inputFile){
	char fileName[100]="";
	
	// Leitura do arquivo via argumento de linha de comando
	if(argc < 2){ // valida argumento obrigatório
		fprintf(stderr,"use: knapSA <input file>\n\n");
		exit(1);
	}else{
		strcpy(fileName,inputFile[1]); // caminho da instância
	}

	// Parâmetros opcionais
	unsigned int seed = 42; // padrão reprodutível
	long double penaltyFactor = 10.0L; // fator ajustável da penalidade
	for(int ai = 2; ai < argc; ++ai){
		const char* arg = inputFile[ai];
		if(strcmp(arg, "--seed") == 0 || strcmp(arg, "-s") == 0){
			if(ai+1 < argc){ seed = (unsigned int)strtoul(inputFile[++ai], nullptr, 10); }
		}else if(strncmp(arg, "--seed=", 8) == 0){
			seed = (unsigned int)strtoul(arg+8, nullptr, 10);
		}else if(strcmp(arg, "--penalty") == 0 || strcmp(arg, "-p") == 0){
			if(ai+1 < argc){ penaltyFactor = strtold(inputFile[++ai], nullptr); }
		}else if(strncmp(arg, "--penalty=", 10) == 0){
			penaltyFactor = strtold(arg+10, nullptr);
		}
	}
	rng.seed(seed);

	readFile(fileName);

	// Coeficiente de penalização baseado na média lucro/peso dos itens
	long double totalProfit = 0.0L, totalWeight = 0.0L;
	for(int i=0; i<size; ++i){
		totalProfit += static_cast<long double>(itens[i][0]);
		totalWeight += static_cast<long double>(itens[i][1]);
	}
	long double avgProfitPerWeight = (totalWeight > 0.0L) ? (totalProfit / totalWeight) : 1.0L;
	double penaltyCoef = static_cast<double>(avgProfitPerWeight * penaltyFactor);

	bool sol[size];
	for(int i=0; i<size; i++)
		sol[i]=false;

	// Heurística gulosa: ordena por lucro/peso (decrescente) e seleciona enquanto couber.

	// Estrutura auxiliar
	struct Item { int idx; long long profit; long long weight; double ratio; };
	std::vector<Item> items;
	items.reserve(size);
	// Calcula a razão e preenche o vetor
	for(int i=0; i<size; ++i){
		long long p = itens[i][0];
		long long w = itens[i][1];
		double r = (w > 0) ? static_cast<double>(p) / static_cast<double>(w) : DBL_MAX;
		items.push_back({i, p, w, r});
	}
	// Ordena por razão decrescente; empate: maior lucro, depois menor peso.
	std::sort(items.begin(), items.end(), [](const Item& a, const Item& b){
		if(a.ratio == b.ratio){
			// Critério de desempate
			if(a.profit == b.profit) return a.weight < b.weight;
			return a.profit > b.profit;
		}
		return a.ratio > b.ratio;
	});

	// Seleciona itens na ordem gulosa se couberem
	auto greedyStart = std::chrono::high_resolution_clock::now();
	int remainingCapacity = maxWeight;
	for(const auto& it : items){
		if(it.weight <= remainingCapacity){
			sol[it.idx] = true;
			remainingCapacity -= it.weight;
		}
	}

	long long greedyProfit = calculateSolProfit(sol);
	auto greedyEnd = std::chrono::high_resolution_clock::now();
	auto greedyMs = std::chrono::duration_cast<std::chrono::milliseconds>(greedyEnd - greedyStart).count();


	bool bestSol[size];
	// SA inicia com solução zerada (não usar a gulosa como base)
	for(int i=0; i<size; ++i) bestSol[i] = false;
// 	S.A.
	//implement simulated annealing

	// Parâmetros do Simulated Annealing (SA): temperatura inicial/final e taxa de resfriamento (alpha)
	double initialTemp = 10000.0;
	double finalTemp = 0.1;
	double alpha = 0.99; // cooling rate

	// Gerador aleatório e uniforme [0,1)
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<double> urand(0.0, 1.0);

	// Estado atual e vizinho; inicia com solução zerada
	bool currentSol[size];
	for(int i=0; i<size; ++i) currentSol[i] = false; // solução inicial zerada
	long long currentProfit = calculateSolProfit(currentSol);
	long long bestProfit = currentProfit;
	long long currentWeight = 0; // solução zerada tem peso 0
	// Não usar a gulosa como solução inicial do SA

	bool neighborSol[size];

	double temperature = initialTemp;
	double currentScore = calculatePenalizedScore(currentSol, penaltyCoef);
	auto saStart = std::chrono::high_resolution_clock::now();
	while(temperature > finalTemp){
		int innerLoops = (size >= 20) ? (size/2) : 10; // mais tentativas por temperatura
		for(int it = 0; it < innerLoops; ++it){
			memcpy(neighborSol, currentSol, sizeof(bool)*size); // copia a solução atual
			int f1=-1, f2=-1; bool two=false;
			tweak(neighborSol, f1, f2, two); // aplica mutação (bit flip) e devolve índices

			// Avaliação incremental
			long long neighborProfit = currentProfit;
			long long neighborWeight = currentWeight;
			if(f1>=0){
				if(neighborSol[f1]){ neighborProfit += itens[f1][0]; neighborWeight += itens[f1][1]; }
				else{ neighborProfit -= itens[f1][0]; neighborWeight -= itens[f1][1]; }
			}
			if(two && f2>=0){
				if(neighborSol[f2]){ neighborProfit += itens[f2][0]; neighborWeight += itens[f2][1]; }
				else{ neighborProfit -= itens[f2][0]; neighborWeight -= itens[f2][1]; }
			}

			// Score penalizado
			long long excess = (neighborWeight > maxWeight) ? (neighborWeight - maxWeight) : 0;
			double neighborScore = static_cast<double>(neighborProfit) - penaltyCoef * static_cast<double>(excess);
			double delta = neighborScore - currentScore; // melhora/piora no score penalizado

			if(delta >= 0.0){ // melhor (ou igual): aceita
				memcpy(currentSol, neighborSol, sizeof(bool)*size);
				currentProfit = neighborProfit;
				currentWeight = neighborWeight;
				currentScore = neighborScore;
			}else{ // pior: aceita com probabilidade exp(delta/temperatura)
				// Teste equivalente: aceite se delta >= T * ln(u), u ~ U(0,1)
				double u = urand(rng);
				if(u <= 0.0) u = std::numeric_limits<double>::min();
				double threshold = temperature * std::log(u);
				if(delta >= threshold){
					memcpy(currentSol, neighborSol, sizeof(bool)*size);
					currentProfit = neighborProfit;
					currentWeight = neighborWeight;
					currentScore = neighborScore;
				}
			}

			// Atualiza a melhor solução
			if(currentWeight <= maxWeight && currentProfit > bestProfit){
				bestProfit = currentProfit;
				memcpy(bestSol, currentSol, sizeof(bool)*size);
			}
		}
		temperature *= alpha; // resfriamento geométrico
	}


	long long saProfit = calculateSolProfit(bestSol);
	auto saEnd = std::chrono::high_resolution_clock::now();
	auto saMs = std::chrono::duration_cast<std::chrono::milliseconds>(saEnd - saStart).count();
	long long totalMs = static_cast<long long>(greedyMs + saMs);
	// Saída CSV: instancia, lucro_guloso, lucro_sa, tempo_guloso_ms, tempo_sa_ms, tempo_total_ms
	printf("%s,%lld,%lld,%lld,%lld,%lld\n", inputFile[1], greedyProfit, saProfit, (long long)greedyMs, (long long)saMs, totalMs);

}
void tweak(bool *sol, int &idx1, int &idx2, bool &twoFlips){
	// Mutação: bit flip de 1 bit; com 10% de chance, flip de 2 bits distintos
	idx1 = -1; idx2 = -1; twoFlips = false;
	if(size <= 0) return;
	std::uniform_int_distribution<int> dist(0, size - 1);
	std::uniform_real_distribution<double> urand(0.0, 1.0);
	idx1 = dist(rng);
	sol[idx1] = !sol[idx1];
	if(urand(rng) < 0.10 && size > 1){
		idx2 = dist(rng);
		if(idx2 == idx1) idx2 = (idx1 + 1) % size; // garante distinto
		sol[idx2] = !sol[idx2];
		twoFlips = true;
	}
}

// Calcula o lucro total; retorna -1 se ultrapassar a capacidade
long long calculateSolProfit(const bool *sol){//calculate the profit of a given solution - invalid solutions get -1
	long long profit=0;
	long long weight=0;
	for(int i=0; i<size; i++){
		if(sol[i]){
			profit+=itens[i][0];//[profit, weight]
			weight+=itens[i][1];
		}
	}
	if(weight>maxWeight)
		profit=-1;// inválida
	return profit;
}

// Score penalizado: lucro - penaltyCoef * max(0, peso - maxWeight)
double calculatePenalizedScore(const bool *sol, double penaltyCoef){
	long long profit = 0;
	long long weight = 0;
	for(int i=0; i<size; ++i){
		if(sol[i]){
			profit += itens[i][0];
			weight += itens[i][1];
		}
	}
	long long excess = (weight > maxWeight) ? (weight - maxWeight) : 0;
	return static_cast<double>(profit) - penaltyCoef * static_cast<double>(excess);
}

// Leitura do arquivo no formato:
// 0: N (número de itens)
// 1..N: id lucro peso
// N+1: capacidade (maxWeight)
void readFile(const char* fileName){
	char s[500];
	FILE *stream = fopen(fileName, "r"); // abre em modo leitura
	if( stream == NULL ){ // erro de abertura
		fprintf(stderr,"\nFail to Open File!!\n");
		exit(1);
	}
	int line=-1;
	while(fgets(s,500,stream)){ // lê linhas (<= 500 chars)
		if(strlen(s)-1>0) // remove \n final
			s[strlen(s)-1]='\0';
		if(('\r'==s[strlen(s)-1]))// remove \r se existir
			s[strlen(s)-1]=0;
		
		char* value1 = strtok(s," "); // tokens separados por espaço
		char* value2 = strtok(NULL," ");
		char* value3 = strtok(NULL," ");
		line++;
		
		if(line==0){ // número de itens
			size=atoi(value1);
			itens=(long long**)malloc(size * sizeof(long long*));            
			for (int i=0;i<size;i++)
				itens[i] = (long long*)malloc(2 * sizeof(long long)); // [profit, weight]
		}
		
		if(line>0 && line<size+1){ // itens: id, lucro, peso
			int id=atoi(value1);
			if(id!=line-1){
				fprintf(stderr,"\nERROR! Invalid knapsack file, aborting!!\n");
				exit(1);
			}
			itens[id][0]=atoll(value2);
			itens[id][1]=atoll(value3);
		}
		if(line==size+1)
			maxWeight=atoll(value1);
	}
}
