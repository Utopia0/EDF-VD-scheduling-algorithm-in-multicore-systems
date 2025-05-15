#include "testset.h"
#include "function.h"
#include <iostream>
#include <vector>
#include <algorithm>


TaskSet generate_n_periodic_tasks_taskset(
	int n, double p_HI, int cores, double U_bound,
	int period_lowbound, int period_upbound,
	double U_lowbound, double U_upbound, 
	double Z_lowbound, double Z_upbound)
{
	double U_total = U_bound * cores;
	int taskid = 1;
	int taskLevel;
	int period;			// All jobs arrive at t=0, with implicit deadline
	int WCET_LO;
	int WCET_HI;
	double z;			// For a task, the range of its (u_HI / u_LO)
	double u_HI;
	double u_LO;
	double u_sum_LO = 0;
	double u_sum_HI = 0;
	double u_average;
	std::vector<Task> taskVector = {};
	
	while (taskid <= n) {
		period = round_to_int(uniform_distribution_func(period_lowbound, period_upbound));
		u_average = (U_total - u_sum_LO) / (n - taskid + 1);
		u_LO = uniform_distribution_func(
			std::max(0.01 * u_average, U_lowbound),
			std::min({ 1.99 * u_average, U_upbound, (U_total - u_sum_LO)})
			);

		if (uniform_distribution_func(0, 1) < p_HI) {
			// The probability of generating HI tasks is p_HI
			taskLevel = 2;
			z = uniform_distribution_func(Z_lowbound, Z_upbound);
			u_HI = u_LO * z;
			WCET_HI = positive_round_to_int(period * u_HI);
		}
		else {
			taskLevel = 1;
			WCET_HI = 0;
		}
		WCET_LO = positive_round_to_int(period * u_LO);
		u_sum_LO += (1.0 * WCET_LO / period);
		u_sum_HI += (1.0 * WCET_HI / period);

		std::cout << "u_sum_LO = " << u_sum_LO << std::endl;
		std::cout << "u_sum_HI = " << u_sum_HI << std::endl;

		if (u_sum_LO > U_total || u_sum_HI > U_total) {
			std::cout << "generate_n_periodic_tasks_taskset failed." << std::endl;
			break;
		}

		taskVector.emplace_back(Task(taskid, taskLevel, true, 0, period, period, { WCET_LO, WCET_HI }));
		taskid++;
	}

	// Adjust taskset
	double u_LO_min = 1.0;
	int u_min_id = 0;
	for (int i = 0; i < taskVector.size(); i++) {
		if (taskVector[i].utilization[0] < u_LO_min) {
			u_min_id = i;
		}
	}
	std::cout << "u_min_id = " << u_min_id+1 << std::endl;

	Task& target = taskVector[u_min_id];
	if (target.criticalityLevel == 1) {
		target.utilization[0] = std::min(U_upbound, target.utilization[0] + U_total - u_sum_LO);
		target.wcet[0] = positive_floor_to_int(target.period * target.utilization[0]);
	}
	else {
		z = target.utilization[1] / target.utilization[0];
		if (U_total - u_sum_LO < (U_total - u_sum_HI)/z ) {// Remaining LO utilization is less than HI utilization
			target.utilization[0] = std::min(U_upbound, target.utilization[0] + U_total - u_sum_LO);
			target.utilization[1] = std::min(U_upbound, target.utilization[0] * z);
		}
		else {// Remaining LO utilization is more than HI utilization
			target.utilization[1] = std::min(U_upbound, target.utilization[1] + U_total - u_sum_HI);
			target.utilization[0] = target.utilization[1] / z;
		}
		target.wcet[0] = positive_floor_to_int(target.period * target.utilization[0]);
		target.wcet[1] = positive_floor_to_int(target.period * target.utilization[1]);
	}
	target.compute_utilization();
	target.print_task_info();

	TaskSet set(taskVector);
	return set;
}

TaskSet Back_generate_n_periodic_tasks_taskset(
	int n, double p_HI, int cores, double U_bound,
	int period_lowbound, int period_upbound,
	double U_lowbound, double U_upbound,
	double Z_lowbound, double Z_upbound)
{
	// All jobs arrive at t=0, with implicit deadline
	int taskid = 1;
	int taskLevel;
	int period;
	int WCET_LO;
	int WCET_HI;
	double z;			// For a task, the range of its (u_HI / u_LO)
	double u_HI;
	double u_LO;
	double u_sum_LO = 0;
	double u_sum_HI = 0;
	std::vector<Task> taskVector = {};

	while (taskid <= n) {
		period = round_to_int(uniform_distribution_func(period_lowbound, period_upbound));
		if (uniform_distribution_func(0, 1) < p_HI) {
			// The probability of generating HI tasks is p_HI
			taskLevel = 2;
			z = uniform_distribution_func(Z_lowbound, Z_upbound);
			u_HI = uniform_distribution_func(U_lowbound, U_upbound);
			u_LO = u_HI / z;
			WCET_HI = positive_round_to_int(period * u_HI);
		}
		else {
			taskLevel = 1;
			double u_average = (U_bound * cores - u_sum_LO) / cores;
			u_LO = uniform_distribution_func(0.2 * u_average, std::min(1.8 * u_average, (U_bound * cores - u_sum_LO)));
			u_LO = std::min(U_upbound, u_LO);
			WCET_HI = 0;
		}
		WCET_LO = positive_round_to_int(period * u_LO);
		u_sum_LO += (1.0 * WCET_LO / period);
		u_sum_HI += (1.0 * WCET_HI / period);

		std::cout << "u_sum_LO = " << u_sum_LO << std::endl;
		std::cout << "u_sum_HI = " << u_sum_HI << std::endl;


		if (u_sum_LO > U_bound * cores || u_sum_HI > U_bound * cores) {
			std::cout << "generate_n_periodic_tasks_taskset failed." << std::endl;
			break;
		}
		taskVector.emplace_back(Task(taskid, taskLevel, true, 0, period, period, { WCET_LO, WCET_HI }));
		taskid++;
	}

	TaskSet set(taskVector);
	std::cout << "generate_n_periodic_tasks_taskset success" << std::endl;
	return set;
}