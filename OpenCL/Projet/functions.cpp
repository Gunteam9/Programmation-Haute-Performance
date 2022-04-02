#include "functions.h"


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

// Chrono 
double calcTime(chrono::time_point<chrono::system_clock> start, chrono::time_point<chrono::system_clock> end) {
	chrono::duration<double> tps = end - start;
	return tps.count();
}

// Print in console
void print_dir(const string* vec, const int xSize, const int ySize) {
	for (int i = 0; i < xSize; i++) {
		for (int j = 0; j < ySize; j++) {
			cout << vec[i * xSize + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

// Print in file with arrows
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

// Print response
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