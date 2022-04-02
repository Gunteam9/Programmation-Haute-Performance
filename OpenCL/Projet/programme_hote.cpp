#define __CL_ENABLE_EXCEPTIONS

#include "CL/cl.hpp"
#include "omp.h"

#include <vector>
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <map>

#include "functions.h"

using namespace std;

// Disable CS Error for Visual Studio
#pragma warning(disable : 26451 6031)

int nx;
int ny;
int gauche;
int droite;
int cell;
int nodata;
float* inputMat;
float* outputMat;

// Load the data in file
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
	const size_t byteSizeBufferB = sizeof(float) * nx * ny * 2;
	float* dir = new float[nx * ny * 2]{ 0 };
	vector<cellData*> dir_cell_data;
	// Création des buffers de données dans le context
	cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, byteSizeBufferA);
	cl::Buffer bufferB = cl::Buffer(context, CL_MEM_WRITE_ONLY, byteSizeBufferB);

	// Chargement des données en mémoire video
	//queue.enqueueWriteBuffer(bufferA, CL_TRUE, 0, byteSizeBuffer, inputMat);
	queue.enqueueWriteBuffer(bufferA, CL_TRUE, 0, byteSizeBufferA, inputMat);
	// creation du kernel (fonction à exécuter)
	cl::Kernel kernel(program, "test_neighbors");
	// Attribution des paramètres de ce kernel
	//kernel.setArg(0, nx);
	//kernel.setArg(1, ny);
	kernel.setArg(0, nx);
	kernel.setArg(1, ny);
	kernel.setArg(2, bufferA);
	kernel.setArg(3, bufferB);


	// création de la topologie des processeurs
	cl::NDRange global(nx, ny); // nombre total d'éléments de calcul -processing elements
	cl::NDRange local(25, 25); // dimension des unités de calcul -compute units- c'à-dire le nombre d'éléments de calcul par unités de calcul

	// lancement du programme en GPU
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

	// recupération du résultat
	queue.enqueueReadBuffer(bufferB, CL_TRUE, 0, byteSizeBufferB, dir);

	chrono::time_point<chrono::system_clock> startInt = chrono::system_clock::now();

	for (size_t i = 0; i < nx; i++)
	{
		for (size_t j = 0; j < ny; j++)
		{
			dir_cell_data.push_back(new cellData());
		}
	}

	for (size_t i = 0; i < nx; i++)
	{
		for (size_t j = 0; j < ny; j++)
		{
			int from = dir[(i * nx + j) * 2];
			int to = dir[(i * nx + j) * 2 + 1];
			if (from != -1 && to != -1)
				dir_cell_data[to]->source.push_back(dir_cell_data[from]);
		}
	}

	chrono::time_point<chrono::system_clock> endInt = chrono::system_clock::now();

	delete[] dir;

	int cmpt = 0;

	while (cmpt < nx * ny) {
		for (int i = 0; i < nx; i++)
		{
			for (int j = 0; j < ny; j++)
			{
				if (!dir_cell_data[i * nx + j]->hasFinalValue) {
					for (int k = dir_cell_data[i * nx + j]->source.size() - 1; k >= 0; k--)
					{
						cellData* neightbor = dir_cell_data[i * nx + j]->source[k];
						if (neightbor->hasFinalValue) {
							dir_cell_data[i * nx + j]->value += neightbor->value;
							dir_cell_data[i * nx + j]->source.erase(dir_cell_data[i * nx + j]->source.begin() + k);
							delete neightbor;
						}
					}

					if (dir_cell_data[i * nx + j]->source.empty()) {
						dir_cell_data[i * nx + j]->hasFinalValue = true;
						dir_cell_data[i * nx + j]->value += 1;
						outputMat[i * nx + j] = dir_cell_data[i * nx + j]->value;
						cmpt += 1;
					}
				}
			}
		}
	}
	

	chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();

	cout << "Résultat GPU" << endl;
	//print_res(outputMat, nx, ny);
	cout << "Temps intermediaire GPU: " << calcTime(startInt, endInt) << endl;
	cout << "Temps execution GPU: " << calcTime(start, end) << endl;
}

int main() {
	// OpenCL pipeline
	try {
		// Plateformes
		vector<cl::Platform> plateformes;
		cl::Platform::get(&plateformes);

		// Devices
		vector<cl::Device> devices;
		plateformes[0].getDevices(CL_DEVICE_TYPE_ALL, &devices);

		// Context
		cl::Context context(devices);

		// Program
		cl::Program program = createProgram("ocl.cl", context);
		try {
			program.build(devices);
		}
		catch (...) {
			// Errors
			cl_int buildErr = CL_SUCCESS;
			auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &buildErr);
			cerr << buildInfo << endl << endl;
			int t;
			cin >> t;
			exit(0);
		}

		// CommandQueue
		cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);

		// Initialize data
		loadData();
		cout << "Données initialisées" << endl;

		// Calc
		test_CPU();
		test_CPU_omp();
		test_GPU(program, queue, context);

		delete[] inputMat;
		delete[] outputMat;

#ifdef _WIN32 || _WIN64
		system("pause");
#endif

	}
	catch (cl::Error& err) {
		cout << "Exception\n";
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
		system("pause");
		return EXIT_FAILURE;
	}
}


#pragma warning(restore : 26451 6031)