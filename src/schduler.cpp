#include "task.h"
#include "schduler.h"
#include "globals.h"
#include <iostream>
#include <cmath>

using namespace std;

// Constructor function
MulticoreScheduler::MulticoreScheduler(TaskSet& sys)
    : taskset(sys), sysCriticalityLevel(1), currentTime(0), latestDeadlineCoreID(NUM_CORES - 1), latestDeadline(0){
    int id = 0;
    for (auto& core : coreList){
        core.coreID = id;
        id++;
    }
}


// Main scheduling function
void MulticoreScheduler::edf_vd_schedule(int simulationDuration) {
    cout << "\n***********************************************" << endl;
    cout << "EDF-VD Multicore Scheduling Staring" << endl;
    cout << "***********************************************\n" << endl;

    while (currentTime <= simulationDuration)
    {
        cout << "Time " << currentTime << ":" << endl;
        generate_jobs();
        bool isSystemIdle = readyQueue.empty();
        for (auto& core : coreList){
            excute_jobs(core);
            isSystemIdle = isSystemIdle && !core.isBusy;
        }
        if (isSystemIdle && sysCriticalityLevel == 2) {
            system_mode_degrade();
        }
        handle_job_preemption();
        print_all_cores_status();
        currentTime++;
    }
}


// Add job to readyQueue
int MulticoreScheduler::add_job_to_queue(const Job& job) {
    readyQueue.push(job);
    return StatusCode::OK;
}


// Create job object
Job MulticoreScheduler::create_job(Task& task, int releaseTime) {
    // A new job is created, so taskNumber + 1
    task.taskNumber = task.taskNumber + 1;

    bool useVirtualDeadline = (task.criticalityLevel == 2 && sysCriticalityLevel == 1);
    int schedulingDeadline;
    double executionTime;

    // Set schedulingDeadline (due to task level) and executionTime (randomly)
    if (useVirtualDeadline) {
        // HI level tasks in LO mode
        schedulingDeadline = static_cast<int>(ceil(task.virtualDeadline));    // Round up, but prioritize scheduling during scheduling

        double lowerBound = min(0.75 * task.wcet[0], 0.75 * task.wcet[1]);
        double upperBound = task.wcet[1];
        executionTime = uniform_distribution_func(lowerBound, upperBound);
    }
    else {
        // LO level tasks , or HI level tasks in HI taskset
        schedulingDeadline = task.deadline;                 // For LO task, do not modify the deadline

        double lowerBound = 0.75 * task.wcet[task.criticalityLevel - 1];
        double upperBound = task.wcet[task.criticalityLevel - 1];
        executionTime = uniform_distribution_func(lowerBound, upperBound);
    }

    return {
        task.taskID,
        task.taskNumber,
        task.criticalityLevel,
        releaseTime,
        releaseTime + schedulingDeadline,  // scheduling deadline
        releaseTime + task.deadline,       // original absolute deadline
        0,  // executedTime
        executionTime,
        task.wcet[0],
        useVirtualDeadline
    };
    /*
    The definition of Job:
    int taskID;
    int jobID;                 // Job id (j) means it is the j-th job of the task
    int criticalityLevel;      // 1: LO , 2: HI
    int releaseTime;
    int schedulingDeadline;    // Use virtual deadline (HI task in LO mode)
    int originalDeadline;      // Original deadline (for restoration)
    int executedTime;          // The time this job has been executed
    double executionTime;      // The expected execution time of this job , randomly generated
    int wcet_LO;
    bool useVirtualDeadline;    // Mark whether to use virtual deadline
    */
}

// Generate Jobs
void MulticoreScheduler::generate_jobs() {
    for (auto& task : taskset.tasklist) {
        // Discard LO task in HI mode
        if (sysCriticalityLevel == 2 && task.criticalityLevel == 1) continue;

        if (task.isPeriodic && (currentTime - task.releaseTime) % task.period == 0
            || !task.isPeriodic && currentTime == task.releaseTime) {
            Job newJob = create_job(task, currentTime);
            add_job_to_queue(newJob);
            // readyQueue.push(newJob);

            // Print info
            if (newJob.useVirtualDeadline) {
                cout << "New Job [Task" << newJob.taskID << " Job" << newJob.jobID << "]"
                    << " released (Virtual Deadline @ " << newJob.schedulingDeadline << ")\n";
            }
            else {
                cout << "New Job [Task" << newJob.taskID << " Job" << newJob.jobID << "]"
                    << " released (Deadline @ " << newJob.schedulingDeadline << ")\n";
            }
        }
    }
}


