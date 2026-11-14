#ifndef CAMPUS_COMPUTE_H
#define CAMPUS_COMPUTE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <errno.h>

/* FR1 & FR2: Job Management & Scheduling */
#define MAX_JOBS 32
#define DEFAULT_QUANTUM 2

typedef enum {
    JOB_READY,
    JOB_RUNNING,
    JOB_COMPLETED
} JobState;

typedef struct {
    int job_id;
    int arrival_time;
    int burst_time;
    int remaining_burst;
    int priority;           /* Lower value = Higher priority; Tie-breaker: Earlier Arrival */
    int start_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    JobState state;
} Job;

void run_fcfs_scheduler(Job jobs[], int n);
void run_round_robin_scheduler(Job jobs[], int n, int quantum);
void run_priority_scheduler(Job jobs[], int n);
void compare_schedulers(Job jobs[], int n, int quantum);

/* FR3: Concurrent Activity Logger */
#define LOG_QUEUE_CAPACITY 8
#define LOG_MAX_LINE 256

typedef struct {
    char messages[LOG_QUEUE_CAPACITY][LOG_MAX_LINE];
    int head;
    int tail;
    int count;
    bool done;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} LogQueue;

void run_concurrent_logger_demo(const char *log_filepath);

/* FR4: Shared Memory IPC & Semaphores */
#define SHM_NAME "/campus_compute_shm"
#define SEM_PARENT "/campus_compute_sem_parent"
#define SEM_CHILD  "/campus_compute_sem_child"

typedef struct {
    int active_jobs;
    int completed_jobs;
    double avg_turnaround;
    char lab_status[64];
} SharedLabData;

void run_ipc_demo(void);
void cleanup_system_ipc(void);

/* FR5: Banker's Algorithm */
#define MAX_PROCESSES 5
#define MAX_RESOURCES 3

typedef struct {
    int num_processes;
    int num_resources;
    int available[MAX_RESOURCES];
    int max[MAX_PROCESSES][MAX_RESOURCES];
    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
} ResourceState;

bool check_safe_state(const ResourceState *rs, int safe_seq[]);
void run_bankers_demo(void);

/* FR6 & FR7: Worker Management & Resource Limits */
void run_worker_normal_demo(void);
void run_worker_failure_demo(void);
void run_resource_limit_demo(void);

#endif /* CAMPUS_COMPUTE_H */