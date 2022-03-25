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
    unsigned short* array0 = (unsigned short*) _mm_malloc(sizeof(unsigned short) * SIZE, 16);
    unsigned short* array1 = (unsigned short*) _mm_malloc(sizeof(unsigned short) * SIZE, 16);
    unsigned short* array2 = (unsigned short*) _mm_malloc(sizeof(unsigned short) * SIZE, 16);

    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array0[i] = rand() % 10;
        array1[i] = rand() % 10;
    }

    for (int i = 0; i < SIZE / 8; ++i) {
        __m128i r0 = _mm_load_si128((__m128i*) (array0 + i * 8));
        __m128i r1 = _mm_load_si128((__m128i*) (array1 + i * 8));
        __m128i res = _mm_add_epi32(r0, r1);
        _mm_store_si128((__m128i*) (array2 + i * 8), res);
    }

    float sum = 0;
    for (int i = 0; i < SIZE; ++i) {
//        printf("%d + %d = %d \n", array0[i], array1[i], array2[i]);
        sum += array2[i];
    }

    printf("\n");

    _mm_free(array0);
    _mm_free(array1);
    _mm_free(array2);

    return sum;
}

