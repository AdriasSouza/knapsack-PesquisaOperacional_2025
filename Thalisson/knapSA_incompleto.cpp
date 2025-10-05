#include <stdio.h> // print, fgets function
#include <stdlib.h> // exit, atoi function
#include <string.h> //strlen function
#include <math.h> // sqrt function
#include <algorithm>    // std::min
#include <limits.h> // INT_MAX
#include <float.h> //DBL_MAX
#include <time.h> 
//n_400_c_1000000_g_14_f_0.1_eps_0_s_100
//n - number of itens
//c - capacity
//g - groups of itens
//f - influences the number of items in the different groups (fraction of items in the last group)
//eps - influences in the calculation of profit and weight
//s - numbers sampled between 1 and s

long long int calculateSolProfit(bool *sol);//calculate the profit of a given solution - invalid solutions get -1
void tweak(bool* sol);
void readFile(const char* fileName);

long long int **itens; int size=-1; long long int maxWeight=-1; //[profit, weight]

int main(const int argc, const char **inputFile){
	char fileName[100]="";
		
	// if(argc < 2){ //verify if an argument was passed (the first argument in C is the name of the executable)
	// 	fprintf(stderr,"use: knapSA <input file>\n\n");
	// 	exit(1);
	// }else
	// 	strcpy(fileName,inputFile[1]);//read filename from stdin
	
	strcpy(fileName,"problemInstances/n_1000_c_100000000_g_10_f_0.3_eps_1e-05_s_300/test.in");//put a fixed filename  - 1005410
	// strcpy(fileName,"problemInstances/n_1200_c_1000000_g_14_f_0.1_eps_0_s_100/test.in");//put a fixed filename - no solution

	readFile(fileName);

	bool sol[size];
	for(int i=0; i<size; i++)
		sol[i]=false;

	//Greedy
	//implement greedy search based on ratio weight/profit, the smaller the better

	// Estrutura para armazenar os itens junto com sua razão peso/lucro e índice original.
	// Isso é necessário para ordenar os itens sem perder a referência ao seu índice original.
	struct ItemInfo {
		int id;
		double ratio; // Razão
	};

	// Aloca dinamicamente um array de ItemInfo para armazenar as informações de cada item.
	ItemInfo* itemRatios = (ItemInfo*)malloc(size * sizeof(ItemInfo));

	// Calcula a razão peso/lucro para cada item e armazena junto com seu ID original.
	// A razão é calculada como peso / lucro. Itens com lucro zero ou negativo são tratados para evitar divisão por zero
	// ou para serem considerados menos desejáveis (razão alta).
	for (int i = 0; i < size; i++) {
		itemRatios[i].id = i;
		if (itens[i][0] > 0) { // Se o lucro for positivo, calcula a razão peso/lucro.
			itemRatios[i].ratio = (double)itens[i][1] / itens[i][0]; // peso / lucro
		} else { // Se o lucro for zero ou negativo, atribui uma razão muito alta para que seja selecionado por último (ou nunca).
			itemRatios[i].ratio = DBL_MAX; // DBL_MAX é um valor muito grande definido em <float.h>
		}
	}

	// Ordena os itens com base na razão peso/lucro em ordem crescente (menor razão primeiro).
	// Isso significa que itens com menor peso por unidade de lucro (ou maior lucro por unidade de peso) serão priorizados.
	for (int i = 0; i < size - 1; i++) {
		for (int j = i + 1; j < size; j++) {
			if (itemRatios[i].ratio > itemRatios[j].ratio) {
				ItemInfo temp = itemRatios[i];
				itemRatios[i] = itemRatios[j];
				itemRatios[j] = temp;
			}
		}
	}

	// Solução gulosa foi iniciada na linha 37 deste código onde nenhum item é selecionado inicialmente.
	
	// Aqui vamos definir e o peso atual da mochila.
	long long int currentWeight = 0;

	// Percorre os itens ordenados e os adiciona à mochila se houver capacidade.
	for (int i = 0; i < size; i++) {
		int itemId = itemRatios[i].id;
		// Verifica se o item pode ser adicionado sem exceder a capacidade máxima da mochila.
		if (currentWeight + itens[itemId][1] <= maxWeight) {
			sol[itemId] = true; // Adiciona o item à solução.
			currentWeight += itens[itemId][1]; // Atualiza o peso atual da mochila.
		}
	}

	// printf("best greedy sol: ");  //Não sei se isso aqui é interessante, portanto vou deixar comentado para o print do resultado ficar mais limpo
	// for(int i=0;i<size;i++)//print all sol positions
	// 	printf("%d", sol[i]);

	printf("\nGreedy: %lld\n",calculateSolProfit(sol));
	
	for(int i=0; i<size; i++)//reset sol
		sol[i]=false;

	bool bestSol[size];
	memcpy(bestSol,sol,sizeof(bool)*size);//copy sol to new sol

	// 	S.A.
	//implement simulated annealing
	
	// Inicializa o gerador de números pseudo-aleatórios com a hora atual.
	srand(time(NULL));

	double t = 1000.0; // t: temperatura, inicialmente um número alto
	double taxaResfriamento = 0.9; 	// Taxa de resfriamento para diminuir a temperatura
	
	// Limite máximo de iterações para o loop principal do SA (para evitar loops infinitos)
	int maxIteracoes = 100000;
	int iteracaoAtual = 0;

	// S: Solução candidata inicial
	long long int qualidadeS = calculateSolProfit(sol);

	long long int qualidadeBest = qualidadeS;

	// Loop principal do Simulated Annealing: repeat...until (Pseudocódigo do Luke 2015)
	// A condição de parada será t <= 0 ou um limite de iterações.
	while (t > 0 && iteracaoAtual < maxIteracoes) {
		// S será sol
		// R <- Tweak(Copy(S))
		bool R[size];
		memcpy(R, sol, sizeof(bool) * size); // Copia S para R
		tweak(R); // Aplica a mutação (bit flip) na solução vizinha R

		long long int qualidadeR = calculateSolProfit(R);

		// Verifica se a solução vizinha R é válida (não excede o peso máximo).
		if (qualidadeR != -1) { // Se a solução vizinha é válida
			// if Quality(R) > Quality(S) or if a random number chosen from 0 to 1 < e Quality(R)-Quality(S)/t then (Pseudocódigo do Luke 2015)
			long long int deltaQualidade = qualidadeR - qualidadeS;
			
			//Regra de metropolis
			// rand() / RAND_MAX gera um número aleatório entre 0 e 1.
			if (deltaQualidade > 0 || ( (double)rand() / RAND_MAX ) < exp((double)deltaQualidade / t)) {
				// S <- R
				memcpy(sol, R, sizeof(bool) * size);
				qualidadeS = qualidadeR;
			}
		}

		// Decrease t
		t *= taxaResfriamento; 
		iteracaoAtual++; // Incrementa o contador de iterações

		// if Quality(S) > Quality(Best) then
		if (qualidadeS > qualidadeBest) {
			// Best <- S
			memcpy(bestSol, sol, sizeof(bool) * size);
			qualidadeBest = qualidadeS;
		}
	}

    // printf("best S.A sol: "); //Comentei para uma saída mais limpa
    // for(int i=0;i<size;i++)//percorre todas as posições da sol
    //     printf("%d", bestSol[i]);

    printf("\nS.A: %lld\n",calculateSolProfit(bestSol));

}

void tweak(bool *sol){
	//implement
	//bit flip mutation
	// Seleciona um índice aleatório dentro do tamanho da solução (size).
	// A função rand() é usada para gerar um número pseudo-aleatório.
	// O operador % size garante que o índice esteja dentro dos limites válidos [0, size-1].
	int itemParaFlip = rand() % size;

	// Inverte o estado do item selecionado (se estava incluído, agora não está; e vice-versa).
	sol[itemParaFlip] = !sol[itemParaFlip];
}

long long int calculateSolProfit(bool *sol){//calculate the profit of a given solution - invalid solutions get -1
	long long int profit=0;
	long long int weight=0;
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
			itens=(long long int**)malloc(size * sizeof(long long int*));			
			for (int i=0;i<size;i++)
				itens[i] = (long long int*)malloc(2 * sizeof(long long int));//two collumns [profit, weight]
		}
		
		if(line>0 && line<size+1){//descriptions of the itens [id, profit, weight]
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