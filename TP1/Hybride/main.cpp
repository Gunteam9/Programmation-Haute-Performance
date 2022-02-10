#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>

#include <mpi.h>
#include <omp.h>

#include "fonctions.h"

using namespace std;

int main(int argc, char **argv) {

    // Pour initialiser l'environnement MPI avec la possibilité d'utiliser des threads (OpenMP)
    int provided; // renvoi le mode d'initialisation effectué
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided); // MPI_THREAD_MULITPLE chaque processus MPI peut faire appel à plusieurs threads.

    // Pour connaître son pid et le nombre de processus de l'exécution paralléle (sans les threads)
    int pid, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int n = atoi(argv[1]); // taille de la matrice carrée
    int m = atoi(argv[2]); // nombre de vecteurs en entrée

    int root = atoi(argv[3]); // processeur root : référence pour les données

    string name = argv[4]; // le nom du fichier pour que le processus root copie les données initiales et les résultats

    // Petit test pour vérifier qu'on peut avoir plusieurs threads par processus.
    #pragma omp parallel num_threads(4)
    {
        int id = omp_get_thread_num();
        #pragma omp critical
        cout << "je suis le thread " << id << " pour pid=" << pid << endl;
    }

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    
    int *matrice = new int[n * n]; // la matrice
    int *vecteurs; // l'ensemble des vecteurs connu uniquement par root et distribué à tous.

    int* sendbuf;
    int* recvbuf;
    int* sendcounts;
    int* displs;

    int n_local = ceil(n / float(nprocs));

    cout << "N_Local:" << n_local << endl;

    fstream f;
    matrice = new int[n * n];

    if (pid == root) {
        f.open(name, std::fstream::out);
        srand(time(NULL));

        for (int i = 0; i < n * n; i++)
            matrice[i] = rand() % 20;

        f << "Matrice" << endl;
        for (int i = 0; i < n; i++) {
	        for (int j = 0; j < n; j++)
                f << matrice[i * n + j] << " ";
            f << endl;
        }
        f << endl;

        vecteurs = new int[m * n];

        for (int i = 0; i < m; i++) {
            int nb_zero = rand() % (n / 2);
            generation_vecteur(n, vecteurs + i * n, nb_zero);
        }
        f << "Les vecteurs" << endl;
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++)
                f << vecteurs[i * n + j] << " ";
            f << endl;
        }
    }

    if (pid == root)
      debut = chrono::system_clock::now();


    // A compléter à partir d'ici.

    // MPI_Bcast(matrice, n * n, MPI_INT, root, MPI_COMM_WORLD);

    if (pid == root) {
        sendbuf = (int *)malloc(nprocs * sizeof(int));

        //SEND COUNT
        sendcounts = new int[nprocs];

        int div = n / nprocs;
        int mod = n % nprocs;
        for (size_t i = 0; i < nprocs; i++)
            sendcounts[i] = div;
        for (size_t i = 0; i < mod; i++)
            sendcounts[i] += 1;

        //DISPLS
        displs = new int[nprocs];
        for (size_t i = 0; i < nprocs; i++)
        {
            if (i == 0)
                displs[i] = 0;
            else
                displs[i] = sendcounts[i-1] + sendcounts[i];
        }

        for (size_t i = 0; i < nprocs; i++)
        {
            cout << "Sendcout " << sendcounts[i] << endl;
            cout << "Displs " << displs[i] << endl;
        }
    }

    recvbuf = (int *)malloc(n_local * sizeof(int));

    // MPI_Scatterv(sendbuf, sendcounts, displs, MPI_INT, recvbuf, n_local, MPI_INT, root, MPI_COMM_WORLD);


    // Dans le temps écoulé on ne s'occupe que de la partie communications et calculs
    // (on oublie la génération des données et l'écriture des résultats sur le fichier de sortie)
    if (pid == root) {
        fin = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = fin - debut;
        cout << "temps en secondes : " << elapsed_seconds.count() << endl;

    }


    int *resultats;
    if (pid == root)
        resultats = new int[m * n];
    
    // A compléter également avec la récupération des résultats
    
    
    if (pid == root) {
      f << "Les vecteurs" << endl;
      for (int i = 0; i < m; i++) {
	for (int j = 0; j < n; j++)
	  f << resultats[i * n + j] << " ";
	f << endl;
      }
      f.close();
    }
    

    MPI_Finalize();
    return 0;
}
