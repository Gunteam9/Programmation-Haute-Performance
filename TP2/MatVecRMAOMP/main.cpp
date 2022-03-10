#include <iostream>
#include <fstream>
#include <chrono>

#include <mpi.h>
#include <omp.h>

#include "fonctions.h"

using namespace std;

int main(int argc, char **argv)
{

    // Pour initialiser l'environnement MPI avec la possibilité d'utiliser des threads (OpenMP)
    int provided;                                                  // renvoi le mode d'initialisation effectué
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); // MPI_THREAD_MULITPLE chaque processus MPI peut faire appel à plusieurs threads.

    MPI_Win theWinMat;
    MPI_Win theWinVec;

    // Pour connaître son pid et le nombre de processus de l'exécution paralléle (sans les threads)
    int pid, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int n = atoi(argv[1]); // taille de la matrice carrée
    int m = atoi(argv[2]); // nombre de vecteurs en entrée

    int root = atoi(argv[3]); // processeur root : référence pour les données
    string fileName = "";
    if (argc > 4)
        fileName = argv[4];

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    int* matrice = new int[n * n];  // la matrice
    int* vecteurs = new int[n * m]; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.

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

    // Chrono + Déclaration des fenêtres pour le processus root
    if (pid == root)
    {
        debut = chrono::system_clock::now();
        MPI_Win_create(matrice, n * n * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &theWinMat);  // déclaration de la fenêtre pour la matrice
        MPI_Win_create(vecteurs, n * m * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &theWinVec); // déclaration de la fenêtre pour les vecteurs
    }

    // Déclaration des fenêtres pour les processus non-root
    if (pid != root)
    {
        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &theWinMat);
        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &theWinVec);
    }

    // Barrières pour s'assurer que les fenêtres soient créées
    MPI_Win_fence(0, theWinMat);
    MPI_Win_fence(0, theWinVec);

    // On récup la matrice
    if (pid != root)
    {
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, theWinMat);
        MPI_Get(matrice, n * n, MPI_INT, 0, 0, n * n, MPI_INT, theWinMat);
        MPI_Win_unlock(0, theWinMat);
    }

    MPI_Win_free(&theWinMat);

    // MPI_Win_fence(0, theWinMat);
    // MPI_Win_fence(0, theWinVec);

    // Affichage de la matrice

    // if (pid == root) {
    //     cout << "Matrice: " << endl;
    //     for (int i = 0; i < n; i++) {
    //         for (int j = 0; j < n; j++) {
    //             cout << matrice[i * n + j] << " ";
    //         }
    //         cout << endl;
    //     }
    // }

    // Affiche des vecteurs

    // if (pid == root)
    // {
    //     cout << "Contenu des vecteurs" << endl;

    //     for (int i = 0; i < m; i++)
    //     {
    //         cout << "Vecteur " << i << ": ";
    //         for (int j = 0; j < n; j++)
    //         {
    //             cout << vecteurs[i * n + j] << " ";
    //         }
    //         cout << endl;
    //     }
    // }

    // Calcule de l'index et get pour chaque proc sauf root (le x)

    int vecGetCount = pid < m % nprocs ? (m / nprocs) + 1 : m / nprocs;
    int vecGetIndex = pid < m % nprocs ? ((m / nprocs) + 1) * pid : ((m / nprocs) + 1) * (m % nprocs) + (m / nprocs) * (pid - (m % nprocs));
    int** localVector = new int*[vecGetCount];

    // On récup les vecteurs
    if (pid != root) {
        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, theWinVec);
        MPI_Get(vecteurs, n * m, MPI_INT, root, vecGetIndex * n, vecGetCount * n, MPI_INT, theWinVec);
	    MPI_Win_unlock(0, theWinVec);
    }

    MPI_Win_free(&theWinVec);

    // Copie dans un vecteur local
    for (int i = 0; i < vecGetCount; i++) {
        localVector[i] = new int[n];
        localVector[i] = vecteurs + i * n;
    }

    // Vecteur local

    // for (int i = 0; i < vecGetCount; i++)
    // {
    //     cout << "PID " << pid << " Local Vector " << i << ": ";
    //     for (int j = 0; j < n; j++)
    //     {
    //         cout << localVector[i][j] << " ";
    //     }
    //     cout << endl;
    // }

    // Calcul du produit matrice vecteur

    // cout << "Après le calcul pour le PID " << pid << endl;
    int* tabRes = new int[n * vecGetCount];
    #pragma omp parallel for
    for (int i = 0; i < vecGetCount; i++) {
        int* res = new int[n];
        matrix_vector_product(n, matrice, localVector[i], res);

        // Affichage
        // cout << "Res on PID " << pid << ": ";
        // for (int j = 0; j < n; j++) {
        //     cout << res[j] << " ";
        // }
        // cout << endl;

		for (size_t j = 0; j < n; j++)
			tabRes[i * n +  j] = res[j];

        delete[] localVector[i];
        delete[] res;
    }

    delete[] localVector;
    delete[] matrice;

    int* vecRes = new int[m * n];

    MPI_Win winVecRes;

    if (pid == root)
        MPI_Win_create(vecRes, m * n * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winVecRes);
    else
        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &winVecRes);

    MPI_Win_fence(0, winVecRes);

    if (pid == root) {
        for (size_t i = 0; i < vecGetCount * n; i++)
			vecRes[i] = tabRes[i];
    } 
    else {
        MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, winVecRes);
        MPI_Put(tabRes, vecGetCount * n, MPI_INT, root, vecGetIndex * n, m * n, MPI_INT, winVecRes);
        MPI_Win_unlock(0, winVecRes);
    }

    MPI_Win_free(&winVecRes);

    // if (pid == root)
    // {
    //     cout << "Contenu des vecteurs sur root" << endl;

    //     for (int i = 0; i < m; i++)
    //     {
    //         cout << "Vec Res " << i << ": ";
    //         for (int j = 0; j < n; j++)
    //         {
    //             cout << vecRes[i * n + j] << " ";
    //         }
    //         cout << endl;
    //     }
    // }

    delete[] vecRes;
    delete[] tabRes;


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
            o << "MatVecRMAOMP;" << elapsed_seconds.count() << ";" << endl;
            o.close();
        }
    }

    MPI_Finalize();
    return 0;
}
