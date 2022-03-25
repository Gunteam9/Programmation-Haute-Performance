/**
 * Copy values from an array to another.
 */
#include <stdio.h>
#include "stdlib.h"
#include "time.h"

#define SIZE 1000000000

int main(void) {
    // Static arrays are stored into the stack thus we need to add an alignment attribute to tell the compiler to correctly align both arrays.
    float* array = (float*) malloc(sizeof(float) * SIZE);

    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array[i]  = rand() / ((float) RAND_MAX);
    }

    float max = -1.0;
    for (int i = 0; i < SIZE; ++i) {
        if (max < array[i])
            max = array[i];
    }

    for (int i = 0; i < SIZE; ++i) {
//        printf("%f ", array[i]);
    }
    printf("\n");
    printf("Max: %f", max);


    printf("\n");

    free(array);

    return max;
}

