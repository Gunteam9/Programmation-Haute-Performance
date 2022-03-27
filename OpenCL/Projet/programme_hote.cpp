#define __CL_ENABLE_EXCEPTIONS

#include "CL/cl.hpp"
#include "omp.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <ctime>  
#include <map>

using namespace std;

#pragma warning(disable : 26451 6031)

int nx;
int ny;
int gauche;
int droite;
int cell;
int nodata;
float* inputMat;
float* outputMat;

enum class Direction {
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	CENTER_LEFT,
	CENTER_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT,
	NONE
};

struct cellData {
	vector<cellData*> source;
	bool hasFinalValue = false;
	float value = 0;
};

string direction_arrow_converter(Direction dir) {
	switch (dir)
	{
	case Direction::TOP_LEFT:
		return "↖";
		break;
	case Direction::TOP_CENTER:
		return "↑";
		break;
	case Direction::TOP_RIGHT:
		return "↗";
		break;
	case Direction::CENTER_LEFT:
		return "←";
		break;
	case Direction::CENTER_RIGHT:
		return "→";
		break;
	case Direction::BOTTOM_LEFT:
		return "↙";
		break;
	case Direction::BOTTOM_CENTER:
		return "↓";
		break;
	case Direction::BOTTOM_RIGHT:
		return "↘";
		break;
	case Direction::NONE:
		return " ";
		break;
	}
}

// fonction permettant de récupérer le temps écoulé entre debut et fin
double calcTime(chrono::time_point<chrono::system_clock> start, chrono::time_point<chrono::system_clock> end) {
	chrono::duration<double> tps = end - start;
	return tps.count();
}

void loadData() {
	FILE* fp = fopen("data.txt", "r");
	if (fp == nullptr) {
		cerr << "File not found" << endl;
		exit(0);
	}

	fscanf(fp, "%d", &nx);
	fscanf(fp, "%d", &ny);
	fscanf(fp, "%d", &gauche);
	fscanf(fp, "%d", &droite);
	fscanf(fp, "%d", &cell);
	fscanf(fp, "%d", &nodata);

	inputMat = new float[nx * ny];
	outputMat = new float[nx * ny];


	for (int i = 0; i < nx; i++) {
		for (int j = 0; j < ny; j++) {
			float tmp;
			fscanf(fp, "%f", &tmp);
			inputMat[i * nx + j] = tmp;
		}
	}
	fclose(fp);
}

