#ifndef EDFVD_SCHDULER_H
#define EDFVD_SCHDULER_H

#include "task.h"
#include "function.h"
#include <queue>
#include <random>

struct Core {
    int coreID;
    Job currentJob;
    // std::priority_queue<Job> waitQueue;
    bool isBusy;
    int jobDeadline;

    Core() : coreID(-1), currentJob(Job()), isBusy(false), jobDeadline(0) {}
};

class MulticoreScheduler {
private:
    TaskSet& taskset;
    int sysCriticalityLevel;   // 1: LO , 2: HI
    std::priority_queue<Job> readyQueue;
    int currentTime;

    std::array<Core, NUM_CORES> coreList;
    int latestDeadlineCoreID;
    int latestDeadline;

public:
    MulticoreScheduler(TaskSet& sys);
    void edf_vd_schedule(int simulationDuration);

private:
    int add_job_to_queue(const Job& job);
    Job create_job(Task& task, int releaseTime);
    void generate_jobs();

    int assign_job_to_core(Core& core, const Job& job); // Assign the specified job to the core
    int assign_job_to_core(Core& core);    // Assign jobs from the readyQueue to the core
    int remove_job_from_core(Core& core, bool isFinished);
    int excute_jobs(Core& core);

    void overdue_job_process(const Job& job);
    void check_missed_deadlines();
    void system_mode_upgrade();     // increase system criticality level
    void system_mode_degrade();     // decrease system criticality level
    int handle_job_preemption();
    int update_deadline_info();   // Update 2 infos: latestDeadline and latestDeadlineCoreID

    void print_all_cores_status();
};


#endif