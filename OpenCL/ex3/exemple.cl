__kernel
void matrice_matrice(const int taille, __global int* A, __global int* B, __global int* C){
    int i = get_global_id(0);
    int j = get_global_id(1);
    int tmp = 0;
    for(int k = 0; k < taille; k++) {
        tmp += A[i * taille + k] * B[k * taille + j];
    }
    C[i * taille + j] = tmp;
}

__kernel
void matrice_matrice_line(const int taille, __global int* A, __global int* B, __global int* C){
    int i = get_global_id(0);
    for (int j = 0; j < taille; ++j) {
        int tmp = 0;
        for(int k = 0; k < taille; k++) {
            tmp += A[i * taille + k] * B[k * taille + j];
        }
        C[i * taille + j] = tmp;
    }
}