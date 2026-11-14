#include "campus_compute.h"

void run_worker_normal_demo(void) {
    printf("\n[Process Management] Forking child worker for normal execution...\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        printf("  [Worker PID=%d] Commencing CPU-bound task simulation...\n", getpid());
        usleep(100000); /* 100ms simulation */
        printf("  [Worker PID=%d] Completed execution successfully. Exiting code 0.\n", getpid());
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("[Parent/Coordinator] Detected Child PID=%d exited normally with status=%d.\n",
                   pid, WEXITSTATUS(status));
        } else {
            printf("[Parent/Coordinator] Child PID=%d terminated abnormally.\n", pid);
        }
    }
}

void run_worker_failure_demo(void) {
    printf("\n[Process Management] Forking child worker for SIMULATED FAILURE...\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        printf("  [Worker PID=%d] Running simulation... Encounters error code 42 (Simulated Worker Failure).\n", getpid());
        exit(42);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("[Parent/Coordinator] Worker failure detected! PID=%d exited with non-zero status=%d.\n",
                   pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[Parent/Coordinator] Child PID=%d killed by signal=%d.\n", pid, WTERMSIG(status));
        }
    }
}

void run_resource_limit_demo(void) {
    printf("\n[Resource Limits] Demonstrating CPU time limit enforcement via setrlimit()...\n");
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        /* Restrict process to 1 second of CPU time */
        struct rlimit rl;
        rl.rlim_cur = 1;
        rl.rlim_max = 1;
        if (setrlimit(RLIMIT_CPU, &rl) != 0) {
            perror("setrlimit failed");
            exit(1);
        }

        printf("  [Child PID=%d] RLIMIT_CPU set to 1s. Entering infinite CPU computation...\n", getpid());
        volatile unsigned long count = 0;
        while (1) {
            count++;
        }
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFSIGNALED(status)) {
            printf("[Parent/Coordinator] Success: Process exceeded resource limit and was terminated by signal: %s (SIGXCPU=%d)\n",
                   strsignal(WTERMSIG(status)), WTERMSIG(status));
        } else {
            printf("[Parent/Coordinator] Process exited unexpectedly without signal.\n");
        }
    }
}