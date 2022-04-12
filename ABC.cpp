#include<stdio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>

#define MAX_NUM_PARAM 4
#define COLONY_SIZE 10
#define NUM_FOOD_SOURCE COLONY_SIZE/2
#define NUM_EMLOYED_BESS NUM_FOOD_SOURCE
#define LOWER_BOUND -1
#define UPPER_BOUND 1

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

double objectiveFunct(double param[], int maxNum){
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
        int newParam;
        while(true){
            double randInt = (double)(rand() % (200+1));
            double phi = (-1.0 + randInt/200.0)/10000.0;
            double step = phi*(currSrc.params[randParam] - neighSrc.params[randParam]);
            newParam = currSrc.params[randParam] + step;
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
    for (int i = 0; i < NUM_FOOD_SOURCE; i++){
        origFood[i].onlookerProb = origFood[i].nectarVal/totalFitness;
        double randInt = (double)(rand() % (100+1));
        double randP = randInt/100.0;     
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
        }       
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
int main(int argc, const char *argv[]) {
    //first arg = num iterations
    int MAX_ITERS = atoi(argv[1]);
    int MAX_TRIALS = atoi(argv[2]);
    printf("Max iters: %d\n", MAX_ITERS);
    printf("Max trials: %d\n", MAX_TRIALS);
    foodSource origFood[NUM_FOOD_SOURCE];
    for (int i = 0; i < MAX_ITERS; i++){
        initializeFood(origFood);
        employedBeesPhase(origFood);
        onlookerBeesPhase(origFood);
        scoutBeesPhase(origFood, MAX_TRIALS);
        printf("\nIter: %d\n", i);
        double min = 9999;
        int minIdx = 0;

        for (int j = 0; j < NUM_FOOD_SOURCE; j++){
            if (origFood[j].functionVal < min){
                min = origFood[j].functionVal;
                minIdx = j;
            }
        }
        for (int j = 0; j < MAX_NUM_PARAM; j++){
            printf("\tMinimum param #%d = %f\n", j, origFood[minIdx].params[j]);
        }
        printf("\tFunction Value = %f \n", origFood[minIdx].functionVal);
    }

    return 0;
}