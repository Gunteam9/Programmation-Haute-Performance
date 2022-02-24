#define __CL_ENABLE_EXCEPTIONS

#include "CL/cl.hpp"

#include <vector>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <math.h>
// pour la génération aléatoire
#include <cstdlib>     
#include <ctime>  


// variables globales
// taille des données (soit le vecteur ou le coté d'une matrice carrée)
//const int taille=2048*2048;
const int taille=2048;
// taille des données en octets Attention au type des données)
size_t nboctets = sizeof(int)*taille*taille;
// pointeurs vers le stockage des données en mémoire centrale
int* A;
int* B;
int* C;


// fonction permettant de récupérer le temps écoulé entre debut et fin
double temps(std::chrono::time_point<std::chrono::system_clock> debut, std::chrono::time_point<std::chrono::system_clock> fin){
  std::chrono::duration<double> tps=fin-debut;
  return tps.count();
}

// initialisation d'un vecteur à une valeur aléatoire avec un max
void init_vec(int* vec, int taille, int max){
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; ++j) {
            vec[i * taille + j] = rand() % max;
        }
    }
}

// initialisation d'un vecteur à une valeur aléatoire entre min et max 
void init_vec(float* vec,int taille, float max) {
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            vec[i * taille + j] = static_cast<float>(rand()) / static_cast<float>(max);
        }
    }
}

