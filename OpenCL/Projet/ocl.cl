__kernel
void test_neighbors(__const int nx, __const int ny, __global float* inputMat, __global float* outputMat) {
	int i = get_global_id(0);
	int j = get_global_id(1);

	int isCenter = 1;
	// Max float value
	float minValue = 340282346638528859811704183484516925440.0f;
	int dest = 0;


	// TOP
	if (i > 0 && j > 0 && inputMat[(i - 1) * nx + j - 1] <= minValue) {
		minValue = inputMat[(i - 1) * nx + j - 1];
		isCenter = 1;
		dest = (i - 1) * nx + j - 1;
	}
	if (i > 0 && inputMat[(i - 1) * nx + j + 0] <= minValue) {
		minValue = inputMat[(i - 1) * nx + j + 0];
		isCenter = 1;
		dest = (i - 1) * nx + j + 0;
	}
	if (i > 0 && j < (ny - 1) && inputMat[(i - 1) * nx + j + 1] <= minValue) {
		minValue = inputMat[(i - 1) * nx + j + 1];
		isCenter = 1;
		dest = (i - 1) * nx + j + 1;
	}

	// CENTER
	if (j > 0 && inputMat[(i + 0) * nx + j - 1] <= minValue) {
		minValue = inputMat[(i + 0) * nx + j - 1];
		isCenter = 1;
		dest = (i + 0) * nx + j - 1;
	}
	if (inputMat[(i + 0) * nx + j + 0] <= minValue) {
		minValue = inputMat[(i + 0) * nx + j + 0];
		isCenter = 0;
		dest = (i + 0) * nx + j + 0;
	}
	if (j < (ny - 1) && inputMat[(i + 0) * nx + j + 1] <= minValue) {
		minValue = inputMat[(i + 0) * nx + j + 1];
		isCenter = 1;
		dest = (i + 0) * nx + j + 1;
	}

	// BOTTOM
	if (i < (nx - 1) && j > 0 && inputMat[(i + 1) * nx + j - 1] <= minValue) {
		minValue = inputMat[(i + 1) * nx + j - 1];
		isCenter = 1;
		dest = (i + 1) * nx + j - 1;
	}
	if (i < (nx - 1) && inputMat[(i + 1) * nx + j + 0] <= minValue) {
		minValue = inputMat[(i + 1) * nx + j + 0];
		isCenter = 1;
		dest = (i + 1) * nx + j + 0;
	}
	if (i < (nx - 1) && j < (ny - 1) && inputMat[(i + 1) * nx + j + 1] <= minValue) {
		minValue = inputMat[(i + 1) * nx + j + 1];
		isCenter = 1;
		dest = (i + 1) * nx + j + 1;
	}

	if (isCenter == 1) {
		outputMat[(i * nx + j) * 2] = i * nx + j;
		outputMat[(i * nx + j) * 2 + 1] = dest;
	}
	else {
		outputMat[(i * nx + j) * 2] = -1;
		outputMat[(i * nx + j) * 2 + 1] = -1;
	}
}