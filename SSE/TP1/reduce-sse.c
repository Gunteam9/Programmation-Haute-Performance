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

    for (int i = 0; i < SIZE; ++i) {
        array[i]  = rand() / ((float) RAND_MAX);
    }

    __m128 sumReg = _mm_load_ps(array);
    for (int i = 1; i < SIZE / 4; ++i) {
        __m128 r0 = _mm_load_ps(array + i * 4);
        sumReg = _mm_add_ps(sumReg, r0);
    }

    _mm_store_ps(array, sumReg);

    float sum = array[0] + array[1] + array[2] + array[3];

    for (int i = 0; i < SIZE; ++i) {
//        printf("%f ", array[i]);
    }
    printf("\n");
    printf("Sum: %f", sum);


    printf("\n");

    free(array);

    return sum;
}

