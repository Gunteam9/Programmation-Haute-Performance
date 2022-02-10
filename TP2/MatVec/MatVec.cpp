#include <iostream>
#include <fstream>
#include <random>
#include <mpi.h>

using namespace std;

#define TAG 10
/// \brief To compute a matrix vector product
/// \param n the number of lines of the matrix
/// \param m the number of vector elements
/// \param matrix the matrix of size n x m
/// \param vector the vector of size m
/// \param res the result (the memory is allocated in the calling program)
void matrix_vector_product(int n, int m, int *matrix, int *vector, int *res)
{
    for (int i = 0; i < n; i++)
    {
        res[i] = 0;
        for (int j = 0; j < m; j++)
            res[i] += matrix[i * m + j] * vector[j];
    }
}

int main(int argc, char **argv)
{

    int pid, nprocs;        // The processus rank and the number of process
    MPI_Init(&argc, &argv); // MPI context initialization
    // From now the program is executed by all the process
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);    //to get the process rank
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs); // to get the number of process of the parallel execution

    int n = atoi(argv[1]);    // the size. The matrix is a n x n matrix and the vector has n elements
    int root = atoi(argv[2]); // a particular process (usually 0) which will be the master for some tasks

    default_random_engine re(time(0));
    uniform_int_distribution<int> distrib{1, 10};

    int *mat;              // the matrix
    int *vec = new int[n]; // the vector
    int *reference;
    if (pid == root)
    { // The unique root process will generate the initial data. It will have to distribute them
        mat = new int[n * n];
        // root allocates memory for the matrix and the vector. Then it initializes the elements
        for (int i = 0; i < n * n; i++)
            mat[i] = distrib(re);
        for (int i = 0; i < n; i++)
            vec[i] = distrib(re);
        // To check the final result, root computes also the product in sequential. But it is only for checking reasons
        reference = new int[n];
        matrix_vector_product(n, n, mat, vec, reference);
    }

    // Hypothesis n is divisible by the number of process
    // n_part is the number of lines that each process will receive
    int n_part = n / nprocs;

    // each process receives n_part lines of the matrix mat in the mat_part variable
    // each process receives the complete vector vec in the variable vec (that is why all the process allocate memory for vec)
    // each process needs an additional variable for the result
    int *mat_part = new int[n_part * n];
    int *res_part = new int[n_part];

    // the initial communications to distribute the matrix and the vector
    // for the moment only the root process has the data

    // with collective communications: a collective communication = an unique function called by all the process
    
    MPI_Scatter(mat, n * n_part, MPI_INT, mat_part, n * n_part, MPI_INT, root, MPI_COMM_WORLD); // distribution of the matrix
    MPI_Bcast(vec, n, MPI_INT, root, MPI_COMM_WORLD);                                           // diffusion of the vector

    // each process computes its matrix vector product
    matrix_vector_product(n_part, n, mat_part, vec, res_part);

    // We want to gather the result on the root process
    int *res;
    if (pid == root)
        res = new int[n];

    MPI_Gather(res_part, n_part, MPI_INT, res, n_part, MPI_INT, root, MPI_COMM_WORLD);

    if (pid == root)
    {
        bool test = true;
        for (int i = 0; i < n; i++)
            if (reference[i] != res[i])
            {
                test = false;
                break;
            }
        if (test == true) {
            cout << "the result is correct" << endl;
            cout << "Res: ";
            for (int i = 0; i < n; i++) {
                cout << res[i] << ", ";
            }
            cout << endl;
        }
        else
            cout << "WRONG" << endl;
    }

    if (pid == root)
    {
        delete[] mat;
        delete[] reference;
        delete[] res;
    }
    delete[] mat_part;
    delete[] res_part;
    delete[] vec;

    MPI_Finalize();

    return 0;
}
