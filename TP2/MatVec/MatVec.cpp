#include <iostream>
#include <fstream>
#include <chrono>

#include <mpi.h>

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

    int *matrice = new int[n * n];  // la matrice
    int *vecteurs = new int[n * m]; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.

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
    //     cout << "Contenu des vecteurs sur root" << endl;

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


    int vecGetCount = pid < m % nprocs ? (m / nprocs) + 1 : m / nprocs;
    int vecGetIndex = pid < m % nprocs ? ((m / nprocs) + 1) * pid : ((m / nprocs) + 1) * (m % nprocs) + (m / nprocs) * (pid - (m % nprocs));
    int* localVector = new int[vecGetCount * n];

    int* sendCount = new int[nprocs];
    int* displs = new int[nprocs];
    if (pid == root) {
        for (int i = 0; i < nprocs; i++) {
            if (i < m % nprocs) {
                sendCount[i] = (m / nprocs + 1) * n;
                displs[i] = (((m / nprocs) + 1) * i) * n;
            }
            else {
                sendCount[i] = (m / nprocs) * n;
                displs[i] = (((m / nprocs) + 1) * (m % nprocs) + (m / nprocs) * (i - (m % nprocs))) * n;
            }
        }
    }
    
    // On Bcast la matrice
    MPI_Bcast(matrice, n*n, MPI_INT, root, MPI_COMM_WORLD);

    // On récup les vecteurs
    // MPI_Scatterv( const void* sendbuf , const int sendcounts[] , const int displs[] , MPI_Datatype sendtype , void* recvbuf , int recvcount , MPI_Datatype recvtype , int root , MPI_Comm comm);
    MPI_Scatterv(vecteurs, sendCount, displs, MPI_INT, localVector, vecGetCount * n, MPI_INT, root, MPI_COMM_WORLD);

    // Vecteur local

    // cout << "Contenu des vecteurs" << endl;

    // for (int i = 0; i < vecGetCount * n; i++)
    // {
    //     cout << "PID " << pid << " Local Vector " << i << ": ";
    //     cout << localVector[i] << " ";
    //     cout << endl;
    // }

    // Calcul du produit matrice vecteur

    // cout << "Après le calcul pour le PID " << pid << endl;
    int* res = new int[n * vecGetCount];
    for (int i = 0; i < vecGetCount; i++) {
        matrix_vector_product(n, matrice, localVector, res);

        // Affichage

        // for (int j = 0; j < n; j++) {
        //     cout << res[j] << " ";
        // }
        // cout << endl;
    }

    delete[] matrice;
    delete[] localVector;
    
    MPI_Gatherv(res, n * vecGetCount, MPI_INT, vecteurs, sendCount, displs, MPI_INT, root, MPI_COMM_WORLD);

    delete[] sendCount;
    delete[] displs;
    delete[] res;

    // Affichage des vecteurs résultats sur root

    // if (pid == root)
    // {
    //     cout << "Contenu des vecteurs sur root" << endl;

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
            o << "MatVec;" << elapsed_seconds.count() << ";" << endl;
            o.close();
        }
    }

    delete[] vecteurs;
    
    MPI_Finalize();
    return 0;
}
