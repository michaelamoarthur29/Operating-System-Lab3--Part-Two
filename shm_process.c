// Shared Memory Example: Parent creates and shares data with child
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

void runClient(int *sharedData);

int main(int argc, char *argv[]) {
    int shm_id;
    int *shm_ptr;
    pid_t pid;
    int status;

    if (argc != 5) {
        fprintf(stderr, "Usage: %s num1 num2 num3 num4\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Create shared memory for 4 integers
    shm_id = shmget(IPC_PRIVATE, 4 * sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    printf("[Parent] Shared memory segment created successfully.\n");

    // Attach shared memory
    shm_ptr = (int *) shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    printf("[Parent] Shared memory attached.\n");

    // Store command-line arguments in shared memory
    for (int i = 0; i < 4; i++) {
        shm_ptr[i] = atoi(argv[i + 1]);
    }

    printf("[Parent] Stored values: %d %d %d %d\n",
           shm_ptr[0], shm_ptr[1], shm_ptr[2], shm_ptr[3]);

    printf("[Parent] Forking a child process...\n");

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) {
        // Child process
        runClient(shm_ptr);
        exit(EXIT_SUCCESS);
    } 
    else {
        // Parent process
        wait(&status);
        printf("[Parent] Child process completed.\n");

        // Detach and remove shared memory
        shmdt((void *) shm_ptr);
        printf("[Parent] Detached shared memory.\n");

        shmctl(shm_id, IPC_RMID, NULL);
        printf("[Parent] Shared memory removed.\n");

        printf("[Parent] Exiting program.\n");
    }

    return 0;
}

void runClient(int *sharedData) {
    printf("   [Child] Client process running...\n");
    printf("   [Child] Read values: %d %d %d %d\n",
           sharedData[0], sharedData[1], sharedData[2], sharedData[3]);
    printf("   [Child] Exiting client process.\n");
}
