// knapSA.cpp
// Compile: g++ -O3 -std=c++11 knapSA.cpp -o knapSA

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <ctime>
#include <vector>
#include <utility>
#include <algorithm>
#include <limits>

using ll = long long;

ll calculateSolProfitBool(const std::vector<char> &sol); // returns -1 if infeasible? we'll use ll and check weight separately
double calculatePenalizedScore(const std::vector<char> &sol, double penalty_coef, ll maxWeight);
void tweak(std::vector<char> &sol);
void readFile(const char* fileName);

// items: itens[i][0] = profit, itens[i][1] = weight
ll **itens = nullptr;
int sizeItems = -1;
ll maxWeight = -1;

// helper to copy
static inline void copySol(std::vector<char> &dst, const std::vector<char> &src){
    std::memcpy(dst.data(), src.data(), sizeof(char)*src.size());
}

int main(int argc, char **argv){
    if(argc < 2){
        std::fprintf(stderr, "Usage: %s <path/to/test.in>\n", argv[0]);
        return 1;
    }
    const char *fileName = argv[1];
    readFile(fileName);

    if(sizeItems <= 0){
        std::fprintf(stderr,"Invalid instance size\n");
        return 1;
    }

    // compute some stats for penalty calculation
    long double totalProfit = 0.0L, totalWeight = 0.0L;
    for(int i=0;i<sizeItems;i++){
        totalProfit += (long double)itens[i][0];
        totalWeight += (long double)itens[i][1];
    }
    long double avgProfitPerWeight = 1.0L;
    if(totalWeight > 0.0L) avgProfitPerWeight = totalProfit / totalWeight;
    // penalty coefficient per unit overweight:
    // factor (tunable) - larger factor penalizes infeasible more strongly.
    long double penalty_factor = 10.0L;
    double penalty_coef = (double)(avgProfitPerWeight * penalty_factor);

    // -- GREEDY (profit/weight descending)
    std::vector<std::pair<long double,int>> order; order.reserve(sizeItems);
    for(int i=0;i<sizeItems;i++){
        long double ratio = (long double)itens[i][0] / (long double)itens[i][1];
        order.emplace_back(-ratio, i); // negative so std::sort ascending gives highest ratio first
    }
    std::sort(order.begin(), order.end());

    std::vector<char> greedySol(sizeItems, 0);
    long double curWeight = 0.0L;
    for(size_t k=0;k<order.size();k++){
        int idx = order[k].second;
        if(curWeight + (long double)itens[idx][1] <= (long double)maxWeight){
            greedySol[idx] = 1;
            curWeight += (long double)itens[idx][1];
        }
    }

    ll greedyProfit = calculateSolProfitBool(greedySol);
    printf("best greedy sol: ");
    for(int i=0;i<sizeItems;i++) printf("%d", greedySol[i]?1:0);
    printf("\nGreedy: %lld\n", greedyProfit);

    // ---------- Simulated Annealing with penalization ----------
    // SA parameters (tune as needed)
    double T = 1000.0;
    double alpha = 0.995;
    int maxIter = 200000;
    if(sizeItems > 800) maxIter = 300000;
    if(sizeItems > 1000) maxIter = 400000;

    std::srand((unsigned)time(NULL));

    // initial solution for SA: use greedySol
    std::vector<char> currentSol = greedySol;
    std::vector<char> candidateSol(sizeItems);
    std::vector<char> bestFeasibleSol = greedySol; // best feasible (valid) solution found
    double currentScore = calculatePenalizedScore(currentSol, penalty_coef, maxWeight);
    ll currentProfit = calculateSolProfitBool(currentSol);
    double bestScore = currentScore;
    ll bestFeasibleProfit = (currentProfit >= 0 ? currentProfit : -1);
    if(bestFeasibleProfit < 0) bestFeasibleProfit = -1;

    for(int iter=0; iter<maxIter; ++iter){
        // generate neighbor
        copySol(candidateSol, currentSol);
        tweak(candidateSol);

        double candScore = calculatePenalizedScore(candidateSol, penalty_coef, maxWeight);
        ll candProfit = calculateSolProfitBool(candidateSol);

        double delta = candScore - currentScore;
        bool accept = false;
        if(delta >= 0.0) accept = true;
        else {
            double p = exp(delta / T);
            double u = (double)rand() / (double)RAND_MAX;
            if(u < p) accept = true;
        }

        if(accept){
            copySol(currentSol, candidateSol);
            currentScore = candScore;
            currentProfit = candProfit;
            if(candScore > bestScore) bestScore = candScore;
            // if feasible and better than best feasible, update
            if(candProfit >= 0){
                if(bestFeasibleProfit < candProfit){
                    bestFeasibleProfit = candProfit;
                    copySol(bestFeasibleSol, candidateSol);
                }
            }
        }

        // cooling
        T *= alpha;
        if(T < 1e-12) T = 1e-12;
    }

    printf("best S.A sol: ");
    for(int i=0;i<sizeItems;i++) printf("%d", bestFeasibleSol[i]?1:0);
    printf("\nS.A: %lld\n", bestFeasibleProfit >= 0 ? bestFeasibleProfit : 0LL);

    // cleanup
    for(int i=0;i<sizeItems;i++) free(itens[i]);
    free(itens);
    return 0;
}

