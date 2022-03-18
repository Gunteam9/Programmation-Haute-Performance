#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <omp.h>

#include "fonctions.h"

#define TAG 10

using namespace std;

int main(int argc, char **argv)
{
	int nslaves, pid, nmasters, flag;
	MPI_Comm intercom;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nslaves);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_get_parent(&intercom);			   // obtention de l'intercommunicateur vers le/les parents
	MPI_Comm_remote_size(intercom, &nmasters); // permet de connaître le nombre de parents
	// code de l'esclave

	// Attention pour les esclaves on a esclave ./maitre 16 4 0 donc n est l'argument n°2
	int n = atoi(argv[3]);
	int m = atoi(argv[4]);
	int root = atoi(argv[5]);

	MPI_Status matriceStatus;
	int* matrice = new int[n * n];

	MPI_Recv(matrice, n*n, MPI_INT, root, TAG, intercom, &matriceStatus);

	// cout << "Matrice" << endl;
	// for (size_t i = 0; i < n; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << matrice[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	int vecGetCount = pid < m % nslaves ? (m / nslaves) + 1 : m / nslaves;
	int vecGetIndex = pid < m % nslaves ? ((m / nslaves) + 1) * pid : ((m / nslaves) + 1) * (m % nslaves) + (m / nslaves) * (pid - (m % nslaves));

	MPI_Status vecStatus;
	int* tab = new int[vecGetCount * n];
	MPI_Recv(tab, vecGetCount * n, MPI_INT, root, TAG, intercom, &vecStatus);

	int** localVector = new int*[vecGetCount];

	// Copie dans un vecteur local
    for (int i = 0; i < vecGetCount; i++) {
        localVector[i] = new int[n];
        localVector[i] = tab + i * n;
    }

	// cout << "Après le calcul pour le PID " << pid << endl;
	#pragma omp parallel for
    for (int i = 0; i < vecGetCount; i++) {
        int* res = new int[n];
        matrix_vector_product(n, matrice, localVector[i], res);

        // Affichage

        // for (int j = 0; j < n; j++) {
        //     cout << res[j] << " ";
        // }
        // cout << endl;

		for (size_t j = 0; j < n; j++)
			tab[i * n +  j] = res[j];

		delete[] res;
    }

	delete[] localVector;
	delete[] matrice;
	

	MPI_Ssend(tab, vecGetCount * n, MPI_INT, root, TAG, intercom);
	
	delete[] tab;

	MPI_Comm_free(&intercom);
	MPI_Finalize();
	return 0;
}
