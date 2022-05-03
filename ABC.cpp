#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include <omp.h>
#include <chrono>

#define MAX_NUM_PARAM 2
#define COLONY_SIZE 10000
#define NUM_FOOD_SOURCE COLONY_SIZE/2
#define NUM_EMLOYED_BESS NUM_FOOD_SOURCE
#define LOWER_BOUND -512
#define UPPER_BOUND 512
using namespace std::chrono;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::duration<double> dsec;

struct foodSource {
    double params[MAX_NUM_PARAM];
    double functionVal;
    double nectarVal;
    double numTrials;
    double onlookerProb;
};

double getNectarVal(double functVal){
    double nectarVal;
    if (functVal >= 0) {
        nectarVal = 1/(1+functVal);
    } else{
        nectarVal = 1 + abs(functVal);
    }
    return nectarVal;
}
double objectiveFunct(double param[], int maxNum) {
    double x1 = param[0];
    double x2 = param[1];

    double result = -(x2 + 47) * sin(sqrt(abs(x2 + x1/2 + 47))) - x1 * sin(sqrt(abs(x1 - (x2 + 47))));
    return result;
}

double objectiveFunctTest(double param[], int maxNum){
    double functionVal;
    //Sphere function sum(1 to maxNum) (x-0.5)^2
    functionVal = pow((pow(param[0], 2) + param[1] - 11), 2) + pow((pow(param[1], 2) + param[0] - 7), 2);
    return functionVal;
}

double objectiveFunctold(double param[], int maxNum){
    double functionVal;
    //Sphere function sum(1 to maxNum) (x-0.5)^2
    for (int start = 0; start < maxNum; start++){
        functionVal += pow((param[start] - 0.5), 2.0);
    }
    return functionVal;
}

void initializeOneFood(foodSource origFood[], int i){
    for (int j = 0; j < MAX_NUM_PARAM; j++){
        double randInt = (double)(rand() % (100+1));
        double randP = randInt/100.0;            
        origFood[i].params[j] = LOWER_BOUND + randP * (UPPER_BOUND-LOWER_BOUND);
    }
    origFood[i].functionVal = objectiveFunct(origFood[i].params, MAX_NUM_PARAM);
    origFood[i].nectarVal = getNectarVal(origFood[i].functionVal);
    origFood[i].numTrials = 0;
    origFood[i].onlookerProb = 0;
}
void initializeFood(foodSource origFood[]){
    for (int i = 0; i < NUM_FOOD_SOURCE; i++){
        initializeOneFood(origFood, i);
    }
}

void employedBeesPhase(foodSource origFood[], int thread, int iter){
    auto employed_start = Clock::now();
    unsigned seed = omp_get_thread_num();
    for (int i = 0; i < NUM_FOOD_SOURCE; i++){
        
        int randParam = (int)(rand_r(&seed)%(MAX_NUM_PARAM));
        int randNeigh = (int)(rand_r(&seed)%(NUM_FOOD_SOURCE));
        
        while (randNeigh == i){
            randNeigh = (int)(rand_r(&seed)%(NUM_FOOD_SOURCE));
        }
        
        foodSource currSrc = origFood[i];
        foodSource neighSrc = origFood[randNeigh];
        foodSource newSrc = origFood[i];
        double newParam;
        double randInt = (double)(rand_r(&seed) % (200+1));
        double phi = (-1.0 + randInt/200.0)/10;
        double step = phi*(currSrc.params[randParam] - neighSrc.params[randParam]);
        
        newParam = currSrc.params[randParam] + step;
        if (newParam < LOWER_BOUND){
            newParam = LOWER_BOUND;
        }
        if (newParam > UPPER_BOUND){
            newParam = UPPER_BOUND;
        }

        newSrc.params[randParam] = newParam;
        newSrc.functionVal = objectiveFunct(newSrc.params, MAX_NUM_PARAM);
        newSrc.nectarVal = getNectarVal(newSrc.functionVal);
        if (newSrc.nectarVal > currSrc.nectarVal){
            newSrc.numTrials = 0;
            origFood[i] = newSrc;
        } else{
            origFood[i].numTrials += 1;
        }

        if (i == 2500 && iter == 0){
            printf("Thread: %d, Employed time: %lf.\n", thread, duration_cast<dsec>(Clock::now() - employed_start).count());
        }

    }
    
}

