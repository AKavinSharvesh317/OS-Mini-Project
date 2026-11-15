#include "campus_compute.h"

static void reset_jobs(const Job src[], Job dest[], int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
        dest[i].remaining_burst = src[i].burst_time;
        dest[i].start_time = -1;
        dest[i].completion_time = 0;
        dest[i].turnaround_time = 0;
        dest[i].waiting_time = 0;
        dest[i].state = JOB_READY;
    }
}

static void print_metrics(Job jobs[], int n, const char *policy_name) {
    double total_tat = 0.0, total_wt = 0.0;
    printf("\n--- %s Execution Metrics ---\n", policy_name);
    printf("ID | Arr | Burst | Prio | Compl | Turnaround | Waiting\n");
    printf("---+-----+-------+------+-------+------------+--------\n");
    for (int i = 0; i < n; i++) {
        total_tat += jobs[i].turnaround_time;
        total_wt += jobs[i].waiting_time;
        printf("%2d | %3d | %5d | %4d | %5d | %10d | %7d\n",
               jobs[i].job_id, jobs[i].arrival_time, jobs[i].burst_time,
               jobs[i].priority, jobs[i].completion_time,
               jobs[i].turnaround_time, jobs[i].waiting_time);
    }
    printf("---+-----+-------+------+-------+------------+--------\n");
    printf("Average Turnaround Time: %.2f units\n", total_tat / n);
    printf("Average Waiting Time   : %.2f units\n", total_wt / n);
}

void run_fcfs_scheduler(Job jobs[], int n) {
    Job temp[MAX_JOBS];
    reset_jobs(jobs, temp, n);

    /* Sort by arrival time (Tie-break: lower Job ID) */
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp[j].arrival_time > temp[j + 1].arrival_time ||
               (temp[j].arrival_time == temp[j + 1].arrival_time && temp[j].job_id > temp[j + 1].job_id)) {
                Job t = temp[j]; temp[j] = temp[j + 1]; temp[j + 1] = t;
            }
        }
    }

    printf("\n[FCFS] Execution Timeline: ");
    int current_time = 0;
    for (int i = 0; i < n; i++) {
        if (current_time < temp[i].arrival_time)
            current_time = temp[i].arrival_time;
        
        printf("[T=%d: J%d runs to T=%d] ", current_time, temp[i].job_id, current_time + temp[i].burst_time);
        temp[i].start_time = current_time;
        current_time += temp[i].burst_time;
        temp[i].completion_time = current_time;
        temp[i].turnaround_time = temp[i].completion_time - temp[i].arrival_time;
        temp[i].waiting_time = temp[i].turnaround_time - temp[i].burst_time;
        temp[i].state = JOB_COMPLETED;
    }
    printf("\n");
    print_metrics(temp, n, "First-Come, First-Served (FCFS)");
}

void run_round_robin_scheduler(Job jobs[], int n, int quantum) {
    Job temp[MAX_JOBS];
    reset_jobs(jobs, temp, n);

    printf("\n[Round Robin] Time Quantum = %d\n", quantum);
    printf("Timeline: ");
    int completed = 0, current_time = 0;
    
    while (completed < n) {
        bool idle = true;
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= current_time && temp[i].remaining_burst > 0) {
                idle = false;
                int slice = (temp[i].remaining_burst > quantum) ? quantum : temp[i].remaining_burst;
                printf("[T=%d: J%d(%d)] ", current_time, temp[i].job_id, slice);
                temp[i].remaining_burst -= slice;
                current_time += slice;

                if (temp[i].remaining_burst == 0) {
                    temp[i].completion_time = current_time;
                    temp[i].turnaround_time = temp[i].completion_time - temp[i].arrival_time;
                    temp[i].waiting_time = temp[i].turnaround_time - temp[i].burst_time;
                    temp[i].state = JOB_COMPLETED;
                    completed++;
                }
            }
        }
        if (idle) {
            current_time++;
        }
    }
    printf("\n");
    print_metrics(temp, n, "Round Robin (RR)");
}

void run_priority_scheduler(Job jobs[], int n) {
    Job temp[MAX_JOBS];
    reset_jobs(jobs, temp, n);

    printf("\n[Non-Preemptive Priority] Rule: Lower numeric value = higher priority. Tie-breaker: Arrival time.\n");
    printf("Timeline: ");
    int completed = 0, current_time = 0;
    bool done_flags[MAX_JOBS] = {false};

    while (completed < n) {
        int best_idx = -1;
        int highest_prio = 999999;

        for (int i = 0; i < n; i++) {
            if (!done_flags[i] && temp[i].arrival_time <= current_time) {
                if (temp[i].priority < highest_prio) {
                    highest_prio = temp[i].priority;
                    best_idx = i;
                } else if (temp[i].priority == highest_prio && best_idx != -1) {
                    if (temp[i].arrival_time < temp[best_idx].arrival_time) {
                        best_idx = i;
                    }
                }
            }
        }

        if (best_idx == -1) {
            current_time++;
            continue;
        }

        printf("[T=%d: J%d runs to T=%d] ", current_time, temp[best_idx].job_id, current_time + temp[best_idx].burst_time);
        current_time += temp[best_idx].burst_time;
        temp[best_idx].completion_time = current_time;
        temp[best_idx].turnaround_time = temp[best_idx].completion_time - temp[best_idx].arrival_time;
        temp[best_idx].waiting_time = temp[best_idx].turnaround_time - temp[best_idx].burst_time;
        temp[best_idx].state = JOB_COMPLETED;
        done_flags[best_idx] = true;
        completed++;
    }
    printf("\n");
    print_metrics(temp, n, "Non-Preemptive Priority");
}

void compare_schedulers(Job jobs[], int n, int quantum) {
    printf("\n=======================================================\n");
    printf("             SCHEDULING POLICY COMPARISON             \n");
    printf("=======================================================\n");
    run_fcfs_scheduler(jobs, n);
    run_round_robin_scheduler(jobs, n, quantum);
    run_priority_scheduler(jobs, n);
}