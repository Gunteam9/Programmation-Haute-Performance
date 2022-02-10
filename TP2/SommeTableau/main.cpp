#include <iostream>
#include <mpi.h>
#include <chrono>
using namespace std;
int main(int argc, char**argv) {

    int nprocs;
    int pid;

    MPI_Init (&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);

    int count = atoi(argv[1]);
    if (count == 0)
        count = nprocs;

    const int dataLength = nprocs * count;
    const int localLength = count;

    // Pour mesurer le temps (géré par le processus root)
    chrono::time_point<chrono::system_clock> debut, fin;

    MPI_Win TheWin; // Déclaration de la fenêtre

    int* tab = (int*)calloc(dataLength, sizeof(int));
    // int* tabLocal = (int*)calloc(localLength, sizeof(int));
    if (pid == 0) {
        srand(time(NULL));
        for (int i = 0; i < dataLength; i++)
            tab[i] = rand() % 10;

        if (count < 10) {
            cout << "AVANT lES FONCTIONS MPI" << endl;

            cout << "Pour le proc " << pid << ", j'ai le tableau ";
            for (int j = 0; j < dataLength; j++) {
                cout << tab[j] << ", ";
            }
            cout << endl;
            cout << endl;
            cout << endl;

            cout << "APRES LE GET" << endl;
        }
    }

    if (pid == 0)
        debut = chrono::system_clock::now();

    // Création de la fenêtre associée au tableau tab
    // MPI_INFO_NULL pour indiquer qu'on ne donne aucune information
    MPI_Win_create(tab, dataLength * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &TheWin);

    // Première barrière sans assertion (on peut en donner pour indiquer des règles sur les instructions avant la barrière)
    MPI_Win_fence(0, TheWin);

    if (pid != 0)
        MPI_Get(tab, localLength, MPI_INT, 0, pid * nprocs, localLength, MPI_INT, TheWin); // On va chercher la donnée dans la fenêtre distante pour la copier dans tab

    // for (int i = 0; i < nprocs; i++) {
    //     if (i != pid) {
    //         //MPI_Get(void *origin_addr, int origin_count, MPI_Datatype, origin_datatype, int target_rank, MPI_Aint target_disp, int target_count,
    //         //MPI_Datatype target_datatype, MPI_Win win)
    //         MPI_Get(tab + i * nprocs, localLength, MPI_INT, i, localLength * pid, localLength, MPI_INT, TheWin); // On va chercher la donnée dans la fenêtre distante pour la copier dans tab
    //     }
    // }

    // Barrière de fin sans assertion (on peut en donner pour indiquer des règles sur les instructions entre les 2 barrières)
    MPI_Win_fence(0, TheWin);

    if (count < 10) {
        cout << "Pour le proc " << pid << ", j'ai le tableau ";
        for (int j = 0; j < dataLength; j++) {
            cout << tab[j] << ", ";
        }
        cout << endl;
    }

    // MPI_Accumulate( const void* origin_addr , int origin_count , MPI_Datatype origin_datatype , int target_rank , MPI_Aint target_disp , int target_count , MPI_Datatype target_datatype , MPI_Op op , MPI_Win win);
    if (pid != 0)
        MPI_Accumulate(tab, localLength, MPI_INT, 0, 0, localLength, MPI_INT, MPI_SUM, TheWin);

    // for (int i = 0; i < nprocs; i++) {
    //     if (i != pid) {
    //         MPI_Accumulate(tab, localLength , MPI_INT , i , 0 , localLength , MPI_INT , MPI_SUM , TheWin);
    //     }
    // }

    MPI_Win_fence(0, TheWin);

    // Dans le temps écoulé on ne s'occupe que de la partie communications et calculs
    // (on oublie la génération des données et l'écriture des résultats sur le fichier de sortie)
    if (pid == 0) {
        fin = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = fin - debut;
        cout << "Temps en secondes : " << elapsed_seconds.count() << endl;

    }

    if (pid == 0) {
        if (count < 10) {
            cout << endl;
            cout << endl;
            
            cout << "APRES lE ACCUMULATE" << endl;
            cout << "Pour le proc " << pid << ", j'ai le tableau ";
            for (int j = 0; j < dataLength; j++) {
                cout << tab[j] << ", ";
            }
            cout << endl;
        }
    }

    MPI_Win_free(&TheWin);
    MPI_Finalize();

    free(tab);
    // free(tabLocal);

    return 0;
}
