#include "task.h"
#include "schduler.h"
#include "testset.h"
#include <iostream>
// #include <cmath>

using namespace std;

int main() {
    //// Set task set
    //vector<Task> taskA = {
    //    Task(1, 1, true, 0, 10, 10, {3, 0}),
    //    Task(2, 1, true, 0, 9, 9, {4, 0}),
    //    Task(3, 2, true, 3, 20, 20, {3, 4})
    //};

    ////
    //vector<Task> taskB = {
    //    Task(1, 1, true, 0, 10, 10, {3, 0}),
    //    Task(2, 1, true, 1, 12, 12, {2, 0}),
    //    Task(3, 2, true, 0, 15, 15, {3, 6}),
    //    Task(4, 2, true, 0, 20, 20, {3, 10})
    //};

    //vector<Task> taskC = {
    //    Task(1, 1, true, 0, 9, 9, {3, 0}),
    //    Task(2, 2, true, 1, 12, 12, {4, 6}),
    //    Task(3, 2, true, 0, 15, 15, {3, 5})
    //};

    //// Very high HI-mode load
    //vector<Task> taskD = {
    //    Task(1, 1, true, 0, 20, 20, {5, 0}),
    //    Task(2, 2, true, 0, 5, 5, {2, 4}),
    //    Task(3, 2, true, 3, 10, 10, {6, 8}),
    //    Task(4, 2, true, 3, 12, 12, {3, 6})
    //};

    //vector<Task> taskE = {
    //    Task(1, 2, true, 0, 3, 3, {1, 2})

    //};

    // TaskSet taskset(taskD);
    TaskSet set = generate_n_periodic_tasks_taskset(10, 0.4, NUM_CORES, 0.75,
                                                    5, 100, 0.01, 0.75, 1.2, 2.0);

    MulticoreScheduler MulticoreScheduler(set);


    MulticoreScheduler.edf_vd_schedule(200);

    return 0;
}