double totalFitnessProb(foodSource origFood[]){
    double totalFitness;
    for (int i = 0; i < NUM_FOOD_SOURCE; i++){
        totalFitness += origFood[i].nectarVal;
    }
    return totalFitness;
}

void onlookerBeesPhase(foodSource origFood[], int iter){
    double totalFitness = totalFitnessProb(origFood);
    int bee_index = 0;
    int i = 0;
    int count = 0;
    auto onlooker_start = Clock::now();
    double compute_time = 0;
    unsigned seed = omp_get_thread_num();
            
    while(bee_index < NUM_FOOD_SOURCE){
        count++;
        
        if (i >= NUM_FOOD_SOURCE) {
            i = 0;
        }
        origFood[i].onlookerProb = origFood[i].nectarVal/totalFitness;
        double randInt = (double)(rand_r(&seed) % (100+1));
        double randP = randInt/100.0; 
        //printf("bee_index: %d, i: %d, onlocker prob: %f, randP %f \n", bee_index, i, origFood[i].onlookerProb, randP);    
        if (origFood[i].onlookerProb > randP){
            int randParam = (int)(rand_r(&seed)%(MAX_NUM_PARAM));
            int randNeigh = (int)(rand_r(&seed)%(NUM_FOOD_SOURCE));
            while (randNeigh == i){
                randNeigh = (int)(rand_r(&seed)%(NUM_FOOD_SOURCE));
            }
            foodSource currSrc = origFood[i];
            foodSource neighSrc = origFood[randNeigh];
            foodSource newSrc = origFood[i];
            int newParam;
            double randInt = (double)(rand_r(&seed) % (200+1));
            double phi = -1.0 + randInt/200.0;
            newParam = currSrc.params[randParam] + phi*(currSrc.params[randParam] - neighSrc.params[randParam]);
            if (newParam < LOWER_BOUND){
                newParam = LOWER_BOUND;
            }
            if (newParam > UPPER_BOUND){
                newParam = UPPER_BOUND;
            }
            newSrc.params[randParam] = newParam;
            newSrc.functionVal = objectiveFunct(newSrc.params, MAX_NUM_PARAM);
            newSrc.nectarVal = getNectarVal(newSrc.functionVal);
            if (newSrc.nectarVal > currSrc.nectarVal){
                newSrc.numTrials = 0;
                origFood[i] = newSrc;
            } else{
                origFood[i].numTrials += 1;
            }
            bee_index += 1;
        }       
        i += 1;
    }
    if (iter == 0){
        printf("Count = %d\n", count);
    }

}

void scoutBeesPhase(foodSource origFood[], int max_num_trials){
    int maxTrials = 0;
    int maxIdx = 0;
    for (int i = 0; i < NUM_FOOD_SOURCE; i++){
        if (origFood[i].numTrials > maxTrials){
            maxTrials = origFood[i].numTrials;
            maxIdx = i;
        }
    }
    if (maxTrials > max_num_trials){
        initializeOneFood(origFood, maxIdx);
    }
}
bool compareFunction(foodSource a, foodSource b){
    return a.functionVal > b.functionVal;
}

