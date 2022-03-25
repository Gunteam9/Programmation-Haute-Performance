#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>
#include "omp.h"

const size_t dim = 4;
const size_t sse_block_size_float = 4;

void printer(float *A, float *B, float *C, const size_t dim) {
    printf("Matrice A \n");

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            if (j < dim - 1)
                printf("%f, ", A[i * dim + j]);
            else
                printf("%f", A[i * dim + j]);        }
        printf("\n");
    }

    printf("Matrice B \n");

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            if (j < dim - 1)
                printf("%f, ", B[i * dim + j]);
            else
                printf("%f", B[i * dim + j]);
        }
        printf("\n");
    }

    printf("Matrice C \n");

    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            if (j < dim - 1)
                printf("%f, ", C[i * dim + j]);
            else
                printf("%f", C[i * dim + j]);
        }
        printf("\n");
    }
}

void mul_naive(float *A, float *B, float *res, const size_t dim) {
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            for (int k = 0; k < dim; ++k) {
                res[i * dim + j] += A[i * dim + k] * B[k * dim + j];
            }
        }
    }
}


void mul_naive_omp(float *A, float *B, float *res, const size_t dim) {
#pragma omp parallel for collapse(2)
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            for (int k = 0; k < dim; ++k) {
                res[i * dim + j] += A[i * dim + k] * B[k * dim + j];
            }
        }
    }
}


void mul_naive_sse_style(float *A, float *B, float *res, const size_t dim) {
    for (int i = 0; i < dim; ++i) {
        for (int k = 0; k < dim; ++k) {
            for (int j = 0; j < dim; ++j) {
                res[i * dim + j] += A[i * dim + k] * B[k * dim + j];
            }
        }
    }
}


void mul_sse(float *A, float *B, float *res, const size_t dim) {
    for (size_t i = 0; i < dim; ++i) {
        for (size_t k = 0; k < dim; ++k) {
            __m128 r0 = _mm_set1_ps(A[i * dim + k]);
            for (size_t j = 0; j < dim; j+=sse_block_size_float) {
                __m128 r1 = _mm_load_ps(B + k * dim + j);
                __m128 r2 = _mm_mul_ps(r0, r1);

                __m128 r3 = _mm_load_ps(res + i * dim + j);
                __m128 r4 = _mm_add_ps(r2, r3);
                _mm_store_ps(res + i * dim + j, r4);

            }
        }
    }
}


void check(float *A, float *B, float *res, const size_t dim) {
    float *exp = calloc(dim * dim, sizeof(float));

    mul_naive_omp(A, B, exp, dim);

    unsigned char isCheckPassed = 0;

    for (size_t i = 0; i < dim * dim; i++) {
        if (exp[i] != res[i]) {
            printf("Value at %lu differs: %f\n", i, exp[i] - res[i]);
            isCheckPassed = 1;
        }
    }

    if (isCheckPassed == 0)
        printf("Check passed \n");
    else
        printf("Check failed \n");

    free(exp);
}

int main() {
    srand(time(NULL));

    float *A = (float*) _mm_malloc(dim * dim * sizeof(float), 16);
    float *B = (float*) _mm_malloc(dim * dim * sizeof(float), 16);
    float *C = (float*) _mm_malloc(dim * dim * sizeof(float), 16);

    for (size_t i = 0; i < dim * dim; i++) {
        A[i] = (int) (rand() / ((float) RAND_MAX / 10));
        B[i] = (int) (rand()/ ((float) RAND_MAX / 10));
        C[i] = 0;
    }

    mul_sse(A, B, C, dim);

    check(A, B, C, dim);

    printer(A, B, C, dim);

    _mm_free(A);
    _mm_free(B);
    _mm_free(C);

    return 0;
}
