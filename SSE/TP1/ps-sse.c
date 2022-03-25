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
    float* array2 = (float*) _mm_malloc(sizeof(float) * SIZE, 16);


    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array0[i]  = rand() / ((float) RAND_MAX);
        array1[i]  = rand() / ((float) RAND_MAX);
    }

    for (int i = 0; i < SIZE / 4; ++i) {
        __m128 r0 = _mm_load_ps(array0 + i * 4);
        __m128 r1 = _mm_load_ps(array1 + i * 4);
        __m128 r2 = _mm_mul_ps(r0, r1);
        _mm_store_ps(array2 + i * 4, r2);
    }

    float sum = 0;
    for (int i = 0; i < SIZE; ++i) {
//        printf("%f * %f = %f \n", array0[i], array1[i], array2[i]);
        sum += array2[i];
    }

    printf("\n");

    _mm_free(array0);
    _mm_free(array1);
    _mm_free(array2);

    return sum;
}

