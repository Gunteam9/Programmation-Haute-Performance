#include <mpi.h>
#include <iostream>
#include <algorithm>
using namespace std;

int main(int argc, char **argv)
{
	int nmasters, pid;
	MPI_Comm intercom;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nmasters);
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// le code des maîtres

	int n = atoi(argv[1]);
	int nslaves = atoi(argv[2]);
	int root = atoi(argv[3]);

	if (nmasters > 1)
	{
		cout << "Un processus maitre max" << endl;
		exit(1);
	}

	cout << "je suis le maître " << pid << " parmi " << nmasters << " maîtres" << endl;
	int *tab = new int[n];

	cout << "Tab: ";
	srand(time(NULL));
	for (size_t i = 0; i < n; i++)
	{
		tab[i] = rand() % 20;
		cout << tab[i] << " ";
	}
	cout << endl;

	MPI_Comm_spawn("esclave",		   // exécutable
				   argv,			   // arguments à passer sur la ligne de commande
				   nslaves,			   // nombre de processus esclave
				   MPI_INFO_NULL,	   //permet de préciser où et comment lancer les processus
				   root,			   // rang du processus maître qui effectue réellement le spawn
				   MPI_COMM_WORLD,	   // Monde dans lequel est effectué le spwan
				   &intercom,		   // l'intercommunicateur permettant de communiquer avec les esclaves
				   MPI_ERRCODES_IGNORE // tableau contenant les erreurs
	);

	// for (int i = 0; i < nslaves; i++) {
	//     //MPI_Ssend( const void* buf , int count , MPI_Datatype datatype , int dest , int tag , MPI_Comm comm);
	//     MPI_Ssend(tab, n, MPI_INT, i, 10, intercom);
	// }

	unsigned int* nlocal = new unsigned int[nslaves];
	unsigned int mod = n % nslaves;
	
	for (size_t i = 0; i < nslaves; i++)
		nlocal[i] = n / nslaves;	

	for (size_t i = 0; i < mod; i++)
		nlocal[i] += 1;
	
	unsigned int offset = 0;

	for (size_t i = 0; i < nslaves; i++) {
		MPI_Ssend(tab + offset, nlocal[i], MPI_INT, i, 10, intercom);
		offset += nlocal[i];
	}

	MPI_Status status;
	int* res = new int[nslaves];

	for (size_t i = 0; i < nslaves; i++)
		MPI_Recv(res + i, nslaves, MPI_INT, i, 10, intercom, &status);
	
	int* smallest = std::min_element(res, res + nslaves);
	cout << "Le minimum est " << *smallest << endl;

	MPI_Comm_free(&intercom);
	MPI_Finalize();
	return 0;
}
