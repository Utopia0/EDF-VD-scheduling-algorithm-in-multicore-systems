#include "task.h"
#include <iostream>
#include <cmath>

using namespace std;

// Constructor function
Task::Task(int id, int level, bool periodic, int r, int p, int d, const vector<int>& wcet_values)
    : taskID(id), criticalityLevel(level), isPeriodic(periodic), releaseTime(r), period(p), deadline(d), wcet(wcet_values), taskNumber(0) {
    compute_utilization();  // Calculate utilization during initialization
    print_task_info();
}


// Calculate utilization rates at different critical levels
void Task::compute_utilization() {
    utilization.clear();    // Clear the original data
    for (int i : wcet) {
        utilization.push_back(static_cast<double>(i) / deadline);
    }
}

// Print task information
void Task::print_task_info() {
    cout << "Task " << taskID << " (Criticality Level: " << criticalityLevel << ")\n";
    cout << "  Period: " << period << ", Deadline: " << deadline << "\n";
    cout << "  WCET: ";
    for (size_t i = 0; i < wcet.size(); ++i) {
        cout << wcet[i] << " ";
    }
    cout << "\n  Utilization: ";
    for (size_t i = 0; i < utilization.size(); ++i) {
        cout << utilization[i] << " ";
    }
    cout << "\n";
}

// Constructor function
TaskSet::TaskSet(const vector<Task>& list): tasklist(list) {
    std::cout << "tasklist size: " << tasklist.size() << std::endl;
    compute_utilization();
    compute_virtual_deadline();
}

void TaskSet::compute_utilization() {
    u_lo_lo = 0;
    u_hi_lo = 0;
    u_hi_hi = 0;
    u_hi_lo_max = 0;

    for (const auto& task : tasklist) {
        if (task.criticalityLevel == 1) {
            u_lo_lo = u_lo_lo + task.utilization[0];
        }
        else if (task.criticalityLevel == 2)
        {
            u_hi_lo = u_hi_lo + task.utilization[0];
            u_hi_hi = u_hi_hi + task.utilization[1];
            u_hi_lo_max = (u_hi_lo_max > task.utilization[0]) ? u_hi_lo_max : task.utilization[0];
        }
    }

    cout << "u_lo_lo = " << u_lo_lo
        << "\nu_hi_lo = " << u_hi_lo
        << "\nu_lo = " << u_lo_lo + u_hi_lo
        << "\nu_hi_hi = " << u_hi_hi << endl;

    scaleFactor = max(u_hi_lo / ((NUM_CORES + 1.0) / 2.0 - u_lo_lo), u_hi_lo_max);

    cout << "\nscaleFactor = " << scaleFactor << endl;
    if (scaleFactor > 1) {
        scaleFactor = 1;
        cerr << "\nWarning: scaleFactor > 1. Set scaleFactor to 1." << endl;
    }

    // Schedulability analysis
    double schedulabilityFactorUpLimit = (NUM_CORES + 1.0) / 2.0;
    double schedulabilityFactor = u_lo_lo + min(u_hi_hi, u_hi_lo / (1.0 - u_hi_hi * 2.0 / (NUM_CORES + 1.0)));
    cout << "\nSchedulability Factor = " << schedulabilityFactor << endl;
    if (schedulabilityFactor <= schedulabilityFactorUpLimit) {
        cout << "\nThis task set is schedulable \n";
    }
    else {
        cout << "\nThis task set may not be schedulable \n";
        // exit(1);
    }
}

// compute virtual deadline according to scaleFactor
void TaskSet::compute_virtual_deadline() {
    for (auto& task : tasklist) {
        if (task.criticalityLevel == 2) {
            task.virtualDeadline = scaleFactor * task.deadline;
        }
    }
}