void print_dir(const string* vec, const int xSize, const int ySize) {
	for (int i = 0; i < xSize; i++) {
		for (int j = 0; j < ySize; j++) {
			cout << vec[i * xSize + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void print_dir(const Direction* vec, const int xSize, const int ySize) {
	ofstream o;
	o.open("out.txt");
	for (int i = 0; i < xSize; i++) {
		for (int j = 0; j < ySize; j++) {
			o << direction_arrow_converter(vec[i * xSize + j]) << " ";
		}
		o << endl;
	}
	o.close();
	cout << endl;
}

void print_res(const float* vec, const int xSize, const int ySize) {
	for (int i = 0; i < xSize; i++) {
		for (int j = 0; j < ySize; j++) {
			if (vec[i * xSize + j] < 10)
				cout << "0" << vec[i * xSize + j] << " ";
			else
				cout << vec[i * xSize + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

cl::Program createProgram(std::string sourceFileName, cl::Context context) {
	// lecture du programme source
	std::ifstream sourceFile(sourceFileName);
	std::string sourceCode(std::istreambuf_iterator <char>(sourceFile), (std::istreambuf_iterator < char >()));
	// la premier argument indique le nombre de programmes sources utilisés, le deuxième est une paire (texte, taille du programme)
	cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
	// creation du programme dans le contexte
	return cl::Program(context, source);
}


void test_CPU() {
	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
	Direction* out_dir = new Direction[nx * ny];
	vector<cellData*> cell_data;

	for (size_t i = 0; i < nx; i++)
	{
		for (size_t j = 0; j < ny; j++)
		{
			cell_data.push_back(new cellData());
		}
	}


	for (size_t i = 0; i < nx; i++)
	{
		for (size_t j = 0; j < ny; j++)
		{
			float minValue = CL_MAXFLOAT;
			Direction dir = Direction::NONE;

			int dest = 0;

			// TOP
			if (i > 0 && j > 0 && inputMat[(i - 1) * nx + j - 1] <= minValue) {
				minValue = inputMat[(i - 1) * nx + j - 1];
				dir = Direction::TOP_LEFT;
				dest = (i - 1) * nx + j - 1;
			}
			if (i > 0 && inputMat[(i - 1) * nx + j + 0] <= minValue) {
				minValue = inputMat[(i - 1) * nx + j + 0];
				dir = Direction::TOP_CENTER;
				dest = (i - 1) * nx + j + 0;
			}
			if (i > 0 && j < (ny - 1) && inputMat[(i - 1) * nx + j + 1] <= minValue) {
				minValue = inputMat[(i - 1) * nx + j + 1];
				dir = Direction::TOP_RIGHT;
				dest = (i - 1) * nx + j + 1;
			}

			// CENTER
			if (j > 0 && inputMat[(i + 0) * nx + j - 1] <= minValue) {
				minValue = inputMat[(i + 0) * nx + j - 1];
				dir = Direction::CENTER_LEFT;
				dest = (i + 0) * nx + j - 1;
			}
			if (inputMat[(i + 0) * nx + j + 0] <= minValue) {
				minValue = inputMat[(i + 0) * nx + j + 0];
				dir = Direction::NONE;
				dest = (i + 0) * nx + j + 0;
			}
			if (j < (ny - 1) && inputMat[(i + 0) * nx + j + 1] <= minValue) {
				minValue = inputMat[(i + 0) * nx + j + 1];
				dir = Direction::CENTER_RIGHT;
				dest = (i + 0) * nx + j + 1;
			}

			// BOTTOM
			if (i < (nx - 1) && j > 0 && inputMat[(i + 1) * nx + j - 1] <= minValue) {
				minValue = inputMat[(i + 1) * nx + j - 1];
				dir = Direction::BOTTOM_LEFT;
				dest = (i + 1) * nx + j - 1;
			}
			if (i < (nx - 1) && inputMat[(i + 1) * nx + j + 0] <= minValue) {
				minValue = inputMat[(i + 1) * nx + j + 0];
				dir = Direction::BOTTOM_CENTER;
				dest = (i + 1) * nx + j + 0;
			}
			if (i < (nx - 1) && j < (ny - 1) && inputMat[(i + 1) * nx + j + 1] <= minValue) {
				minValue = inputMat[(i + 1) * nx + j + 1];
				dir = Direction::BOTTOM_RIGHT;
				dest = (i + 1) * nx + j + 1;
			}

			if (dir != Direction::NONE)
				cell_data[dest]->source.push_back(cell_data[i * nx + j]);
			out_dir[i * nx + j] = dir;
		}
	}

	print_dir(out_dir, nx, ny);

	delete[] out_dir;

	int cmpt = 0;

	while (cmpt < nx * ny) {
		for (size_t i = 0; i < nx; i++)
		{
			for (size_t j = 0; j < ny; j++)
			{
				if (!cell_data[i * nx + j]->hasFinalValue) {
					for (int k = cell_data[i * nx + j]->source.size() - 1; k >= 0; k--)
					{
						cellData* neightbor = cell_data[i * nx + j]->source[k];
						if (neightbor->hasFinalValue) {
							cell_data[i * nx + j]->value += neightbor->value;
							cell_data[i * nx + j]->source.erase(cell_data[i * nx + j]->source.begin() + k);
							delete neightbor;
						}
					}


					if (cell_data[i * nx + j]->source.empty()) {
						cell_data[i * nx + j]->hasFinalValue = true;
						cell_data[i * nx + j]->value += 1;
						outputMat[i * nx + j] = cell_data[i * nx + j]->value;
						cmpt += 1;
					}
				}
			}
		}
	}

	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
	cout << "Résultat CPU" << endl;
	//print_res(out, size, size);

	cout << "Temps execution CPU: " << calcTime(start, end) << endl;
}

void test_CPU_omp() {
	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
	Direction* out_dir = new Direction[nx * ny];
	vector<cellData*> cell_data;

	for (int i = 0; i < nx; i++)
	{
		for (int j = 0; j < ny; j++)
		{
			cell_data.push_back(new cellData());
		}
	}

#pragma omp parallel for collapse(2)
	for (int i = 0; i < nx; i++)
	{
		for (int j = 0; j < ny; j++)
		{
			float minValue = CL_MAXFLOAT;
			Direction dir = Direction::NONE;

			int dest = 0;

			// TOP
			if (i > 0 && j > 0 && inputMat[(i - 1) * nx + j - 1] <= minValue) {
				minValue = inputMat[(i - 1) * nx + j - 1];
				dir = Direction::TOP_LEFT;
				dest = (i - 1) * nx + j - 1;
			}
			if (i > 0 && inputMat[(i - 1) * nx + j + 0] <= minValue) {
				minValue = inputMat[(i - 1) * nx + j + 0];
				dir = Direction::TOP_CENTER;
				dest = (i - 1) * nx + j + 0;
			}
			if (i > 0 && j < (ny - 1) && inputMat[(i - 1) * nx + j + 1] <= minValue) {
				minValue = inputMat[(i - 1) * nx + j + 1];
				dir = Direction::TOP_RIGHT;
				dest = (i - 1) * nx + j + 1;
			}

			// CENTER
			if (j > 0 && inputMat[(i + 0) * nx + j - 1] <= minValue) {
				minValue = inputMat[(i + 0) * nx + j - 1];
				dir = Direction::CENTER_LEFT;
				dest = (i + 0) * nx + j - 1;
			}
			if (inputMat[(i + 0) * nx + j + 0] <= minValue) {
				minValue = inputMat[(i + 0) * nx + j + 0];
				dir = Direction::NONE;
				dest = (i + 0) * nx + j + 0;
			}
			if (j < (ny - 1) && inputMat[(i + 0) * nx + j + 1] <= minValue) {
				minValue = inputMat[(i + 0) * nx + j + 1];
				dir = Direction::CENTER_RIGHT;
				dest = (i + 0) * nx + j + 1;
			}

			// BOTTOM
			if (i < (nx - 1) && j > 0 && inputMat[(i + 1) * nx + j - 1] <= minValue) {
				minValue = inputMat[(i + 1) * nx + j - 1];
				dir = Direction::BOTTOM_LEFT;
				dest = (i + 1) * nx + j - 1;
			}
			if (i < (nx - 1) && inputMat[(i + 1) * nx + j + 0] <= minValue) {
				minValue = inputMat[(i + 1) * nx + j + 0];
				dir = Direction::BOTTOM_CENTER;
				dest = (i + 1) * nx + j + 0;
			}
			if (i < (nx - 1) && j < (ny - 1) && inputMat[(i + 1) * nx + j + 1] <= minValue) {
				minValue = inputMat[(i + 1) * nx + j + 1];
				dir = Direction::BOTTOM_RIGHT;
				dest = (i + 1) * nx + j + 1;
			}

			if (dir != Direction::NONE)
				cell_data[dest]->source.push_back(cell_data[i * nx + j]);
			out_dir[i * nx + j] = dir;
		}
	}

	print_dir(out_dir, nx, ny);

	delete[] out_dir;

	int cmpt = 0;

	while (cmpt < nx * ny) {
		for (int i = 0; i < nx; i++)
		{
			for (int j = 0; j < ny; j++)
			{
				if (!cell_data[i * nx + j]->hasFinalValue) {
					for (int k = cell_data[i * nx + j]->source.size() - 1; k >= 0; k--)
					{
						cellData* neightbor = cell_data[i * nx + j]->source[k];
						if (neightbor->hasFinalValue) {
							cell_data[i * nx + j]->value += neightbor->value;
							cell_data[i * nx + j]->source.erase(cell_data[i * nx + j]->source.begin() + k);
							delete neightbor;
						}
					}


					if (cell_data[i * nx + j]->source.empty()) {
						cell_data[i * nx + j]->hasFinalValue = true;
						cell_data[i * nx + j]->value += 1;
						outputMat[i * nx + j] = cell_data[i * nx + j]->value;
						cmpt += 1;
					}
				}
			}
		}
	}
	
	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
	cout << "Résultat CPU OpenMP" << endl;
	//print_res(outputMat, nx, ny);
	cout << "Temps execution CPU OpenMP: " << calcTime(start, end) << endl;
}

void test_GPU(const cl::Program& program, const cl::CommandQueue& queue, const cl::Context& context) {
	chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
	const size_t byteSizeBufferA = sizeof(float) * nx * ny;
	//const size_t byteSizeBufferB = sizeof(int) * nx * ny;
	// Création des buffers de données dans le context
	cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, byteSizeBufferA);
	cl::Buffer bufferB = cl::Buffer(context, CL_MEM_WRITE_ONLY, byteSizeBufferA);

	// Chargement des données en mémoire video
	queue.enqueueWriteBuffer(bufferA, CL_TRUE, 0, byteSizeBufferA, inputMat);
	// creation du kernel (fonction à exécuter)
	cl::Kernel kernel(program, "test_neighbors");
	// Attribution des paramètres de ce kernel
	kernel.setArg(0, nx);
	kernel.setArg(1, ny);
	kernel.setArg(2, bufferA);
	kernel.setArg(3, bufferB);


	// création de la topologie des processeurs
	cl::NDRange global(nx, ny); // nombre total d'éléments de calcul -processing elements
	cl::NDRange local(4, 4); // dimension des unités de calcul -compute units- c'à-dire le nombre d'éléments de calcul par unités de calcul

	// lancement du programme en GPU
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

	// recupération du résultat
	float* out = new float[nx * ny];
	queue.enqueueReadBuffer(bufferB, CL_TRUE, 0, byteSizeBufferA, out);

	for (size_t i = 0; i < nx; i++)
	{
		for (size_t j = 0; j < ny; j++)
		{
			cout << out[i * nx + j] << " ";
		}
		cout << endl;
	}

	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();

	cout << "Résultat GPU" << endl;
	//print_res(outputMat, nx, ny);
	cout << "Temps execution GPU: " << calcTime(start, end) << endl;
}

int main() {
	// pour mesurer le temps
	chrono::time_point<chrono::system_clock> debut, debut2, fin;

	// création des zone de stockage de données en mémoire centrale

	try { // debut de la zone d'utilisation de l'API pour OpenCL
		// les plateformes
		vector<cl::Platform> plateformes;
		cl::Platform::get(&plateformes); // recherche des plateformes normalement 1 sur un PC

		//les devices
		vector<cl::Device> devices;
		plateformes[0].getDevices(CL_DEVICE_TYPE_ALL, &devices); // recherche des devices (normalement 1)

		// création d'un contexte pour les devices
		cl::Context context(devices);

		// création du programme dans le contexte (voir code fonction)
		cl::Program program = createProgram("ocl.cl", context);
		// compilation du programme
		try {
			program.build(devices);
		}
		catch (...) {
			// Récupération des messages d'erreur au cas où...
			cl_int buildErr = CL_SUCCESS;
			auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &buildErr);
			cerr << buildInfo << endl << endl;
			int t;
			cin >> t;
			exit(0);
		}

		// création de la file de commandes (ordres de l'hote pour le GPU)
		cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);

		loadData();
		// affichage des données initialisées
		cout << "Données initialisées" << endl;
		//printData(inputMat, nx, ny);

		test_CPU();
		test_CPU_omp();
		test_GPU(program, queue, context);
		system("pause");
	}
	catch (cl::Error& err) { // Affichage des erreurs en cas de pb OpenCL
		cout << "Exception\n";
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
		system("pause");
		return EXIT_FAILURE;
	}
}


#pragma warning(restore : 26451 6031)