int MulticoreScheduler::assign_job_to_core(Core& core, const Job& job) {
    if (core.isBusy) {
        cerr << "[ERROR] Core " << core.coreID << "is busy. assign_job_to_core Failed" << endl;
        return StatusCode::ERROR;
    }
    else{
        // Update info of job
        core.currentJob = job;
        core.isBusy = true;
        core.jobDeadline = job.schedulingDeadline;
        return StatusCode::OK;
    }
    return StatusCode::ERROR;
}

int MulticoreScheduler::assign_job_to_core(Core& core) {
    if (readyQueue.empty()){
        cout << "ERROR: readyQueue is empty." << endl;
        return StatusCode::ERROR;
    }
    else{
        assign_job_to_core(core, readyQueue.top());
        readyQueue.pop();
    }
    return StatusCode::OK;
}


int MulticoreScheduler::remove_job_from_core(Core& core, bool isFinished) {
    if (!core.isBusy) {
        cerr << "[ERROR] Core " << core.coreID << "is idle. remove_job_from_core Failed" << endl;
        return StatusCode::ERROR;
    }
    else{
        // If the job isn't finished, push it back to readyQueue;
        if (!isFinished) {
            add_job_to_queue(core.currentJob);
            //cout << "///////////////////////////////////////////////////////////////////////" << endl;
            //cout << core.currentJob.taskID << "  core.currentJob  " << core.currentJob.executedTime << endl;
            //cout << readyQueue.top().taskID << "  readyQueue.top()  " << readyQueue.top().executedTime << endl;
        }
        // The job is finished. Remove this job
        core.currentJob = Job{};
        core.isBusy = false;
        core.jobDeadline = 0;
        return StatusCode::OK;
    }
    return StatusCode::ERROR;
}


int MulticoreScheduler::excute_jobs(Core& core) {
    if (core.isBusy) {
        core.currentJob.executedTime++;
        // If job completes
        if (core.currentJob.executedTime >= core.currentJob.executionTime) {
            cout << "Core " << core.coreID << " [Task " << core.currentJob.taskID << " Job" << core.currentJob.jobID
                << "] completed, executionTime is " << core.currentJob.executionTime << endl;
            remove_job_from_core(core, true);
        }
        else if (currentTime >= core.currentJob.schedulingDeadline && !core.currentJob.useVirtualDeadline) {
            // This job misses the deadline
            overdue_job_process(core.currentJob);
            remove_job_from_core(core, true);
        }
        // Mode upgrade
        else if (core.currentJob.executedTime >= core.currentJob.wcet_LO && 
                core.currentJob.criticalityLevel == 2 && sysCriticalityLevel == 1) {
            system_mode_upgrade();
        }
        check_missed_deadlines();
    }
    // Choose a new job
    if (!core.isBusy && !readyQueue.empty()) {
        assign_job_to_core(core);
        cout << "Core " << core.coreID << ": Start executing [Task";
        if (!core.currentJob.useVirtualDeadline) {
            cout << core.currentJob.taskID << " Job" << core.currentJob.jobID << "] (Deadline @ "
                << core.currentJob.schedulingDeadline << ")\n";
        }
        else {
            cout << core.currentJob.taskID << " Job" << core.currentJob.jobID << "] (Vitural Deadline @ "
                << core.currentJob.schedulingDeadline << ")\n";
        }
    }

    update_deadline_info();

    return StatusCode::OK;
}

// If a job has missed the deadline, print warning
void MulticoreScheduler::overdue_job_process(const Job& job) {
    cerr << "WARNING: [Task" << job.taskID << " Job" << job.jobID << "] missed deadline at "
        << job.schedulingDeadline << endl;
    if (job.criticalityLevel == 2) {
        cerr << "\nERROR: HI level job missed deadline\n" << endl;
        cin.get();
    }
}