int main(int argc, const char *argv[]) {
    //first arg = num iterations
    int MAX_ITERS = atoi(argv[1]);
    int MAX_TRIALS = atoi(argv[2]);
    int NUM_THREADS = atoi(argv[3]);
    printf("Max iters: %d\n", MAX_ITERS);
    printf("Max trials: %d\n", MAX_TRIALS);
    printf("Num Threads: %d\n", NUM_THREADS);

    omp_set_num_threads(NUM_THREADS);

    foodSource origFood[NUM_FOOD_SOURCE];
    initializeFood(origFood);
    double overall_min = INT_MAX;
    double params[MAX_NUM_PARAM];

    foodSource thread_food[NUM_THREADS * NUM_FOOD_SOURCE];
    double thread_mins[NUM_THREADS];
    int thread_min_idx[NUM_THREADS];

    auto compute_start = Clock::now();
    double compute_time = 0;
    
    int num_iters = 1;

    for (int i = 0; i < MAX_ITERS/NUM_THREADS/num_iters; i++) {
        int j;
        #pragma omp parallel for 
        for(j = 0; j < NUM_THREADS; j++) {
            auto copy_start = Clock::now();
            double copy_time = 0;
            int adjust = j*NUM_FOOD_SOURCE;
            for (int k = 0; k < NUM_FOOD_SOURCE; k++) {   
                thread_food[adjust + k] = origFood[k];
            }
            copy_time += duration_cast<dsec>(Clock::now() - copy_start).count();
            if (i == 0){
                printf("Thread: %d, copy time: %lf.\n", j, copy_time);
            }

            auto comp_start = Clock::now();
            double comp_time = 0;
            
            employedBeesPhase(&thread_food[j], j, i);
            if (i == 0){
                printf("Thread: %d, afterEmployed: %lf.\n", j, duration_cast<dsec>(Clock::now() - comp_start).count());
            }
            
            onlookerBeesPhase(&thread_food[j], i);
            if (i == 0){
                printf("Thread: %d, afterPhases: %lf.\n", j, duration_cast<dsec>(Clock::now() - comp_start).count());
            }
    
            double min = INT_MAX;
            int minIdx = 0;
            adjust = j * NUM_FOOD_SOURCE;
            for (int k = 0; k < NUM_FOOD_SOURCE; k++){
                if (thread_food[adjust + k].functionVal < min){
                    min = thread_food[adjust + k].functionVal;
                    minIdx = k;
                }
            }
            thread_mins[j] = min;
            thread_min_idx[j] = minIdx;
            //printf("j: %d \n", j);
            comp_time += duration_cast<dsec>(Clock::now() - comp_start).count();
            if (i == 0){
                printf("Thread: %d, comp_time: %lf.\n", j, comp_time);
            }
        
        }
        //printf("iter: %d\n", i);

        double min = INT_MAX;
        int minIdx = 0;
        int thread = 0;
        for (int j = 0; j < NUM_THREADS; j++) {
            if (thread_mins[j] < min) {
                min = thread_mins[j];
                minIdx = thread_min_idx[j];
                thread = j;
            }
        }
        if (min < overall_min) {
            overall_min = min;
            int adjust = thread*NUM_FOOD_SOURCE;
            for (int j = 0; j < MAX_NUM_PARAM; j++) {
                params[j] = thread_food[adjust + minIdx].params[j];
            }
        }
        int adjust = thread*NUM_FOOD_SOURCE;
        for (int j = 0; j < NUM_FOOD_SOURCE; j++) {
            origFood[j] = thread_food[adjust + j];
        }
        scoutBeesPhase(origFood, MAX_TRIALS);

    }

/*
    for (int i = 0; i < MAX_ITERS; i++){
        employedBeesPhase(origFood);
        onlookerBeesPhase(origFood);
        //printf("\nIter: %d\n", i);
        double min = INT_MAX;
        int minIdx = 0;
        for (int j = 0; j < NUM_FOOD_SOURCE; j++){
            if (origFood[j].functionVal < min){
                min = origFood[j].functionVal;
                minIdx = j;
            }
            if (min < overall_min) {
                overall_min = min;
                for (int j = 0; j < MAX_NUM_PARAM; j++) {
                    params[j] = origFood[minIdx].params[j];
                }
            }
        }


       // for (int j = 0; j < MAX_NUM_PARAM; j++){
       //     printf("\tMinimum param #%d = %f\n", j, origFood[minIdx].params[j]);
       // }
       // printf("\tFunction Value = %f \n", origFood[minIdx].functionVal);
        //sort(origFood, origFood + NUM_FOOD_SOURCE, compareFunction);
        //printf("M1 = (%f, %f)", origFood[0].params[0], origFood[0].params[1]);
        //printf("M2 = (%f, %f)", origFood[1].params[0], origFood[1].params[1]);
       //printf("M3 = (%f, %f)", origFood[2].params[0], origFood[2].params[1]);
        //printf("M4 = (%f, %f)", origFood[3].params[0], origFood[3].params[1]);
        scoutBeesPhase(origFood, MAX_TRIALS);
    }
    */
     compute_time += duration_cast<dsec>(Clock::now() - compute_start).count();
     for (int j = 0; j < MAX_NUM_PARAM; j++){
            printf("\tOverall min param #%d = %f\n", j, params[j]);
        }
        printf("\tFunction Value = %f \n", overall_min);

    printf("Computation Time: %lf.\n", compute_time);
    return 0;
}