/**
 * Copy values from an array to another.
 */
#include <stdio.h>
#include <immintrin.h>
#include "stdlib.h"
#include "time.h"

#define SIZE 1000000000

int main(void) {
    // Static arrays are stored into the stack thus we need to add an alignment attribute to tell the compiler to correctly align both arrays.
    float* array0 = (float*) _mm_malloc(sizeof(float) * SIZE, 16);
    float* array1 = (float*) _mm_malloc(sizeof(float) * SIZE, 16);

    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array0[i] = rand() / ((float) RAND_MAX);
    }


    for (int i = 0; i < SIZE / 4; ++i) {
        // Load 4 values from the first array into a SSE register.
        __m128 r0 = _mm_load_ps(array0 + i*4);
        // Store the content of the register into the second array.
        _mm_store_ps(array1 + i*4, r0);
    }

    float sum = 0;
    for (int i = 0; i < SIZE; ++i) {
//        printf("%f ", array1[i]);
        sum += array1[i];
    }

    printf("\n");

    _mm_free(array0);
    _mm_free(array1);

    return sum;
}

