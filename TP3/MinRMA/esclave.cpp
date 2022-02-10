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
	unsigned int offset = 0;

	if (pid < mod)
		nlocal += 1;

	if (pid < mod)
		offset = nlocal * pid;
	else
		offset = nlocal * pid + mod;

	cout << "PID - nlocal - offset: " << pid << " " << nlocal << " " << offset << endl;

	int* tab = new int[nlocal];

	MPI_Comm intracom;
	MPI_Intercomm_merge (intercom, 1, &intracom);
	int pid_intra;
	MPI_Comm_rank (intracom, &pid_intra);
	
	MPI_Win theWin;
	// int* tmp = new int[n] {0};
	// MPI_Win_create(tab, nlocal * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &theWin); // déclaration de la fenêtre
	MPI_Win_create(tab, nlocal * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &theWin); // déclaration de la fenêtre
	
	MPI_Win_fence(0, theWin);

	// On utilise des lock car plus opti
	// MPI_Win_lock( int lock_type , int rank , int assert , MPI_Win win);	
	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, theWin);

	// MPI_Get( void* origin_addr , int origin_count , MPI_Datatype origin_datatype , int target_rank , MPI_Aint target_disp , int target_count , MPI_Datatype target_datatype , MPI_Win win);
	MPI_Get(tab, nlocal, MPI_INT, 0, offset, nlocal, MPI_INT, theWin);
	// MPI_Get(tmp, n, MPI_INT, 0, 0, n, MPI_INT, theWin);

	// MPI_Win_unlock( int rank , MPI_Win win);
	MPI_Win_unlock(0, theWin);

	// On libère la fenetre car on en a plus besoin
	MPI_Win_free(&theWin);

	cout << "TAB de " << pid << " : ";
	for (size_t i = 0; i < nlocal; i++)
	{
		cout << tab[i] << " ";
	}

	cout << endl;
	
	int* smallest = std::min_element(tab, tab + nlocal);
	cout << "L'esclave " << pid << " a " << *smallest << " comme minimum" << endl;

	MPI_Win resWin;
	MPI_Win_create(smallest, 1 * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &resWin);

	MPI_Win_fence(0, resWin);

	MPI_Win_lock(MPI_LOCK_SHARED, 0, 0, resWin);

	// MPI_Put( const void* origin_addr , int origin_count , MPI_Datatype origin_datatype , int target_rank , MPI_Aint target_disp , int target_count , MPI_Datatype target_datatype , MPI_Win win);
	MPI_Put(smallest, 1, MPI_INT, 0, pid, 1, MPI_INT, resWin);

	MPI_Win_unlock(0, resWin);

	MPI_Win_free(&resWin);

	MPI_Finalize();
	return 0;
}
