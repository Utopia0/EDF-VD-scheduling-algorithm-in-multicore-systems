#ifndef EDFVD_TASK_H
#define EDFVD_TASK_H

#include <vector>
#include <array>

constexpr int NUM_CORES = 2;

struct Job {
    int taskID;
    int jobID;              // Job id (j) means it is the j-th job of the task
    // int coreID;             // The ID of the core executing the job
    int criticalityLevel;   // 1: LO , 2: HI
    int releaseTime;
    int schedulingDeadline; // Use virtual deadline (HI task in LO mode)
    int originalDeadline;   // Original deadline (for restoration)
    int executedTime;       // The time this job has been executed
    double executionTime;   // The expected execution time of this job , randomly generated
    int wcet_LO;
    bool useVirtualDeadline;    // Mark whether to use virtual deadline

    // Set task queue by modifying operator '<'
    bool operator<(const Job& other) const {
        // 1. Prioritize smaller schedulingDeadline
        if (schedulingDeadline != other.schedulingDeadline)
            return schedulingDeadline > other.schedulingDeadline;
        // 2. Prioritize seting VirtualDeadline as true
        if (useVirtualDeadline != other.useVirtualDeadline)
            return !useVirtualDeadline;
        // 3. Prioritize higher criticalityLevel
        if (criticalityLevel != other.criticalityLevel)
            return criticalityLevel < other.criticalityLevel;
        // 4. Prioritize smaller originalDeadline
        if (originalDeadline != other.originalDeadline)
            return originalDeadline > other.originalDeadline;
        // 5.Prioritize smaller releaseTime
        if (releaseTime != other.releaseTime)
            return releaseTime > other.releaseTime;
        // 6. Prioritize smaller taskID
        return taskID > other.taskID;
    };
};

class Task {
public:
    int taskID;
    int criticalityLevel;      // 1: LO , 2: HI
    bool isPeriodic;            // True: periodic task, False: sporadic task or single task
    int releaseTime;
    int period;
    int deadline;
    std::vector<int> wcet;

    int taskNumber;            // To calculate how many jobs has been created
    std::vector<double> utilization;
    double virtualDeadline;

    Task(int id, int level, bool periodic, int r, int p, int d, const std::vector<int>& wcet_values);

private:
    void ComputeUtilization();
    void PrintTaskInfo();
};

class TaskSet {
public:
    std::vector<Task>& tasklist;

    double u_lo_lo;         // The total utilization of all LO tasks in LO level 
    double u_hi_lo;         // The total utilization of all HI tasks in LO level 
    double u_hi_hi;         // The total utilization of all HI tasks in HI level 
    double u_hi_lo_max;     // The maximum utilization of HI task in LO level
    double scaleFactor;     // Calculated by EDF-VD algorithm, aim to reduce HI level tasks' deadline

    TaskSet(std::vector<Task>& list);

private:
    void ComputeUtilization();
    void ComputeVirtualDeadline();
};

#endif
