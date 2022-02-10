#include <iostream>
#include <omp.h>
#include <mpi.h>

using namespace std;

int main(int argc , char **argv ) {

    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int pid, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &pid );
    MPI_Comm_size (MPI_COMM_WORLD, &nprocs );

#pragma omp parallel num_threads(4)
    {
     int num = omp_get_thread_num();
#pragma omp critical
     cout << "je suis " << pid << "/" << num << endl;
    }

    MPI_Finalize();

    return 0;
}
