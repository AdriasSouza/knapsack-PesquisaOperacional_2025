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

int calculateSolProfit(bool *sol);//calculate the profit of a given solution - invalid solutions get -1
void tweak(bool* sol);
void readFile(const char* fileName);

int **itens, size=-1, maxWeight=-1; //[profit, weight]

int main(const int argc, const char **inputFile){
	char fileName[100]="";
    
	if(argc < 2){ // verify if an argument was passed (the first argument in C is the name of the executable)
		fprintf(stderr,"use: knapSA <input file>\n\n");
		exit(1);
	}else{
		strcpy(fileName,inputFile[1]); // read filename from argv
	}

	readFile(fileName);

	bool sol[size];
	for(int i=0; i<size; i++)
		sol[i]=false;

	//Greedy
	//implement greedy search based on ratio weight/profit, the smaller the better

	printf("best greedy sol: ");
	for(int i=0;i<size;i++)//print all sol positions
		printf("%d", sol[i]);
	printf("\nGreedy: %d\n",calculateSolProfit(sol));
	
	for(int i=0; i<size; i++)//reset sol
		sol[i]=false;

	bool bestSol[size];
	memcpy(bestSol,sol,sizeof(bool)*size);//copy sol to new sol
// 	S.A.
	//implement simulated annealing

    printf("best S.A sol: ");
    for(int i=0;i<size;i++)//percorre todas as posições da sol
        printf("%d", bestSol[i]);
    printf("\nS.A: %d\n",calculateSolProfit(bestSol));

}
void tweak(bool *sol){
	//implement
	//bit flip mutation
}

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
