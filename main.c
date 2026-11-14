#include "campus_compute.h"

static void print_usage(const char *prog_name) {
    printf("\nUsage: %s [option]\n", prog_name);
    printf("Options:\n");
    printf("  --help, -h          Display this help and usage manual\n");
    printf("  --fcfs              Run FCFS CPU scheduling\n");
    printf("  --rr [quantum]      Run Round Robin scheduling (default quantum: 2)\n");
    printf("  --priority          Run Non-preemptive Priority scheduling\n");
    printf("  --compare           Run and compare all scheduling policies\n");
    printf("  --logger            Execute concurrent producer-consumer logger\n");
    printf("  --ipc               Demonstrate shared memory and semaphore IPC\n");
    printf("  --banker            Run Banker's resource safety algorithm\n");
    printf("  --worker-normal     Demonstrate normal worker completion\n");
    printf("  --worker-fail       Demonstrate worker failure detection\n");
    printf("  --rlimit            Demonstrate process resource limit (RLIMIT_CPU)\n");
    printf("  --all               Run complete integrated demonstration\n");
    printf("  (no args)           Start interactive menu\n");
}

static Job sample_jobs[4] = {
    { .job_id = 1, .arrival_time = 0, .burst_time = 5, .priority = 2 },
    { .job_id = 2, .arrival_time = 1, .burst_time = 3, .priority = 1 },
    { .job_id = 3, .arrival_time = 2, .burst_time = 8, .priority = 4 },
    { .job_id = 4, .arrival_time = 3, .burst_time = 6, .priority = 3 }
};

static void run_all_integrated(void) {
    printf("\n=======================================================\n");
    printf("     CAMPUSCOMPUTE: INTEGRATED END-TO-END DEMO         \n");
    printf("=======================================================\n");
    compare_schedulers(sample_jobs, 4, DEFAULT_QUANTUM);
    run_concurrent_logger_demo("campus_compute.log");
    run_ipc_demo();
    run_bankers_demo();
    run_worker_normal_demo();
    run_worker_failure_demo();
    run_resource_limit_demo();
    printf("\n=======================================================\n");
    printf("         INTEGRATED DEMONSTRATION COMPLETE             \n");
    printf("=======================================================\n");
}

static void run_menu(void) {
    int choice;
    while (1) {
        printf("\n===================================================\n");
        printf("      CampusCompute: Lab Management System         \n");
        printf("===================================================\n");
        printf(" 1. Run FCFS Scheduling\n");
        printf(" 2. Run Round Robin Scheduling (Quantum=%d)\n", DEFAULT_QUANTUM);
        printf(" 3. Run Non-Preemptive Priority Scheduling\n");
        printf(" 4. Compare All Scheduling Algorithms\n");
        printf(" 5. Test Concurrent Activity Logger (Threads)\n");
        printf(" 6. Test Shared Memory IPC and Semaphores\n");
        printf(" 7. Run Banker's Safety Algorithm\n");
        printf(" 8. Test Worker Normal Execution\n");
        printf(" 9. Test Worker Failure Detection\n");
        printf(" 10. Test Resource Limit Enforcement (setrlimit)\n");
        printf(" 11. Run All Tests (Integrated)\n");
        printf("  0. Exit\n");
        printf("Select an option [0-11]: ");

        if (scanf("%d", &choice) != 1) {
            printf("\n[ERROR] Invalid input. Please enter a valid number.\n");
            while (getchar() != '\n'); /* Flush buffer */
            continue;
        }

        switch (choice) {
            case 1: run_fcfs_scheduler(sample_jobs, 4); break;
            case 2: run_round_robin_scheduler(sample_jobs, 4, DEFAULT_QUANTUM); break;
            case 3: run_priority_scheduler(sample_jobs, 4); break;
            case 4: compare_schedulers(sample_jobs, 4, DEFAULT_QUANTUM); break;
            case 5: run_concurrent_logger_demo("campus_compute.log"); break;
            case 6: run_ipc_demo(); break;
            case 7: run_bankers_demo(); break;
            case 8: run_worker_normal_demo(); break;
            case 9: run_worker_failure_demo(); break;
            case 10: run_resource_limit_demo(); break;
            case 11: run_all_integrated(); break;
            case 0:
                printf("\nExiting CampusCompute. Ensuring all resources are unlinked...\n");
                cleanup_system_ipc();
                return;
            default:
                printf("\n[ERROR] Choice out of range. Choose between 0 and 11.\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        run_menu();
        return 0;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
    } else if (strcmp(argv[1], "--fcfs") == 0) {
        run_fcfs_scheduler(sample_jobs, 4);
    } else if (strcmp(argv[1], "--rr") == 0) {
        int q = DEFAULT_QUANTUM;
        if (argc >= 3) {
            q = atoi(argv[2]);
            if (q <= 0) {
                fprintf(stderr, "[ERROR] Invalid quantum value '%s'. Quantum must be a positive integer.\n", argv[2]);
                return 1;
            }
        }
        run_round_robin_scheduler(sample_jobs, 4, q);
    } else if (strcmp(argv[1], "--priority") == 0) {
        run_priority_scheduler(sample_jobs, 4);
    } else if (strcmp(argv[1], "--compare") == 0) {
        compare_schedulers(sample_jobs, 4, DEFAULT_QUANTUM);
    } else if (strcmp(argv[1], "--logger") == 0) {
        run_concurrent_logger_demo("campus_compute.log");
    } else if (strcmp(argv[1], "--ipc") == 0) {
        run_ipc_demo();
    } else if (strcmp(argv[1], "--banker") == 0) {
        run_bankers_demo();
    } else if (strcmp(argv[1], "--worker-normal") == 0) {
        run_worker_normal_demo();
    } else if (strcmp(argv[1], "--worker-fail") == 0) {
        run_worker_failure_demo();
    } else if (strcmp(argv[1], "--rlimit") == 0) {
        run_resource_limit_demo();
    } else if (strcmp(argv[1], "--all") == 0) {
        run_all_integrated();
    } else {
        fprintf(stderr, "[ERROR] Unrecognized command '%s'.\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}