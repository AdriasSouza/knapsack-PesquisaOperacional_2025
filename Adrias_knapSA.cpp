#include <stdio.h> // print, fgets function
#include <stdlib.h> // exit, atoi function
#include <string.h> //strlen function
#include <math.h> // sqrt function
#include <algorithm>    // std::min
#include <limits.h> // INT_MAX
#include <float.h> //DBL_MAX
#include <time.h> 
#include <vector>   // usado para armazenar e ordenar itens (Greedy)
#include <random>   // gerador moderno de números aleatórios (tweak/SA)
#include <cmath>    // exp() usada no critério de aceitação do SA
#include <chrono>   // medição de tempo por instância
//n_400_c_1000000_g_14_f_0.1_eps_0_s_100
//n - number of itens
//c - capacity
//g - groups of itens
//f - influences the number of items in the different groups (fraction of items in the last group)
//eps - influences in the calculation of profit and weight
//s - numbers sampled between 1 and s

int calculateSolProfit(bool *sol);//calculate the profit of a given solution - invalid solutions get -1
void tweak(bool* sol);
void readFile(const char* fileName);

// Matriz de itens: para cada índice i, itens[i][0] = lucro (profit), itens[i][1] = peso (weight)
// size = número de itens lidos do arquivo; maxWeight = capacidade da mochila
int **itens, size=-1, maxWeight=-1; //[profit, weight]

int main(const int argc, const char **inputFile){
	char fileName[100]="";
	
	// Leitura do arquivo via argumento de linha de comando
	if(argc < 2){ // verifica se foi passado um argumento (caminho do arquivo)
		fprintf(stderr,"use: knapSA <input file>\n\n");
		exit(1);
	}else{
		strcpy(fileName,inputFile[1]); // lê o nome do arquivo a partir de argv[1]
	}

	readFile(fileName);

	bool sol[size];
	for(int i=0; i<size; i++)
		sol[i]=false;

	//Greedy
	// Implementação da Busca Gulosa (custo/benefício): seleciona itens por maior razão lucro/peso
	// Ideia: calcular a razão r = lucro/peso para cada item, ordenar em ordem decrescente e
	// inserir na solução enquanto couber na capacidade restante.

	// Estrutura auxiliar para a busca gulosa com a razão lucro/peso
	struct Item { int idx; int profit; int weight; double ratio; };
	std::vector<Item> items;
	items.reserve(size);
	// Preenche o vetor de itens calculando a razão lucro/peso
	for(int i=0; i<size; ++i){
		int p = itens[i][0];
		int w = itens[i][1];
		double r = (w > 0) ? static_cast<double>(p) / static_cast<double>(w) : DBL_MAX;
		items.push_back({i, p, w, r});
	}
	// Ordena por razão decrescente (maior lucro/peso primeiro). Em empates, maior lucro e depois menor peso.
	std::sort(items.begin(), items.end(), [](const Item& a, const Item& b){
		if(a.ratio == b.ratio){
			// Tie-breaker: higher profit first, then lighter weight
			if(a.profit == b.profit) return a.weight < b.weight;
			return a.profit > b.profit;
		}
		return a.ratio > b.ratio;
	});

	// Percorre os itens na ordem gulosa e adiciona se couber na capacidade restante
	auto greedyStart = std::chrono::high_resolution_clock::now();
	int remainingCapacity = maxWeight;
	for(const auto& it : items){
		if(it.weight <= remainingCapacity){
			sol[it.idx] = true;
			remainingCapacity -= it.weight;
		}
	}

	int greedyProfit = calculateSolProfit(sol);
	auto greedyEnd = std::chrono::high_resolution_clock::now();
	auto greedyMs = std::chrono::duration_cast<std::chrono::milliseconds>(greedyEnd - greedyStart).count();


	bool bestSol[size];
	memcpy(bestSol,sol,sizeof(bool)*size);// melhor conhecida inicia como a gulosa (solução inicial do SA)
// 	S.A.
	//implement simulated annealing

	// Parâmetros do Simulated Annealing (SA)
	// initialTemp: temperatura inicial alta para permitir aceitar piores soluções no começo
	// finalTemp: temperatura final baixa para convergência
	// alpha: taxa de resfriamento (geométrica)
	double initialTemp = 10000.0;
	double finalTemp = 0.1;
	double alpha = 0.99; // cooling rate

	// Gerador aleatório e distribuição uniforme [0,1) para o critério de aceitação probabilístico
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<double> urand(0.0, 1.0);

	// Solução atual e vizinha; começamos a busca a partir da solução gulosa
	bool currentSol[size];
	memcpy(currentSol, sol, sizeof(bool)*size); // começar pela gulosa
	int currentProfit = calculateSolProfit(currentSol);
	int bestProfit = currentProfit;

	bool neighborSol[size];

	// Função auxiliar: mapeia lucro inválido (-1) para um valor bem baixo na comparação do SA
	// Assim, soluções inválidas dificilmente são aceitas, mas ainda podem ser por probabilidade > 0
	auto evalProfit = [](int p){ return (p < 0) ? -1000000000 : p; };

	double temperature = initialTemp;
	auto saStart = std::chrono::high_resolution_clock::now();
	while(temperature > finalTemp){
		memcpy(neighborSol, currentSol, sizeof(bool)*size); // copia a solução atual
		tweak(neighborSol); // aplica mutação (bit flip) para gerar uma solução vizinha

		int neighborProfit = calculateSolProfit(neighborSol);
		int delta = evalProfit(neighborProfit) - evalProfit(currentProfit); // melhora (>=0) ou piora (<0)

		if(delta >= 0){ // se vizinha é melhor (ou igual), aceita determinísticamente
			memcpy(currentSol, neighborSol, sizeof(bool)*size);
			currentProfit = neighborProfit;
		}else{ // caso contrário, aceita com probabilidade exp(delta/temperatura)
			double acceptProb = std::exp(static_cast<double>(delta) / temperature);
			if(urand(rng) < acceptProb){
				memcpy(currentSol, neighborSol, sizeof(bool)*size);
				currentProfit = neighborProfit;
			}
		}

		// Atualiza a melhor solução encontrada até agora
		if(currentProfit > bestProfit){
			bestProfit = currentProfit;
			memcpy(bestSol, currentSol, sizeof(bool)*size);
		}

		temperature *= alpha; // resfriamento geométrico
	}


	int saProfit = calculateSolProfit(bestSol);
	auto saEnd = std::chrono::high_resolution_clock::now();
	auto saMs = std::chrono::duration_cast<std::chrono::milliseconds>(saEnd - saStart).count();
	long long totalMs = static_cast<long long>(greedyMs + saMs);
	// Saída CSV: caminho_da_instancia, lucro_guloso, lucro_sa, tempo_guloso_ms, tempo_sa_ms, tempo_total_ms
	printf("%s,%d,%d,%lld,%lld,%lld\n", inputFile[1], greedyProfit, saProfit, (long long)greedyMs, (long long)saMs, totalMs);

}
void tweak(bool *sol){
	// Mutação do tipo "bit flip": escolhe um índice aleatório e inverte o bit
	// true -> false (remove item) ou false -> true (adiciona item)
	static std::mt19937 rng(std::random_device{}());
	if(size <= 0) return;
	std::uniform_int_distribution<int> dist(0, size - 1);
	int idx = dist(rng);
	sol[idx] = !sol[idx];
}

