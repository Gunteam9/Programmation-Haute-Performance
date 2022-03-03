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
    string fileName = argv[4];

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    int *matrice = new int[n * n];  // la matrice
    int *vecteurs = new int[n * m]; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.
    int *vecRes = new int[n * m];   // les vecteurs résultats

    if (pid == root)
    {
        // Génération de la matrice
        matrice = new int[n * n];
        srand(time(NULL));
        for (int i = 0; i < n * n; i++)
            matrice[i] = rand() % 10;

        // Génération des vecteurs d'entrée
        // vecteurs = new int[m * n];
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
        // MPI_Get( void* origin_addr , int origin_count , MPI_Datatype origin_datatype , int target_rank , MPI_Aint target_disp , int target_count , MPI_Datatype target_datatype , MPI_Win win);
        MPI_Get(matrice, n * n, MPI_INT, 0, 0, n * n, MPI_INT, theWinMat);
    }

    MPI_Win_fence(0, theWinMat);
    MPI_Win_fence(0, theWinVec);

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
    //         cout << "PID " << pid << " Vecteur " << i << ": ";
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

    // cout << "PID " << pid << " VecGetCount " << vecGetCount << " VecGetIndex " << vecGetIndex << endl;

    // On récup les vecteurs
    if (pid != root) {
        MPI_Get(vecteurs, n * m, MPI_INT, root, vecGetIndex * n, vecGetCount * n, MPI_INT, theWinVec);
    }

    MPI_Win_fence(0, theWinVec);

    // Copie dans un vecteur local
    for (int i = 0; i < vecGetCount; i++) {
        localVector[i] = new int[n];
        localVector[i] = vecteurs + i * n;
    }

    // Vecteur global

    // cout << "Contenu des vecteurs" << endl;

    // for (int i = 0; i < m; i++)
    // {
    //     cout << "PID " << pid << " Vector " << i << ": ";
    //     for (int j = 0; j < n; j++)
    //     {
    //         cout << vecteurs[i * n + j] << " ";
    //     }
    //     cout << endl;
    // }
    
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

    MPI_Win_fence(0, theWinVec);

    // Calcul du produit matrice vecteur

    // cout << "Après le calcul pour le PID " << pid << endl;
    for (int i = 0; i < vecGetCount; i++) {
        int* res = new int[n];
        matrix_vector_product(n, matrice, localVector[i], res);

        // Affichage

        // for (int j = 0; j < n; j++) {
        //     cout << res[j] << " ";
        // }
        // cout << endl;
    }


    MPI_Win_fence(0, theWinVec);

    // Dans le temps écoulé on ne s'occupe que de la partie communications et calculs
    // (on oublie la génération des données et l'écriture des résultats sur le fichier de sortie)
    if (pid == root)
    {
        fin = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = fin - debut;
        cout << "temps en secondes : " << elapsed_seconds.count() << endl;

        ofstream o;
        o.open("../" + fileName, ios::app);
        o << "MatVecRMA;" << elapsed_seconds.count() << ";" << endl;
        o.close();
    }

    MPI_Finalize();
    return 0;
}
