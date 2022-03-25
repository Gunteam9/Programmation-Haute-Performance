/**
 * Copy values from an array to another.
 */
#include <stdio.h>
#include <string.h>
#include "stdlib.h"
#include "time.h"

#define SIZE 1000000000

int main(void) {
    // Static arrays are stored into the stack thus we need to add an alignment attribute to tell the compiler to correctly align both arrays.
    unsigned char* array0 = (unsigned char*) malloc(sizeof(unsigned char) * SIZE);
//    unsigned char* array1 = (unsigned char*) malloc(sizeof(unsigned char) * SIZE);

    srand(time(NULL));

    for (int i = 0; i < SIZE; ++i) {
        array0[i] = 97 + (rand() % 26);
    }

//    memcpy(array1, array0, SIZE);
//
//    for (int i = 0; i < SIZE; ++i) {
//        printf("%c - %c | ", array0[i], array1[i]);
//    }

    printf("\n");

    for (int i = 0; i < SIZE; ++i) {
        array0[i] = array0[i] ^ 32;
    }

    float sum = 0;
    for (int i = 0; i < SIZE; ++i) {
//        printf("%c - %c | ", array0[i], array1[i]);
        sum += array0[i];
    }

    printf("\n");

    free(array0);
//    free(array1);

    return sum;
}

