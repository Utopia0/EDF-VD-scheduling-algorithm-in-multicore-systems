#ifndef EDFVD_SCHDULER_H
#define EDFVD_SCHDULER_H

#include "task.h"
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
    std::random_device rd;
    std::mt19937 gen;

    std::array<Core, NUM_CORES> coreList;
    int latestDeadlineCoreID;
    int latestDeadline;


public:
    MulticoreScheduler(TaskSet& sys);
    void EDF_VD_Schedule(int simulationDuration);

private:
    int AddJobToQueue(const Job& job);
    Job CreateJob(Task& task, int releaseTime);
    void GenerateJobs();

    int AssignJobToCore(Core& core, const Job& job); // Assign the specified job to the core
    int AssignJobToCore(Core& core);    // Assign jobs from the readyQueue to the core
    int RemoveJobFromCore(Core& core, bool isFinished);
    int ExcuteJobs(Core& core);

    void CheckMissedDeadlines();
    void HandleModeSwitch();
    int HandleJobPreemption();
    int UpdateDeadlineInfo();   // Update 2 infos: latestDeadline and latestDeadlineCoreID

    void PrintAllCoresStatus();
};


#endif