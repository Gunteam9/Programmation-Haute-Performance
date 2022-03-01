#include <mpi.h>
#include <iostream>
#include <algorithm>
#include "fonctions.h"

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

	int n = atoi(argv[3]);
	int m = atoi(argv[4]);
	int root = atoi(argv[5]);

	MPI_Comm intracom;
	MPI_Intercomm_merge (intercom, 1, &intracom);
	int pid_intra;
	MPI_Comm_rank (intracom, &pid_intra);


	MPI_Win winGetMat;
	int* matrice = new int[n * n];
	
	MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, intracom, &winGetMat);

	MPI_Win_fence(0, winGetMat);

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, winGetMat);
	MPI_Get(matrice, n * n, MPI_INT, root, 0, n * n, MPI_INT, winGetMat);
	MPI_Win_unlock(0, winGetMat);

	MPI_Win_free(&winGetMat);

	// Affichage de la matrice

	// cout << "Matrice" << endl;
	// for (size_t i = 0; i < n; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << matrice[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	// Récup les vecteurs

	MPI_Win winGetVec;
	int vecGetCount = pid < m % nslaves ? (m / nslaves) + 1 : m / nslaves;
	int vecGetIndex = pid < m % nslaves ? ((m / nslaves) + 1) * pid : ((m / nslaves) + 1) * (m % nslaves) + (m / nslaves) * (pid - (m % nslaves));
	int* tab = new int[vecGetCount * n];
	
	MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, intracom, &winGetVec);

	MPI_Win_fence(0, winGetVec);

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, winGetVec);
	MPI_Get(tab, vecGetCount * n, MPI_INT, root, vecGetIndex * n, vecGetCount * n, MPI_INT, winGetVec);
	MPI_Win_unlock(0, winGetVec);

	MPI_Win_free(&winGetVec);

	// Affichage des vecteurs

	// cout << "PID " << pid << " Vecteurs" << endl;
	// for (size_t i = 0; i < vecGetCount; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << tab[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	int** localVector = new int*[vecGetCount];

	// Copie dans un vecteur local
    for (int i = 0; i < vecGetCount; i++) {
        localVector[i] = new int[n];
        localVector[i] = tab + i * n;
    }

	// cout << "Après le calcul pour le PID " << pid << endl;
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
    }
	
	MPI_Win winPutRes;

	MPI_Win_create(tab, vecGetCount * n * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &winPutRes); // déclaration de la fenêtre
	
    MPI_Win_fence(0, winPutRes);

	//Put vers le master
	MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, winPutRes);
	MPI_Put(tab, vecGetCount * n, MPI_INT, root, vecGetIndex * n, m * n, MPI_INT, winPutRes);
	MPI_Win_unlock(0, winPutRes);

	MPI_Win_free(&winPutRes);


	MPI_Finalize();
	return 0;
}
