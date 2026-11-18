#include "campus_compute.h"

/* Each job is stored once in a linked list. Queues hold pointers to these
   records, allowing scheduling order to change without using job arrays. */
typedef struct JobNode {
    Job job;
    struct JobNode *next;
} JobNode;

typedef struct QueueNode {
    JobNode *job;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} JobQueue;

static void queue_init(JobQueue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

static bool queue_push(JobQueue *q, JobNode *job) {
    QueueNode *node = malloc(sizeof(*node));
    if (node == NULL) return false;
    node->job = job;
    node->next = NULL;
    if (q->rear == NULL) q->front = node;
    else q->rear->next = node;
    q->rear = node;
    q->size++;
    return true;
}

static JobNode *queue_pop(JobQueue *q) {
    if (q->front == NULL) return NULL;
    QueueNode *node = q->front;
    JobNode *job = node->job;
    q->front = node->next;
    if (q->front == NULL) q->rear = NULL;
    q->size--;
    free(node);
    return job;
}

static void queue_clear(JobQueue *q) {
    while (queue_pop(q) != NULL) { }
}

static JobNode *create_jobs(const Job jobs[], int n) {
    JobNode *head = NULL;
    JobNode *tail = NULL;
    for (int i = 0; i < n; i++) {
        JobNode *node = malloc(sizeof(*node));
        if (node == NULL) {
            while (head != NULL) {
                JobNode *next = head->next;
                free(head);
                head = next;
            }
            return NULL;
        }
        node->job = jobs[i];
        node->job.remaining_burst = jobs[i].burst_time;
        node->job.start_time = -1;
        node->job.completion_time = 0;
        node->job.turnaround_time = 0;
        node->job.waiting_time = 0;
        node->job.state = JOB_READY;
        node->next = NULL;
        if (tail == NULL) head = node;
        else tail->next = node;
        tail = node;
    }
    return head;
}

static void free_jobs(JobNode *head) {
    while (head != NULL) {
        JobNode *next = head->next;
        free(head);
        head = next;
    }
}

static void print_metrics(JobNode *jobs, int n, const char *policy_name) {
    double total_tat = 0.0, total_wt = 0.0;
    printf("\n--- %s Execution Metrics ---\n", policy_name);
    printf("ID | Arr | Burst | Prio | Compl | Turnaround | Waiting\n");
    printf("---+-----+-------+------+-------+------------+--------\n");
    for (JobNode *node = jobs; node != NULL; node = node->next) {
        Job *job = &node->job;
        total_tat += job->turnaround_time;
        total_wt += job->waiting_time;
        printf("%2d | %3d | %5d | %4d | %5d | %10d | %7d\n",
               job->job_id, job->arrival_time, job->burst_time,
               job->priority, job->completion_time,
               job->turnaround_time, job->waiting_time);
    }
    printf("---+-----+-------+------+-------+------------+--------\n");
    printf("Average Turnaround Time: %.2f units\n", total_tat / n);
    printf("Average Waiting Time   : %.2f units\n", total_wt / n);
}

static void complete_job(Job *job, int current_time) {
    job->completion_time = current_time;
    job->turnaround_time = job->completion_time - job->arrival_time;
    job->waiting_time = job->turnaround_time - job->burst_time;
    job->state = JOB_COMPLETED;
}

void run_fcfs_scheduler(Job jobs[], int n) {
    JobNode *records = create_jobs(jobs, n);
    if (records == NULL && n > 0) {
        fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
        return;
    }

    /* Stable linked-list sort: arrival time, then lower Job ID. */
    for (JobNode *i = records; i != NULL; i = i->next) {
        for (JobNode *j = i->next; j != NULL; j = j->next) {
            if (i->job.arrival_time > j->job.arrival_time ||
                (i->job.arrival_time == j->job.arrival_time &&
                 i->job.job_id > j->job.job_id)) {
                Job t = i->job;
                i->job = j->job;
                j->job = t;
            }
        }
    }

    JobQueue ready;
    queue_init(&ready);
    for (JobNode *node = records; node != NULL; node = node->next) {
        if (!queue_push(&ready, node)) {
            fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
            queue_clear(&ready);
            free_jobs(records);
            return;
        }
    }

    printf("\n[FCFS] Execution Timeline: ");
    int current_time = 0;
    JobNode *node;
    while ((node = queue_pop(&ready)) != NULL) {
        Job *job = &node->job;
        if (current_time < job->arrival_time)
            current_time = job->arrival_time;
        printf("[T=%d: J%d runs to T=%d] ", current_time, job->job_id,
               current_time + job->burst_time);
        job->start_time = current_time;
        current_time += job->burst_time;
        complete_job(job, current_time);
    }
    printf("\n");
    print_metrics(records, n, "First-Come, First-Served (FCFS)");
    free_jobs(records);
}

void run_round_robin_scheduler(Job jobs[], int n, int quantum) {
    JobNode *records = create_jobs(jobs, n);
    if (records == NULL && n > 0) {
        fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
        return;
    }

    JobQueue ready;
    queue_init(&ready);
    for (JobNode *node = records; node != NULL; node = node->next) {
        if (!queue_push(&ready, node)) {
            fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
            queue_clear(&ready);
            free_jobs(records);
            return;
        }
    }

    printf("\n[Round Robin] Time Quantum = %d\n", quantum);
    printf("Timeline: ");
    int completed = 0, current_time = 0;

    /* Process one full pass of the current queue per round. Re-queueing each
       unfinished job preserves the original program's input-order scan. */
    while (completed < n) {
        bool idle = true;
        int pass_count = ready.size;
        for (int i = 0; i < pass_count; i++) {
            JobNode *node = queue_pop(&ready);
            Job *job = &node->job;
            if (job->arrival_time <= current_time && job->remaining_burst > 0) {
                idle = false;
                int slice = (job->remaining_burst > quantum) ? quantum : job->remaining_burst;
                printf("[T=%d: J%d(%d)] ", current_time, job->job_id, slice);
                job->remaining_burst -= slice;
                current_time += slice;
                if (job->remaining_burst == 0) {
                    complete_job(job, current_time);
                    completed++;
                } else if (!queue_push(&ready, node)) {
                    fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
                    queue_clear(&ready);
                    free_jobs(records);
                    return;
                }
            } else if (!queue_push(&ready, node)) {
                fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
                queue_clear(&ready);
                free_jobs(records);
                return;
            }
        }
        if (idle) current_time++;
    }
    printf("\n");
    print_metrics(records, n, "Round Robin (RR)");
    queue_clear(&ready);
    free_jobs(records);
}

void run_priority_scheduler(Job jobs[], int n) {
    JobNode *records = create_jobs(jobs, n);
    if (records == NULL && n > 0) {
        fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
        return;
    }

    JobQueue ready;
    queue_init(&ready);
    for (JobNode *node = records; node != NULL; node = node->next) {
        if (!queue_push(&ready, node)) {
            fprintf(stderr, "[ERROR] Unable to allocate scheduler queue.\n");
            queue_clear(&ready);
            free_jobs(records);
            return;
        }
    }

    printf("\n[Non-Preemptive Priority] Rule: Lower numeric value = higher priority. Tie-breaker: Arrival time.\n");
    printf("Timeline: ");
    int completed = 0, current_time = 0;

    while (completed < n) {
        JobNode *best = NULL;
        for (QueueNode *entry = ready.front; entry != NULL; entry = entry->next) {
            Job *candidate = &entry->job->job;
            if (candidate->arrival_time <= current_time &&
                (best == NULL || candidate->priority < best->job.priority ||
                 (candidate->priority == best->job.priority &&
                  candidate->arrival_time < best->job.arrival_time))) {
                best = entry->job;
            }
        }

        if (best == NULL) {
            current_time++;
            continue;
        }

        /* Remove the selected job from the ready queue while preserving the
           relative order of all other jobs. */
        QueueNode *previous = NULL;
        QueueNode *entry = ready.front;
        while (entry != NULL && entry->job != best) {
            previous = entry;
            entry = entry->next;
        }
        if (entry != NULL) {
            if (previous == NULL) ready.front = entry->next;
            else previous->next = entry->next;
            if (ready.rear == entry) ready.rear = previous;
            ready.size--;
            free(entry);
        }

        printf("[T=%d: J%d runs to T=%d] ", current_time, best->job.job_id,
               current_time + best->job.burst_time);
        current_time += best->job.burst_time;
        complete_job(&best->job, current_time);
        completed++;
    }
    printf("\n");
    print_metrics(records, n, "Non-Preemptive Priority");
    queue_clear(&ready);
    free_jobs(records);
}

void compare_schedulers(Job jobs[], int n, int quantum) {
    printf("\n=======================================================\n");
    printf("             SCHEDULING POLICY COMPARISON             \n");
    printf("=======================================================\n");
    run_fcfs_scheduler(jobs, n);
    run_round_robin_scheduler(jobs, n, quantum);
    run_priority_scheduler(jobs, n);
}
