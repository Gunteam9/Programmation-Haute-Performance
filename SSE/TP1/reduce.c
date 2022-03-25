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

    float sum = 0.0;
    for (int i = 0; i < SIZE; ++i) {
        sum += array[i];
    }

    for (int i = 0; i < SIZE; ++i) {
//        printf("%f ", array[i]);
    }
    printf("\n");
    printf("Sum: %f", sum);


    printf("\n");

    free(array);

    return sum;
}

