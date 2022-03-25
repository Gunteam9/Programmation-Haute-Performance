/**
 * Copy values from an array to another.
 */
#include <stdio.h>
#include <string.h>
#include <immintrin.h>
#include "stdlib.h"
#include "time.h"

#define SIZE 16

// NE FONCTIONNE PAS

int main(void) {
    // Static arrays are stored into the stack thus we need to add an alignment attribute to tell the compiler to correctly align both arrays.
    unsigned char* array0 = (unsigned char*) _mm_malloc(sizeof(unsigned char) * SIZE, 16);
    unsigned char* array1 = (unsigned char*) _mm_malloc(sizeof(unsigned char) * SIZE, 16);

    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array0[i] = 97 + rand() % 26;
    }

    memcpy(array1, array0, SIZE);

    for (int i = 0; i < SIZE / 16; ++i) {
        __m128i r0 = _mm_load_si128((__m128i*) (array0 + i * 16));
        __m128i r1 = _mm_load_si128((__m128i*) 32);
        r0 = _mm_xor_si128(r0, r1);
        _mm_store_si128((__m128i*) (array0 + i * 16), r0);
    }

    float sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        printf("%c - %c | ", array0[i], array1[i]);
        sum += array0[i];
    }

    printf("\n");

    _mm_free(array0);
    _mm_free(array1);

    return sum;
}

