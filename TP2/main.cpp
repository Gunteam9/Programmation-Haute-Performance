#include <iostream>
#include <mpi.h>
#include <time.h>
#include <unistd.h>
using namespace std;
int main(int argc, char**argv) {

    int nprocs;
    int pid;

    MPI_Init (&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&pid);

    MPI_Win TheWin;

    if (pid==0) {
        MPI_Win_create(NULL, 0, 1, MPI_INFO_NULL, MPI_COMM_WORLD,&TheWin);
        int a = 1;
        int b = 10;
        MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, TheWin);
        MPI_Put(&a, 1, MPI_INT, 1, 0, 1, MPI_INT, TheWin);
        MPI_Put(&b,1,MPI_INT,1,1,1,MPI_INT,TheWin);
        MPI_Win_unlock(1, TheWin);
    }
    if (pid==1) {
        int* tab;
        MPI_Alloc_mem(2*sizeof(int),MPI_INFO_NULL,(void*)tab);
        MPI_Win_create(tab, 2*sizeof(int), sizeof(int),MPI_INFO_NULL, MPI_COMM_WORLD,&TheWin);
        cout << "tab " << tab[0] << " " << tab[1] << endl;
    }
    MPI_Win_free(&TheWin);

    MPI_Finalize();

    return 0;
}
