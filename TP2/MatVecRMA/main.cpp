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

    string name = argv[4]; // le nom du fichier pour que le processus root copie les données initiales et les résultats

    // Petit test pour vérifier qu'on peut avoir plusieurs threads par processus.
    // #pragma omp parallel num_threads(4)
    //     {
    //         int id = omp_get_thread_num();
    // #pragma omp critical
    //         cout << "je suis le thread " << id << " pour pid=" << pid << endl;
    //     }

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    int *matrice = new int[n * n];  // la matrice
    int *vecteurs = new int[n * m]; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.
    int *vecRes = new int[n * m];   // les vecteurs résultats

    fstream f;
    if (pid == root)
    {
        f.open(name, std::fstream::out);
        // Génération de la matrice
        matrice = new int[n * n];
        srand(time(NULL));
        for (int i = 0; i < n * n; i++)
            matrice[i] = rand() % 10;

        // Affichage de la matrice
        f << "Matrice" << endl;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
                f << matrice[i * n + j] << " ";
            f << endl;
        }
        f << endl;

        // Génération des vecteurs d'entrée
        // vecteurs = new int[m * n];
        for (int i = 0; i < m; i++)
        {
            int nb_zero = rand() % (n / 2);
            generation_vecteur(n, vecteurs + i * n, nb_zero);
        }

        // Affichage des vecteurs d'entrée
        f << "Les vecteurs" << endl;
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
                f << vecteurs[i * n + j] << " ";
            f << endl;
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

    // // barrières pour s'assurer que les fenêtres soient créées
    MPI_Win_fence(0, theWinMat);
    MPI_Win_fence(0, theWinVec);

    // TODO si on est pas root récupérer les données des vecteurs d'entrées
    if (pid != root)
    {
        // MPI_Get( void* origin_addr , int origin_count , MPI_Datatype origin_datatype , int target_rank , MPI_Aint target_disp , int target_count , MPI_Datatype target_datatype , MPI_Win win);
        MPI_Get(matrice, n * n, MPI_INT, 0, 0, n * n, MPI_INT, theWinMat);
        // MPI_Get(vecteurs, m * n, MPI_INT, 0, 0, m * n, MPI_INT, theWinVec);
    }

    MPI_Win_fence(0, theWinMat);
    MPI_Win_fence(0, theWinVec);

    // Full vector printer

    // if (pid == root)
    //     cout << "Contenu des vecteurs" << endl;

    // MPI_Win_fence(0, theWinVec);

    // for (int i = 0; i < m; i++) {
    //     cout << "PID " << pid << " Vecteur " << i << ": ";
    //     for (int j = 0; j < n; j++) {
    //         cout << vecteurs[i * n + j] << " ";
    //     }
    //     cout << endl;
    // }

    // MPI_Win_fence(0 , theWinVec);

    // Only root

    if (pid == root)
    {
        cout << "Contenu des vecteurs" << endl;

        for (int i = 0; i < m; i++)
        {
            cout << "PID " << pid << " Vecteur " << i << ": ";
            for (int j = 0; j < n; j++)
            {
                cout << vecteurs[i * n + j] << " ";
            }
            cout << endl;
        }
    }

    MPI_Win_fence(0, theWinVec);

    MPI_Win winIndex;
    int currentVecIndex = 0;
    int *currentVec = new int[n * m];

    MPI_Win_create(&currentVecIndex, 1 * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &winIndex);

    MPI_Win_fence(0, theWinVec);
    MPI_Win_fence(0, winIndex);

    if (pid != root) {
        while (currentVecIndex < m) {
            // Get the currentVecIndex
            MPI_Win_lock(MPI_LOCK_SHARED, root, 0, winIndex);
            MPI_Get(&currentVecIndex, 1, MPI_INT, root, 0, 1, MPI_INT, winIndex);
            MPI_Win_unlock(root, winIndex);

            if (currentVecIndex >= m) {
                MPI_Win_unlock(root, winIndex);
                break;
            }
                
            // Put one to the vec
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, root, 0, winIndex);
            currentVecIndex++;
            MPI_Put(&currentVecIndex, 1, MPI_INT, root, 0, 1, MPI_INT, winIndex);
            MPI_Win_unlock(root, winIndex);
                
            // Get the vec
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, root, 0, theWinVec);
            MPI_Get(currentVec, n, MPI_INT, root, currentVecIndex * n, m * n, MPI_INT, theWinVec);
            MPI_Win_unlock(root, theWinVec);

            cout << "PID " << pid << " Current vec index " << currentVecIndex << " Vecteur courrant: ";
            for (int i = 0; i < n * m; i++)
            {
                cout << currentVec[i] << " ";
            }
            cout << endl;

        }
    }

    MPI_Win_fence(0, winIndex);
    MPI_Win_fence(0, theWinVec);

    // cout << "Current vec index in pos 1: " << currentVecIndex << " on pid " << pid << endl;
    // matrice_vecteur(n, matrice, vecteurs + currentVecIndex * n, vecRes + currentVecIndex * n);
    

    // Dans le temps écoulé on ne s'occupe que de la partie communications et calculs
    // (on oublie la génération des données et l'écriture des résultats sur le fichier de sortie)
    // if (pid == root)
    // {
    //     fin = chrono::system_clock::now();
    //     chrono::duration<double> elapsed_seconds = fin - debut;
    //     cout << "temps en secondes : " << elapsed_seconds.count() << endl;
    // }

    MPI_Finalize();
    return 0;
}