// Check if any jobs in the readyQueue have missed the deadline
void MulticoreScheduler::check_missed_deadlines() {
    while (!readyQueue.empty()) {
        const Job& job = readyQueue.top();
        if (currentTime > job.schedulingDeadline && !job.useVirtualDeadline) {
            overdue_job_process(job);
            readyQueue.pop();
        }
        else return;
    }
}

// Switch the processing mode to HI criticality level
void MulticoreScheduler::system_mode_upgrade() {
    cout << "\n=== Switching to HI-Criticality Mode at Time " << currentTime << " ===\n";
    sysCriticalityLevel = 2;

    // Update the deadline for HI jobs in the queue
    priority_queue<Job> newQueue;
    while (!readyQueue.empty()) {
        Job job = readyQueue.top();
        readyQueue.pop();

        if (job.criticalityLevel == 2) {
            // Restore original deadline
            job.schedulingDeadline = job.originalDeadline;
            job.useVirtualDeadline = false;
        }
        else {
            // Discard LO critical level tasks
            continue;
        }
        newQueue.push(job);
    }
    readyQueue = newQueue;
}



// Switch the processing mode to LO criticality level
void MulticoreScheduler::system_mode_degrade() {
    cout << "\n=== Switching to LO-Criticality Mode at Time " << currentTime << " ===\n";
    sysCriticalityLevel = 1;
}



// Update 2 infos: latestDeadline and latestDeadlineCoreID
int MulticoreScheduler::update_deadline_info() {
    int id = 0, latestDDL = 0;
         
    for (auto& core : coreList){
        if (core.isBusy && latestDDL < core.jobDeadline) {
            latestDDL = core.jobDeadline;
            id = core.coreID;
        }
    }
    latestDeadline = latestDDL;
    latestDeadlineCoreID = id;

    return StatusCode::OK;
}

// Dealing with the task preemption, swapping the current job with the queue header job
int MulticoreScheduler::handle_job_preemption() {
    int preemptionTimes = 0;
    while (!readyQueue.empty() && readyQueue.top().schedulingDeadline < latestDeadline ) {
        if (preemptionTimes > NUM_CORES) {
            cout << "Preemption Times ERROR" << endl;
            return StatusCode::ERROR;
        }
        else{
            remove_job_from_core(coreList[latestDeadlineCoreID], false);
            assign_job_to_core(coreList[latestDeadlineCoreID], readyQueue.top());
            readyQueue.pop();
            cout << readyQueue.top().taskID << "  readyQueue.top()  " << readyQueue.top().executedTime << endl;
            update_deadline_info();
            preemptionTimes++;

            // output and print  
            /*if (!readyQueue.empty() && core.isBusy &&
                core.jobDeadline > readyQueue.top().schedulingDeadline) {
                cout << "[Task " << core.currentJob.taskID << " Job" << core.currentJob.jobID
                    << "] is suspended, executed time is " << core.currentJob.executedTime << endl;

                cout << "Start executing [Task";
                if (!core.currentJob.useVirtualDeadline) {
                    cout << core.currentJob.taskID << " Job" << core.currentJob.jobID << "] (Deadline @ "
                        << core.currentJob.schedulingDeadline << ")\n";
                }
                else {
                    cout << core.currentJob.taskID << " Job" << core.currentJob.jobID << "] (Vitural Deadline @ "
                        << core.currentJob.schedulingDeadline << ")\n";
                }
            }*/
        }
    }
    return StatusCode::OK;
}

// Print the jobs executed by each core
void MulticoreScheduler::print_all_cores_status() {
    // cout << "Time " << currentTime << ":" << endl;
    cout << "latestDDL: " << latestDeadline << ", coreID: " << latestDeadlineCoreID << endl;
    for (auto& core: coreList){
        if (core.isBusy){
            cout << "Core " << core.coreID << " | [Task " << core.currentJob.taskID << " Job " << core.currentJob.jobID
                << "] Deadline @" << core.currentJob.schedulingDeadline << " | Executed time: " << core.currentJob.executedTime << endl;
        }
        else{
            cout << "Core " << core.coreID << " | Idle" << endl;
        }
    }
    cout << endl;
}