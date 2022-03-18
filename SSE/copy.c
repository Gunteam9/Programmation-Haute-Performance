/**
 * Copy values from an array to another.
 */
#include <stdio.h>
#include "stdlib.h"
#include "time.h"

#define SIZE 1000000000

int main(void) {
    // Static arrays are stored into the stack thus we need to add an alignment attribute to tell the compiler to correctly align both arrays.
    float* array0 = (float*) malloc(sizeof(float) * SIZE);
    float* array1 = (float*) malloc(sizeof(float) * SIZE);

    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array0[i]  = rand() / ((float) RAND_MAX);
    }

    for (int i = 0; i < SIZE; ++i) {
        array1[i] = array0[i];
    }

    float sum = 0;
    for (int i = 0; i < SIZE; ++i) {
//        printf("%f ", array1[i]);
        sum += array1[i];
    }

    printf("\n");

    free(array0);
    free(array1);

    return sum;
}

