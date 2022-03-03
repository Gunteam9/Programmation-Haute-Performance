#define __CL_ENABLE_EXCEPTIONS

#include "CL/cl.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <ctime>  

using namespace std;

int nx;
int ny;
int left;
int right;
int cell;
int nodata;
int* inputMat;
int* outputMat;

// fonction permettant de récupérer le temps écoulé entre debut et fin
double calcTime(chrono::time_point<chrono::system_clock> start, chrono::time_point<chrono::system_clock> end){
  chrono::duration<double> tps = end - start;
  return tps.count();
}

void loadData(){
    FILE* fp = fopen("data.txt","r");
    if (fp==nullptr) {
        cerr << "File not found" << endl;
        exit(0);
    }

    fscanf(fp,"%d",&nx);
    fscanf(fp,"%d",&ny);
    fscanf(fp,"%d",&left);
    fscanf(fp,"%d",&right);
    fscanf(fp,"%d",&cell);
    fscanf(fp,"%d",&nodata);

    for (int i=0; i<nx; i++) {
        for (int j=0; j<ny; j++) {
            fscanf(fp,"%f",&hauteur);
            tab[i*nx+j] = hauteur;
        }
    }
    fclose(fp);

}

void printData(int* vec, int xSize, int ySize){
    for (int i = 0; i < xSize; i++) {
        for (int j = 0; j < ySize; j++) {
            cout << vec[i * xSize + j] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

cl::Program createProgram(const string& sourceFileString, const cl::Context& context){
    // lecture du programme source
    ifstream sourceFile(sourceFileString);
    string sourceCode(istreambuf_iterator <char>(sourceFile),(istreambuf_iterator<char>()));
    // la premier argument indique le nombre de programmes sources utilisés, le deuxième est une paire (texte, taille du programme)
    cl::Program::Sources source(1, make_pair(sourceCode.c_str(),sourceCode.length()+1));
    // creation du programme dans le contexte
    return cl::Program(context, sourceFileString);
}


void test_CPU(){
    chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
//    for(int i = 0; i < taille; i++) {
//        for(int j = 0; j < taille; j++) {
//            float center = (1 - 4 * lambda) * A[i * taille + j];
//            float top = i - 1 < 0 ? A[(taille - 1) * taille + j] : A[(i - 1) * taille + j];
//            float bottom = i + 1 >= taille ? A[0 * taille + j] : A[(i + 1) * taille + j];
//            float left = j - 1 < 0 ? A[i * taille + taille - 1] : A[i * taille + j - 1];
//            float right = j + 1 >= taille ? A[i * taille + 0] : A[i * taille + j + 1];
//            B[i * taille + j] = center + lambda * (top + bottom + left + right);
//        }
//    }
    chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
    cout << "Résultat CPU" << endl;
    cout << "Temps execution CPU: " << calcTime(start, end) << endl;
    printData(outputMat, nx, ny);
}

void test_CPU_omp(){
    chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
//#pragma omp parallel for
//    for(int i = 0; i < taille; i++) {
//#pragma omp parallel for
//        for(int j = 0; j < taille; j++) {
//            float center = (1 - 4 * lambda) * A[i * taille + j];
//            float top = i - 1 < 0 ? A[(taille - 1) * taille + j] : A[(i - 1) * taille + j];
//            float bottom = i + 1 >= taille ? A[0 * taille + j] : A[(i + 1) * taille + j];
//            float left = j - 1 < 0 ? A[i * taille + taille - 1] : A[i * taille + j - 1];
//            float right = j + 1 >= taille ? A[i * taille + 0] : A[i * taille + j + 1];
//            B[i * taille + j] = center + lambda * (top + bottom + left + right);
//        }
//    }
    chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();
    cout << "Résultat CPU OpenMP" << endl;
    cout << "Temps execution CPU OpenMP: " << calcTime(start, end) << endl;
    printData(outputMat, nx, ny);
}

void test_GPU(const cl::Program& program, const cl::CommandQueue& queue, const cl::Context& context){
    chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
    const size_t byteSize = sizeof(float) * nx * ny;
    // Création des buffers de données dans le context
    cl::Buffer bufferA = cl::Buffer(context, CL_MEM_READ_ONLY, byteSize);
    cl::Buffer bufferB = cl::Buffer(context, CL_MEM_WRITE_ONLY, byteSize);

    // Chargement des données en mémoire video
    queue.enqueueWriteBuffer(bufferA , CL_TRUE, 0, byteSize , inputMat);
    // creation du kernel (fonction à exécuter)
    cl::Kernel kernel(program, "stencil");
    // Attribution des paramètres de ce kernel
    kernel.setArg(0, nx);
    kernel.setArg(1, ny);
    kernel.setArg(2, bufferA);
    kernel.setArg(3, bufferB);

    // création de la topologie des processeurs
    cl::NDRange global(nx, ny); // nombre total d'éléments de calcul -processing elements
    cl::NDRange local(16, 16); // dimension des unités de calcul -compute units- c'à-dire le nombre d'éléments de calcul par unités de calcul

    // lancement du programme en GPU
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

    // recupération du résultat
    queue.enqueueReadBuffer(bufferB, CL_TRUE,0, byteSize, outputMat);
    chrono::time_point<chrono::system_clock> end = chrono::system_clock::now();

    cout << "Résultat GPU" << endl;
    cout << "Temps execution GPU: " << calcTime(start, end) << endl;
    printData(outputMat, nx, ny);
}

int main(){
    // pour mesurer le temps
    chrono::time_point<chrono::system_clock> debut, debut2, fin;

    // création des zone de stockage de données en mémoire centrale
    inputMat = new int[nx * ny];
    outputMat = new int[nx * ny];

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
        cl::Program program=createProgram("exemple.cl", context);
        // compilation du programme
        try {
            program.build(devices);
        } catch (...) {
            // Récupération des messages d'erreur au cas où...
            cl_int buildErr = CL_SUCCESS;
            auto buildInfo = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &buildErr);
            cerr << buildInfo << endl << endl;
            exit(0);
        }

        // création de la file de commandes (ordres de l'hote pour le GPU)
        cl::CommandQueue queue = cl::CommandQueue(context, devices[0]);

        loadData();
        // affichage des données initialisées
        cout << "Données initialisées" << endl;
        printData(inputMat, nx, ny);

        test_CPU();
//        test_CPU_omp();
//        test_GPU(program, queue, context);
    } catch (cl::Error &err) { // Affichage des erreurs en cas de pb OpenCL
        cout << "Exception\n";
        cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
        return EXIT_FAILURE;
    }
}
