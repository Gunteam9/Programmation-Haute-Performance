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
	
	// unsigned int offset = 0;

	MPI_Comm intracom;
	MPI_Intercomm_merge (intercom, 0, &intracom);
	int pid_intra;
	MPI_Comm_rank (intracom, &pid_intra);

	cout << "PID intra master: " << pid_intra << endl;

	MPI_Win theWin;
	MPI_Win_create(tab, n * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &theWin); // déclaration de la fenêtre
	
    MPI_Win_fence(0, theWin);

	//Ici on envoi les données via un get de l'autre coté

	MPI_Win_free(&theWin);

	MPI_Win resWin;
	int* res = new int[nslaves];
	MPI_Win_create(res, nslaves * sizeof(int), sizeof(int), MPI_INFO_NULL, intracom, &resWin);

	MPI_Win_fence(0, resWin);

	//On libère la fenetre rapidement pour s'assurer que les données ont bien été déposées.
	MPI_Win_free(&resWin);

	cout << "Res: ";
	for (size_t i = 0; i < nslaves; i++)
		cout << res[i] << " ";
	cout << endl;
	
	int* smallest = std::min_element(res, res + nslaves);
	cout << "Le minimum est " << *smallest << endl;


	MPI_Comm_free(&intercom);
	MPI_Comm_free(&intracom);
	MPI_Finalize();
	return 0;
}