void affiche_vec(int* vec, int taille){
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            std::cout << vec[i * taille + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void affiche_vec(float* vec, int taille){
    for (int i = 0; i < taille; i++) {
        for (int j = 0; j < taille; j++) {
            std::cout << vec[i * taille + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


cl::Program creationProgramme(std::string nomFicSource, cl::Context contexte){
  // lecture du programme source
    std::ifstream sourceFile(nomFicSource);
    std::string sourceCode(std::istreambuf_iterator <char>(sourceFile),(std::istreambuf_iterator < char >()));
    // la premier argument indique le nombre de programmes sources utilisés, le deuxième est une paire (texte, taille du programme)
    cl::Program::Sources source(1,std::make_pair(sourceCode.c_str(),sourceCode.length()+1));
    // creation du programme dans le contexte
    return cl::Program(contexte,source);
}


void test_CPU(){
    std::chrono::time_point<std::chrono::system_clock> debut=std::chrono::system_clock::now();
    for(int i = 0; i < taille; i++) {
        for(int j = 0; j < taille; j++) {
            for(int k = 0; k < taille; k++) {
                C[i * taille + j] += A[i * taille + k] * B[k * taille + j];
            }
        }
    }
    std::chrono::time_point<std::chrono::system_clock> fin=std::chrono::system_clock::now();
    std::cout<<"Résultat CPU"<<std::endl;
    std::cout<<"Temps execution CPU: "<<temps(debut,fin)<<std::endl;
//    affiche_vec(C, taille);
}

void test_CPU_omp(){
    std::chrono::time_point<std::chrono::system_clock> debut=std::chrono::system_clock::now();
#pragma omp parallel for
    for(int i = 0; i < taille; i++) {
#pragma omp parallel for
        for(int j = 0; j < taille; j++) {
#pragma omp parallel for
            for(int k = 0; k < taille; k++) {
                C[i * taille + j] += A[i * taille + k] * B[k * taille + j];
            }
        }
    }
    std::chrono::time_point<std::chrono::system_clock> fin=std::chrono::system_clock::now();
    std::cout<<"Résultat CPU OpenMP"<<std::endl;
    std::cout<<"Temps execution CPU OpenMP: "<<temps(debut,fin)<<std::endl;
//    affiche_vec(C, taille);
}

void test_GPU(cl::Program programme, cl::CommandQueue queue, cl::Context contexte){
    std::chrono::time_point<std::chrono::system_clock> debut=std::chrono::system_clock::now();
    // Création des buffers de données dans le contexte
    cl::Buffer bufferA = cl::Buffer(contexte, CL_MEM_READ_ONLY, nboctets);
    cl::Buffer bufferB = cl::Buffer(contexte, CL_MEM_READ_ONLY, nboctets);
    cl::Buffer bufferC = cl::Buffer(contexte, CL_MEM_WRITE_ONLY, nboctets);

    // Chargement des données en mémoire video
    queue.enqueueWriteBuffer(bufferA , CL_TRUE, 0, nboctets , A);
    queue.enqueueWriteBuffer(bufferB , CL_TRUE, 0, nboctets , B);
    // creation du kernel (fonction à exécuter)
    cl::Kernel kernel(programme,"matrice_matrice");
    // Attribution des paramètres de ce kernel
    kernel.setArg(0,taille);
    kernel.setArg(1,bufferA);
    kernel.setArg(2,bufferB);
    kernel.setArg(3,bufferC);

    // création de la topologie des processeurs
    cl::NDRange global(taille, taille); // nombre total d'éléments de calcul -processing elements
    cl::NDRange local(16, 16); // dimension des unités de calcul -compute units- c'à-dire le nombre d'éléments de calcul par unités de calcul

    // lancement du programme en GPU
    queue.enqueueNDRangeKernel(kernel,cl::NullRange,global,local);

    // recupération du résultat
    queue.enqueueReadBuffer(bufferC,CL_TRUE,0,nboctets,C);
    std::chrono::time_point<std::chrono::system_clock> fin=std::chrono::system_clock::now();

    std::cout<<"Résultat GPU"<<std::endl;
    std::cout<<"Temps execution GPU: "<<temps(debut,fin)<<std::endl;
//    affiche_vec(C, taille);
}

void test_GPU_line(cl::Program programme, cl::CommandQueue queue, cl::Context contexte){
    std::chrono::time_point<std::chrono::system_clock> debut=std::chrono::system_clock::now();
    // Création des buffers de données dans le contexte
    cl::Buffer bufferA = cl::Buffer(contexte, CL_MEM_READ_ONLY, nboctets);
    cl::Buffer bufferB = cl::Buffer(contexte, CL_MEM_READ_ONLY, nboctets);
    cl::Buffer bufferC = cl::Buffer(contexte, CL_MEM_WRITE_ONLY, nboctets);

    // Chargement des données en mémoire video
    queue.enqueueWriteBuffer(bufferA , CL_TRUE, 0, nboctets , A);
    queue.enqueueWriteBuffer(bufferB , CL_TRUE, 0, nboctets , B);
    // creation du kernel (fonction à exécuter)
    cl::Kernel kernel(programme,"matrice_matrice_line");
    // Attribution des paramètres de ce kernel
    kernel.setArg(0,taille);
    kernel.setArg(1,bufferA);
    kernel.setArg(2,bufferB);
    kernel.setArg(3,bufferC);

    // création de la topologie des processeurs
    cl::NDRange global(taille); // nombre total d'éléments de calcul -processing elements
    cl::NDRange local(16); // dimension des unités de calcul -compute units- c'à-dire le nombre d'éléments de calcul par unités de calcul

    // lancement du programme en GPU
    queue.enqueueNDRangeKernel(kernel,cl::NullRange,global,local);

    // recupération du résultat
    queue.enqueueReadBuffer(bufferC,CL_TRUE,0,nboctets,C);
    std::chrono::time_point<std::chrono::system_clock> fin=std::chrono::system_clock::now();

    std::cout<<"Résultat GPU line"<<std::endl;
    std::cout<<"Temps execution GPU line: "<<temps(debut,fin)<<std::endl;
//    affiche_vec(C, taille);
}


int main(){
    // pour mesurer le temps
    std::chrono::time_point<std::chrono::system_clock> debut,debut2,fin;
    // initialisation de générateur aléatoire
    srand (time(NULL));

    // création des zone de stockage de données en mémoire centrale
    A = new int[taille * taille];
    B = new int[taille * taille];
    C = new int[taille * taille];

    try { // debut de la zone d'utilisation de l'API pour OpenCL
        // les plateformes
        std::vector <cl::Platform> plateformes;
        cl::Platform::get(&plateformes); // recherche des plateformes normalement 1 sur un PC

        //les devices
        std::vector <cl::Device> devices;
        plateformes[0].getDevices(CL_DEVICE_TYPE_ALL,&devices); // recherche des devices (normalement 1)

        // création d'un contexte pour les devices
        cl::Context contexte(devices);

        // création du programme dans le contexte (voir code fonction)
        cl::Program programme=creationProgramme("exemple.cl",contexte);
        // compilation du programme
        try {
            programme.build(devices);
        } catch (...) {
            // Récupération des messages d'erreur au cas où...
            cl_int buildErr = CL_SUCCESS;
            auto buildInfo = programme.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0],&buildErr);
            std::cerr << buildInfo << std::endl << std::endl;
            exit(0);
        }

        // création de la file de commandes (ordres de l'hote pour le GPU)
        cl::CommandQueue queue= cl::CommandQueue(contexte,devices[0]);

        // initialisation des données sur l'hote
        init_vec(A,taille,10);
        init_vec(B,taille,10);
        // affichage des données initialisées
        std::cout<<" Données initialisées"<<std::endl;
//        affiche_vec(A,taille);
//        affiche_vec(B,taille);

        test_CPU();
        test_CPU_omp();
        test_GPU(programme,queue,contexte);
        test_GPU_line(programme,queue,contexte);
    } catch (cl::Error err) { // Affichage des erreurs en cas de pb OpenCL
        std::cout << "Exception\n";
        std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
        return EXIT_FAILURE;
    }
}