// Calcula o lucro total de uma solução binária (1=pega item i, 0=não pega)
// Retorna -1 se a solução ultrapassar a capacidade da mochila (solução inválida)
int calculateSolProfit(bool *sol){//calculate the profit of a given solution - invalid solutions get -1
	int profit=0;
	int weight=0;
	for(int i=0; i<size; i++){
		if(sol[i]){
			profit+=itens[i][0];//[profit, weight]
			weight+=itens[i][1];
		}
	}
	if(weight>maxWeight)
		profit=-1;//if invalid solution, invalid profit
	return profit;
}

// Lê o arquivo de instância do problema no formato:
// Linha 0: N (número de itens)
// Linhas 1..N: id lucro peso
// Linha N+1: capacidade (maxWeight)
void readFile(const char* fileName){
	char s[500];
	FILE *stream = fopen(fileName, "r"); //read file (read-only permission)
	if( stream == NULL ){ // if the file was not read, error
		fprintf(stderr,"\nFail to Open File!!\n");
		exit(1);
	}
	int line=-1;
	while(fgets(s,500,stream)){ //read every line (not larger than 500 chars) of the input
		if(strlen(s)-1>0) //remove the last break line of the line
			s[strlen(s)-1]='\0';
		if(('\r'==s[strlen(s)-1]))//in some files there is a carriage return at the end, don't know why. This command removes it
			s[strlen(s)-1]=0;
		
		char* value1 = strtok(s," "); //creating sub-strings separated by space
		char* value2 = strtok(NULL," ");
		char* value3 = strtok(NULL," ");
		line++;
		
		if(line==0){//it is the number o items
			size=atoi(value1);
			itens=(int**)malloc(size * sizeof(int*));			
			for (int i=0;i<size;i++)
				itens[i] = (int*)malloc(2 * sizeof(int));//two collumns [profit, weight]
		}
		
		if(line>0 && line<size+1){//descriptions of the itens [id, profit, weight]
			int id=atoi(value1);
			if(id!=line-1){
				fprintf(stderr,"\nERROR! Invalid knapsack file, aborting!!\n");
				exit(1);
			}
			itens[id][0]=atoi(value2);
			itens[id][1]=atoi(value3);
		}
		if(line==size+1)
			maxWeight=atoi(value1);
	}
}
