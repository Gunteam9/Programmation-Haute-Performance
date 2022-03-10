#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <fstream>

#include "fonctions.h"

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
    string fileName = "";
    if (argc > 5)
        fileName = argv[5];

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

	// Chrono
    if (pid == root)
        debut = chrono::system_clock::now();

	MPI_Comm_spawn("esclave",		   // exécutable
				   argv,			   // arguments à passer sur la ligne de commande
				   nslaves,			   // nombre de processus esclave
				   MPI_INFO_NULL,	   //permet de préciser où et comment lancer les processus
				   root,			   // rang du processus maître qui effectue réellement le spawn
				   MPI_COMM_WORLD,	   // Monde dans lequel est effectué le spwan
				   &intercom,		   // l'intercommunicateur permettant de communiquer avec les esclaves
				   MPI_ERRCODES_IGNORE // tableau contenant les erreurs
	);

	MPI_Comm intracom;
	MPI_Intercomm_merge (intercom, 0, &intracom);
	int pid_intra;
	MPI_Comm_rank (intracom, &pid_intra);

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
	// for (size_t i = 0; i < m; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << vecteurs[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	MPI_Win winGetMat;
	MPI_Win_create(matrice, n * n * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &winGetMat); // déclaration de la fenêtre
	
    MPI_Win_fence(0, winGetMat);

	// Les slaves récup la matrice depuis cette fenetre

	MPI_Win_free(&winGetMat);

	MPI_Win winGetVec;
	int maxVecGetCount = (m / nslaves) + 1;

	MPI_Win_create(vecteurs, maxVecGetCount * n * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &winGetVec); // déclaration de la fenêtre
	
    MPI_Win_fence(0, winGetVec);

	// Les slaves récup leurs vecteur depuis cette fenetre

	MPI_Win_free(&winGetVec);


	MPI_Win winPutRes;
	MPI_Win_create(vecteurs, m * n * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &winPutRes);

	MPI_Win_fence(0, winPutRes);

	// Les slaves put leurs résultats ici

	MPI_Win_free(&winPutRes);


	// Résultat

	// cout << "Vecteurs" << endl;
	// for (size_t i = 0; i < m; i++)
	// {
	// 	for (size_t j = 0; j < n; j++)
	// 	{
	// 		cout << vecteurs[i * n + j] << " ";
	// 	}
	// 	cout << endl;
	// }

	// Dans le temps écoulé on ne s'occupe que de la partie communications et calculs
    // (on oublie la génération des données et l'écriture des résultats sur le fichier de sortie)
    if (pid == root)
    {
        fin = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = fin - debut;
        cout << "Temps en secondes : " << elapsed_seconds.count() << endl;

        if (!fileName.empty()) {
			ofstream o;
			o.open("../" + fileName, ios::app);
			o << "MatVecMasterSlaveRMA;" << elapsed_seconds.count() << ";" << endl;
			o.close();
		}
    }

	MPI_Comm_free(&intercom);
	MPI_Finalize();
	return 0;
}
