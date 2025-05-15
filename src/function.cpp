#include "function.h"
#include <iostream>
#include <cmath>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

double uniform_distribution_func(double lowBound, double upBound) {
	if (lowBound > upBound) {
		std::cout << "ERROR: uniform_distribution_func failed." << std::endl;
		std::cout << "lowBound = " << lowBound << ", upBound = " << upBound << std::endl;
		exit(-1);
	}
	std::uniform_real_distribution<> dis(lowBound, upBound);
	return dis(gen);
}

int round_to_int(double x) {
	return static_cast<int>(std::round(x));
}

int positive_round_to_int(double x) {
	if (x <= 1) {
		return 1;
	}
	else {
		return round_to_int(x);
	}
}

int floor_to_int(double x) {
	return static_cast<int>(std::floor(x));
}

int positive_floor_to_int(double x) {
	if (x <= 1) {
		return 1;
	}
	else {
		return floor_to_int(x);
	}
}