// calculate profit if feasible, else return -1
ll calculateSolProfitBool(const std::vector<char> &sol){
    long long profit = 0;
    long long weight = 0;
    for(int i=0;i<sizeItems;i++){
        if(sol[i]){
            profit += itens[i][0];
            weight += itens[i][1];
        }
    }
    if(weight > maxWeight) return -1;
    return profit;
}

// penalized score: profit - penalty_coef * max(0, weight - maxWeight)
double calculatePenalizedScore(const std::vector<char> &sol, double penalty_coef, ll maxWeight){
    long double profit = 0.0L;
    long double weight = 0.0L;
    for(int i=0;i<sizeItems;i++){
        if(sol[i]){
            profit += (long double)itens[i][0];
            weight += (long double)itens[i][1];
        }
    }
    long double overflow = 0.0L;
    if(weight > (long double)maxWeight) overflow = weight - (long double)maxWeight;
    long double score = profit - (long double)penalty_coef * overflow;
    return (double)score;
}

// simple mutation: flip 1 bit, small chance flip 2 bits
void tweak(std::vector<char> &sol){
    int n = sizeItems;
    int i = rand() % n;
    sol[i] = !sol[i];
    double r = (double)rand() / (double)RAND_MAX;
    if(r < 0.10){
        int j = rand() % n;
        if(j == i) j = (j + 1) % n;
        sol[j] = !sol[j];
    }
}

void readFile(const char* fileName){
    char s[1024];
    FILE *stream = fopen(fileName, "r");
    if(stream == NULL){
        std::fprintf(stderr, "\nFail to Open File!! (%s)\n", fileName);
        std::exit(1);
    }
    int line = -1;
    while(fgets(s, sizeof(s), stream)){
        size_t len = strlen(s);
        if(len > 0 && s[len-1] == '\n') s[len-1] = '\0';
        if(len > 1 && s[len-2] == '\r') s[len-2] = '\0';

        char* value1 = std::strtok(s, " ");
        char* value2 = std::strtok(NULL, " ");
        char* value3 = std::strtok(NULL, " ");
        line++;

        if(value1 == NULL) continue;
        if(line == 0){
            sizeItems = atoi(value1);
            itens = (ll**) malloc(sizeof(ll*) * sizeItems);
            for(int i=0;i<sizeItems;i++){
                itens[i] = (ll*) malloc(sizeof(ll)*2);
                itens[i][0] = itens[i][1] = 0;
            }
        }
        if(line > 0 && line <= sizeItems){
            int id = atoi(value1);
            if(id != line - 1){
                std::fprintf(stderr,"\nERROR! Invalid knapsack file, aborting!! read id=%d expected=%d\n", id, line-1);
                std::exit(1);
            }
            long long p = atoll(value2);
            long long w = atoll(value3);
            itens[id][0] = p;
            itens[id][1] = w;
        }
        if(line == sizeItems + 1){
            maxWeight = atoll(value1);
        }
    }
    fclose(stream);
}
