#include "campus_compute.h"

void cleanup_system_ipc(void) {
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_PARENT);
    sem_unlink(SEM_CHILD);
}

void run_ipc_demo(void) {
    cleanup_system_ipc();

    printf("\n[IPC Demo] Creating POSIX Shared Memory segment (%s)...\n", SHM_NAME);
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return;
    }
    if (ftruncate(shm_fd, sizeof(SharedLabData)) == -1) {
        perror("ftruncate failed");
        close(shm_fd);
        return;
    }

    SharedLabData *shared_mem = (SharedLabData*)mmap(NULL, sizeof(SharedLabData),
                                                     PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        return;
    }

    /* Initialize synchronization semaphores */
    sem_t *sem_p = sem_open(SEM_PARENT, O_CREAT | O_EXCL, 0666, 0);
    sem_t *sem_c = sem_open(SEM_CHILD, O_CREAT | O_EXCL, 0666, 0);

    if (sem_p == SEM_FAILED || sem_c == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_mem, sizeof(SharedLabData));
        close(shm_fd);
        cleanup_system_ipc();
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        /* Child Process: Worker/Consumer */
        printf("  [Worker Process PID=%d] Attached to shared memory, waiting for lab metrics...\n", getpid());
        sem_wait(sem_p); /* Wait for parent initialization */

        printf("  [Worker Process PID=%d] Read from SHM: Lab Status=\"%s\", Active Jobs=%d\n",
               getpid(), shared_mem->lab_status, shared_mem->active_jobs);
        
        /* Modify data in shared memory */
        shared_mem->completed_jobs += 3;
        shared_mem->active_jobs -= 3;
        shared_mem->avg_turnaround = 4.33;
        strncpy(shared_mem->lab_status, "OPTIMAL_LOAD_BALANCED", sizeof(shared_mem->lab_status) - 1);
        printf("  [Worker Process PID=%d] Updated SHM: Completed=3, Status=\"%s\"\n",
               getpid(), shared_mem->lab_status);

        sem_post(sem_c); /* Notify parent */

        munmap(shared_mem, sizeof(SharedLabData));
        close(shm_fd);
        sem_close(sem_p);
        sem_close(sem_c);
        exit(EXIT_SUCCESS);
    } else {
        /* Parent Process: Coordinator */
        printf("[Coordinator PID=%d] Writing initial metrics to shared memory...\n", getpid());
        shared_mem->active_jobs = 10;
        shared_mem->completed_jobs = 0;
        shared_mem->avg_turnaround = 0.0;
        strncpy(shared_mem->lab_status, "INITIALIZING_BATCH", sizeof(shared_mem->lab_status) - 1);

        sem_post(sem_p); /* Release child */
        sem_wait(sem_c); /* Wait for child update */

        printf("[Coordinator PID=%d] Post-IPC read from SHM: Status=\"%s\", Completed Jobs=%d, Avg TAT=%.2f\n",
               getpid(), shared_mem->lab_status, shared_mem->completed_jobs, shared_mem->avg_turnaround);

        waitpid(pid, NULL, 0);

        /* Release and Unlink Resources */
        munmap(shared_mem, sizeof(SharedLabData));
        close(shm_fd);
        sem_close(sem_p);
        sem_close(sem_c);
        cleanup_system_ipc();
        printf("[Coordinator PID=%d] Cleaned up shm_unlink() and sem_unlink() successfully.\n", getpid());
    }
}