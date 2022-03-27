__kernel
void test_neighbors(__const int nx, __const int ny, __global float* inputMat, __global float* outputMat) {
	int i = get_global_id(0);
	int j = get_global_id(1);


	//int isCenter = 1;
	//float minValue = 340282346638528859811704183484516925440.0f;
	//int dest = 0;

	//// TOP
	//if (i > 0 && j > 0 && inputMat[(i - 1) * nx + j - 1] <= minValue) {
	//	minValue = inputMat[(i - 1) * nx + j - 1];
	//	dest = (i - 1) * nx + j - 1;
	//}
	//if (i > 0 && inputMat[(i - 1) * nx + j + 0] <= minValue) {
	//	minValue = inputMat[(i - 1) * nx + j + 0];
	//	dest = (i - 1) * nx + j + 0;
	//}
	//if (i > 0 && j < (ny - 1) && inputMat[(i - 1) * nx + j + 1] <= minValue) {
	//	minValue = inputMat[(i - 1) * nx + j + 1];
	//	dest = (i - 1) * nx + j + 1;
	//}

	//// CENTER
	//if (j > 0 && inputMat[(i + 0) * nx + j - 1] <= minValue) {
	//	minValue = inputMat[(i + 0) * nx + j - 1];
	//	dest = (i + 0) * nx + j - 1;
	//}
	//if (inputMat[(i + 0) * nx + j + 0] <= minValue) {
	//	minValue = inputMat[(i + 0) * nx + j + 0];
	//	isCenter = 0;
	//	dest = (i + 0) * nx + j + 0;
	//}
	//if (j < (ny - 1) && inputMat[(i + 0) * nx + j + 1] <= minValue) {
	//	minValue = inputMat[(i + 0) * nx + j + 1];
	//	dest = (i + 0) * nx + j + 1;
	//}

	//// BOTTOM
	//if (i < (nx - 1) && j > 0 && inputMat[(i + 1) * nx + j - 1] <= minValue) {
	//	minValue = inputMat[(i + 1) * nx + j - 1];
	//	dest = (i + 1) * nx + j - 1;
	//}
	//if (i < (nx - 1) && inputMat[(i + 1) * nx + j + 0] <= minValue) {
	//	minValue = inputMat[(i + 1) * nx + j + 0];
	//	dest = (i + 1) * nx + j + 0;
	//}
	//if (i < (nx - 1) && j < (ny - 1) && inputMat[(i + 1) * nx + j + 1] <= minValue) {
	//	minValue = inputMat[(i + 1) * nx + j + 1];
	//	dest = (i + 1) * nx + j + 1;
	//}

	//if (isCenter == 1)
	//	outputMat[i * nx + j] = dest;

}


//__kernel 
//void stencil(__const int taille, __const float lambda, __global int* A, __global int* B) {
//    int i = get_global_id(0);
//    int j = get_global_id(1);
//
//    float center = (1 - 4 * lambda) * A[i * taille + j];
//    float top = i - 1 < 0 ? A[(taille - 1) * taille + j] : A[(i-1) * taille + j];
//    float bottom = i + 1 >= taille ? A[0 * taille + j] : A[(i+1) * taille + j];
//    float left = j - 1 < 0 ? A[i * taille + taille - 1] : A[i * taille + j - 1];
//    float right = j + 1 >= taille ? A[i * taille + 0] : A[i * taille + j + 1];
//    B[i * taille + j] = center + lambda * (top + bottom + left + right);
//}

//__kernel
//void stencil_line(__const int taille, __const float lambda, __global int* A, __global int* B, __local int* ligne){
//    int i = get_global_id(0);
//    int j = get_global_id(1);
//    int id = get_local_id(0);
//    int id2 = get_local_id(1);
//
//    if ((id == 0) && (id2 == 0)) {
//        for (int k = 0; k < taille; ++k) {
//            ligne[0 * taille + k] = i - 1 < 0 ? A[(taille - 1) * taille + k] : A[(i - 1) * taille + k];
//            ligne[1 * taille + k] = A[i * taille + k];
//            ligne[2 * taille + k] = i + 1 >= taille ? A[k] : A[(i + 1) * taille + k];
//        }
//    }
//
//    barrier(CLK_LOCAL_MEM_FENCE);
//
//    float center = (1 - 4 * lambda) * ligne[1 * taille + j];
//    float top = ligne[j];
//    float bottom = ligne[2 * taille + j];
//    float left = j - 1 < 0 ? ligne[1 * taille + taille - 1] : ligne[1 * taille + j - 1] ;
//    float right = j + 1 >= taille ? ligne[1 * taille + 0]  : ligne[1 * taille + j + 1] ;
//    B[i * taille + j] = center + lambda * (top + bottom + left + right);
//}