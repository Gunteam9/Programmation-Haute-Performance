#pragma once

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Classes and structs
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
	int i, j;
};

// Utils
string direction_arrow_converter(Direction dir);
double calcTime(chrono::time_point<chrono::system_clock> start, chrono::time_point<chrono::system_clock> end);

// Printing
void print_dir(const string* vec, const int xSize, const int ySize);
void print_dir(const Direction* vec, const int xSize, const int ySize);
void print_res(const float* vec, const int xSize, const int ySize);
