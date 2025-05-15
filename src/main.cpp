#include "task.h"
#include "schduler.h"
// #include <iostream>
// #include <cmath>

using namespace std;

int main() {
    // Set task set
    vector<Task> taskA = {
        Task(1, 1, true, 0, 10, 10, {3, 0}),
        Task(2, 1, true, 0, 9, 9, {4, 0}),
        Task(3, 2, true, 3, 20, 20, {3, 4})
    };

    //
    vector<Task> taskB = {
        Task(1, 1, true, 0, 10, 10, {3, 0}),
        Task(2, 1, true, 1, 12, 12, {2, 0}),
        Task(3, 2, true, 0, 15, 15, {3, 6}),
        Task(4, 2, true, 0, 20, 20, {3, 10})
    };

    vector<Task> taskC = {
        Task(1, 1, true, 0, 9, 9, {3, 0}),
        Task(2, 2, true, 1, 12, 12, {4, 6}),
        Task(3, 2, true, 0, 15, 15, {3, 5})
    };


    vector<Task> taskD = {
        Task(1, 1, true, 0, 20, 20, {5, 0}),
        Task(2, 1, true, 0, 60, 60, {15, 0}),
        Task(3, 1, true, 3, 15, 15, {3, 0})
        //Task(4, 1, true, 3, 12, 12, {2, 0})
    };

    TaskSet taskset(taskA);
    MulticoreScheduler MulticoreScheduler(taskset);


    MulticoreScheduler.EDF_VD_Schedule(50);

    return 0;
}
