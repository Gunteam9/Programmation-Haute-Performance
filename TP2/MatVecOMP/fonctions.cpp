//
// Created by sophie on 18/02/2020.
//
#include <iostream>
#include "fonctions.h"

using namespace std;
void generation_vecteur(int n, int* vecteur, int nb_zero) {
    for (int i=0; i<nb_zero; i++)
        vecteur[i] = 0;
    for (int i=nb_zero; i<n; i++)
        vecteur[i] = rand()%20;
}

void matrix_vector_product(int n, int *matrix, int *vector, int *res)
{
    for (int i = 0; i < n; i++)
    {
        res[i] = 0;
        for (int j = 0; j < n; j++)
            res[i] += matrix[i * n + j] * vector[j];
    }
}