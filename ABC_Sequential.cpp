#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include <chrono>

#define MAX_NUM_PARAM 2
#define COLONY_SIZE 100
#define NUM_FOOD_SOURCE COLONY_SIZE/2
#define NUM_EMLOYED_BESS NUM_FOOD_SOURCE
#define LOWER_BOUND -512
#define UPPER_BOUND 512

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

void employedBeesPhase(foodSource origFood[]){

    for (int i = 0; i < NUM_FOOD_SOURCE; i++){
        int randParam = (int)(rand()%(MAX_NUM_PARAM));
        int randNeigh = (int)(rand()%(NUM_FOOD_SOURCE));
        while (randNeigh == i){
            randNeigh = (int)(rand()%(NUM_FOOD_SOURCE));
        }
        foodSource currSrc = origFood[i];
        foodSource neighSrc = origFood[randNeigh];
        foodSource newSrc = origFood[i];
        double newParam;
        while(true){
            double randInt = (double)(rand() % (200+1));
            double phi = (-1.0 + randInt/200.0)/10;
            double step = phi*(currSrc.params[randParam] - neighSrc.params[randParam]);
            
            //printf("step: %f randParam: %d ", step, randParam);

            newParam = currSrc.params[randParam] + step;
            if (newParam >= LOWER_BOUND && newParam <= UPPER_BOUND){
                break;
            }
        }

        newSrc.params[randParam] = newParam;
        //printf("old food: %f, %f, %f, %f, new food: %f, %f, %f, %f\n", currSrc.params[0], currSrc.params[1], currSrc.params[2], currSrc.params[3], newSrc.params[0], newSrc.params[1], newSrc.params[2], newSrc.params[3]);

        newSrc.functionVal = objectiveFunct(newSrc.params, MAX_NUM_PARAM);
        newSrc.nectarVal = getNectarVal(newSrc.functionVal);
        //printf("old nectar: %f, new nectar %f \n", currSrc.nectarVal, newSrc.nectarVal);
        if (newSrc.nectarVal > currSrc.nectarVal){
            newSrc.numTrials = 0;
            origFood[i] = newSrc;
        } else{
            origFood[i].numTrials += 1;
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

void onlookerBeesPhase(foodSource origFood[]){
    double totalFitness = totalFitnessProb(origFood);
    int bee_index = 0;
    int i = 0;
    while(bee_index < NUM_FOOD_SOURCE){
        if (i >= NUM_FOOD_SOURCE) {
            i = 0;
        }
        origFood[i].onlookerProb = origFood[i].nectarVal/totalFitness;
        double randInt = (double)(rand() % (100+1));
        double randP = randInt/100.0; 
        //printf("bee_index: %d, i: %d, onlocker prob: %f, randP %f \n", bee_index, i, origFood[i].onlookerProb, randP);    
        if (origFood[i].onlookerProb > randP){
            int randParam = (int)(rand()%(MAX_NUM_PARAM));
            int randNeigh = (int)(rand()%(NUM_FOOD_SOURCE));
            while (randNeigh == i){
                randNeigh = (int)(rand()%(NUM_FOOD_SOURCE));
            }
            foodSource currSrc = origFood[i];
            foodSource neighSrc = origFood[randNeigh];
            foodSource newSrc = origFood[i];
            int newParam;
            while(true){
                double randInt = (double)(rand() % (200+1));
                double phi = -1.0 + randInt/200.0;
                newParam = currSrc.params[randParam] + phi*(currSrc.params[randParam] - neighSrc.params[randParam]);
                if (newParam >= LOWER_BOUND && newParam <= UPPER_BOUND){
                    break;
                }
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
    using namespace std::chrono;
    typedef std::chrono::high_resolution_clock Clock;
    typedef std::chrono::duration<double> dsec;
    //first arg = num iterations
    int MAX_ITERS = atoi(argv[1]);
    int MAX_TRIALS = atoi(argv[2]);
    int NUM_THREADS = atoi(argv[3]);
    printf("Max iters: %d\n", MAX_ITERS);
    printf("Max trials: %d\n", MAX_TRIALS);
    printf("Num Threads: %d\n", NUM_THREADS);

    foodSource origFood[NUM_FOOD_SOURCE];
    initializeFood(origFood);
    double overall_min = INT_MAX;
    double params[MAX_NUM_PARAM];

    auto compute_start = Clock::now();
    double compute_time = 0;


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
     compute_time += duration_cast<dsec>(Clock::now() - compute_start).count();
     for (int j = 0; j < MAX_NUM_PARAM; j++){
            printf("\tOverall min param #%d = %f\n", j, params[j]);
        }
        printf("\tFunction Value = %f \n", overall_min);

    printf("Computation Time: %lf.\n", compute_time);
    return 0;
}