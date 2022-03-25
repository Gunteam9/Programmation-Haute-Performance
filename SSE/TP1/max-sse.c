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
    float* array = (float*) _mm_malloc(sizeof(float) * SIZE, 16);

    srand(time(NULL));
    float *A = malloc(dim * dim * sizeof(float));
    float *B = malloc(dim * dim * sizeof(float));
    float *C = malloc(dim * dim * sizeof(float));
    for (int i = 0; i < SIZE; ++i) {
        array[i]  = rand() / ((float) RAND_MAX);
    }

    for (int i = 0; i < 4; ++i) {
        printf("%f ", array[i]);
    }

    __m128 tmp = _mm_load_ps(array);
    for (int i = 1; i < SIZE / 4; ++i) {
        __m128 r0 = _mm_load_ps(array + i * 4);
        tmp = _mm_max_ps(tmp, r0);
    }

    _mm_store_ps(array, tmp);

    float max = -1.0;
    for (int i = 0; i < 3; ++i) {
        if (array[i] > max)
            max = array[i];
    }

    for (int i = 0; i < 4; ++i) {
//        printf("%f ", array[i]);
    }
    printf("\n");
    printf("Max: %f", max);


    printf("\n");

    free(array);

    return max;
}

