#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include "fonctions.h"

#define TAG 10

using namespace std;

int main(int argc, char **argv)
{
	int nmasters, pid;
	MPI_Comm intercom;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nmasters);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// le code des maîtres

	int nslaves = atoi(argv[1]); // nombre de slave
	int n = atoi(argv[2]); // taille de la matrice carrée
	int m = atoi(argv[3]); // nombre de vecteurs en entrée
	int root = atoi(argv[4]); // processeur root : référence pour les données

	if (nmasters > 1)
	{
		cout << "Un processus maitre max" << endl;
		exit(1);
	}

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    int* matrice = new int[n * n];  // la matrice
    int* vecteurs = new int[n * m]; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.
    int* vecRes = new int[n * m];   // les vecteurs résultats

    if (pid == root)
    {
        // Génération de la matrice
        srand(time(NULL));
        for (int i = 0; i < n * n; i++)
            matrice[i] = rand() % 10;

        // Génération des vecteurs d'entrée
        for (int i = 0; i < m; i++)
        {
            int nb_zero = rand() % (n / 2);
            generation_vecteur(n, vecteurs + i * n, nb_zero);
        }
    }

	MPI_Comm_spawn("esclave",		   // exécutable
				   argv,			   // arguments à passer sur la ligne de commande
				   nslaves,			   // nombre de processus esclave
				   MPI_INFO_NULL,	   //permet de préciser où et comment lancer les processus
				   root,			   // rang du processus maître qui effectue réellement le spawn
				   MPI_COMM_WORLD,	   // Monde dans lequel est effectué le spwan
				   &intercom,		   // l'intercommunicateur permettant de communiquer avec les esclaves
				   MPI_ERRCODES_IGNORE // tableau contenant les erreurs
	);

	// cout << "Matrice" << endl;
	// for (size_t i = 0; i < n; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << matrice[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	// cout << "Vecteurs" << endl;
	// for (size_t i = 0; i < n; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << vecteurs[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	for (size_t i = 0; i < nslaves; i++)
	{
		int vecGetCount = i < m % nslaves ? (m / nslaves) + 1 : m / nslaves;
		int vecGetIndex = i < m % nslaves ? ((m / nslaves) + 1) * i : ((m / nslaves) + 1) * (m % nslaves) + (m / nslaves) * (i - (m % nslaves));
		
		MPI_Ssend(matrice, n*n, MPI_INT, i, TAG, intercom);
		MPI_Ssend(vecteurs + vecGetIndex * n, vecGetCount * n, MPI_INT, i, TAG, intercom);

		MPI_Status recvStatus;
		MPI_Recv(vecteurs + vecGetIndex * n, m * n, MPI_INT, i, TAG, intercom, &recvStatus);
	}

	// Résultat

	cout << "Vecteurs" << endl;
	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < n; j++)
		{
			cout << vecteurs[i * n + j] << " ";
		}
		cout << endl;
	}

	MPI_Comm_free(&intercom);
	MPI_Finalize();
	return 0;
}
