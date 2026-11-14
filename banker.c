#include "campus_compute.h"

bool check_safe_state(const ResourceState *rs, int safe_seq[]) {
    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES] = {false};

    for (int j = 0; j < rs->num_resources; j++) {
        work[j] = rs->available[j];
    }

    int count = 0;
    while (count < rs->num_processes) {
        bool found = false;
        for (int p = 0; p < rs->num_processes; p++) {
            if (!finish[p]) {
                bool can_allocate = true;
                for (int r = 0; r < rs->num_resources; r++) {
                    if (rs->need[p][r] > work[r]) {
                        can_allocate = false;
                        break;
                    }
                }
                if (can_allocate) {
                    for (int r = 0; r < rs->num_resources; r++) {
                        work[r] += rs->allocation[p][r];
                    }
                    safe_seq[count++] = p;
                    finish[p] = true;
                    found = true;
                }
            }
        }
        if (!found) {
            return false; /* Unsafe state */
        }
    }
    return true;
}

static void print_matrices(const ResourceState *rs) {
    printf("PID | Allocation |   Max Need   | Current Need \n");
    printf("----+------------+--------------+--------------\n");
    for (int i = 0; i < rs->num_processes; i++) {
        printf(" P%d | ", i);
        for (int r = 0; r < rs->num_resources; r++) printf("%d ", rs->allocation[i][r]);
        printf("     | ");
        for (int r = 0; r < rs->num_resources; r++) printf("%d ", rs->max[i][r]);
        printf("       | ");
        for (int r = 0; r < rs->num_resources; r++) printf("%d ", rs->need[i][r]);
        printf("\n");
    }
    printf("Available Resources: ");
    for (int r = 0; r < rs->num_resources; r++) {
        printf("R%d=%d ", r, rs->available[r]);
    }
    printf("\n");
}

void run_bankers_demo(void) {
    printf("\n=======================================================\n");
    printf("       BANKER'S ALGORITHM RESOURCE SAFETY ANALYSIS     \n");
    printf("=======================================================\n");

    /* Case A: Demonstrating a Safe State */
    printf("\n--- Test Case 1: Verifying Known Safe State ---\n");
    ResourceState safe_case = {
        .num_processes = 5,
        .num_resources = 3,
        .available = {3, 3, 2},
        .max = {
            {7, 5, 3},
            {3, 2, 2},
            {9, 0, 2},
            {2, 2, 2},
            {4, 3, 3}
        },
        .allocation = {
            {0, 1, 0},
            {2, 0, 0},
            {3, 0, 2},
            {2, 1, 1},
            {0, 0, 2}
        }
    };

    /* Calculate Need = Max - Allocation */
    for (int i = 0; i < safe_case.num_processes; i++) {
        for (int j = 0; j < safe_case.num_resources; j++) {
            safe_case.need[i][j] = safe_case.max[i][j] - safe_case.allocation[i][j];
        }
    }

    print_matrices(&safe_case);
    int safe_seq[MAX_PROCESSES];
    if (check_safe_state(&safe_case, safe_seq)) {
        printf("[RESULT] State is SAFE. Valid execution sequence: <");
        for (int i = 0; i < safe_case.num_processes; i++) {
            printf(" P%d%s", safe_seq[i], (i == safe_case.num_processes - 1) ? " " : " ->");
        }
        printf(">\n");
    } else {
        printf("[RESULT] State is UNSAFE (Deadlock danger detected).\n");
    }

    /* Case B: Demonstrating an Unsafe State */
    printf("\n--- Test Case 2: Verifying Deadlock/Unsafe State ---\n");
    ResourceState unsafe_case = safe_case;
    unsafe_case.available[0] = 0;
    unsafe_case.available[1] = 0;
    unsafe_case.available[2] = 0;

    print_matrices(&unsafe_case);
    if (check_safe_state(&unsafe_case, safe_seq)) {
        printf("[RESULT] State is SAFE.\n");
    } else {
        printf("[RESULT] State is UNSAFE: No process can be allocated resources to completion.\n");
    }
}