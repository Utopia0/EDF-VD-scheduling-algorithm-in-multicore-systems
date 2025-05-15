#pragma once
#include "task.h"


// Generate a task set containing n tasks based on U-bound
TaskSet generate_n_periodic_tasks_taskset(
	int n, double p_HI, int cores, double U_bound,
	int period_lowbound, int period_upbound,
	double U_lowbound, double U_upbound, 
	double Z_lowbound, double Z_upbound);

TaskSet Back_generate_n_periodic_tasks_taskset(
	int n, double p_HI, int cores, double U_bound,
	int period_lowbound, int period_upbound,
	double U_lowbound, double U_upbound,
	double Z_lowbound, double Z_upbound);