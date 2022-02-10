#include <mpi.h>
#include <iostream>
#include <algorithm>
using namespace std;

int main(int argc, char **argv)
{
	int nslaves, pid, nmasters, flag;
	MPI_Comm intercom;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nslaves);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	MPI_Comm_get_parent(&intercom);			   // obtention de l'intercommunicateur vers le/les parents
	MPI_Comm_remote_size(intercom, &nmasters); // permet de connaître le nombre de parents
	// code de l'esclave

	cout << "je suis l'esclave " << pid << " parmi " << nslaves << " esclaves et avec " << nmasters << " parents" << endl;

	// Attention pour les esclaves on a esclave ./maitre 16 4 0 donc n est l'argument n°2
	int n = atoi(argv[2]);
	int root = atoi(argv[4]);

	unsigned int nlocal = n / nslaves;
	unsigned int mod = n % nslaves;

	if (pid < mod)
		nlocal += 1;

	int* tab = new int[nlocal];
	MPI_Recv(tab, nlocal, MPI_INT, root, 10, intercom, &status);
	
	int* smallest = std::min_element(tab, tab + nlocal);
	cout << "L'esclave " << pid << " a " << *smallest << " comme minimum" << endl;

	MPI_Ssend(smallest, 1, MPI_INT, root, 10, intercom);

	MPI_Finalize();
	return 0;
}
