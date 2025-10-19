#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define TRUE 1
#define FALSE 0

int main() {
    int shm_id;
    int *shared_data;
    pid_t child_pid;
    int status;

    // Allocate shared memory for two integers: [0] -> Balance, [1] -> Turn flag
    shm_id = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("Error: shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment
    shared_data = (int *) shmat(shm_id, NULL, 0);
    if ((long)shared_data == -1) {
        perror("Error: shmat failed");
        exit(EXIT_FAILURE);
    }

    // Initialize shared variables
    shared_data[0] = 0; // Bank balance
    shared_data[1] = 0; // Turn (0 = Parent, 1 = Student)

    srand(time(NULL));

    // Fork the process
    child_pid = fork();

    if (child_pid < 0) {
        perror("Error: fork failed");
        exit(EXIT_FAILURE);
    }

    // ---------------- CHILD PROCESS (Poor Student) ----------------
    if (child_pid == 0) {
        for (int i = 0; i < 25; i++) {
            sleep(rand() % 6); // Random delay between 0–5 seconds
            int balance = shared_data[0];

            // Wait for turn
            while (shared_data[1] != 1);

            int withdraw = rand() % 51; // Student needs $0–50
            printf("Poor Student needs $%d\n", withdraw);

            if (withdraw <= balance) {
                balance -= withdraw;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", withdraw, balance);
            } else {
                printf("Poor Student: Not enough money in account ($%d)\n", balance);
            }

            // Update shared memory
            shared_data[0] = balance;
            shared_data[1] = 0; // Parent’s turn
        }
        exit(0);
    }

    // ---------------- PARENT PROCESS (Dear Old Dad) ----------------
    else {
        for (int i = 0; i < 25; i++) {
            sleep(rand() % 6); // Random delay between 0–5 seconds
            int balance = shared_data[0];

            // Wait for turn
            while (shared_data[1] != 0);

            if (balance <= 100) {
                int deposit = rand() % 101; // Deposit $0–100
                if (deposit % 2 == 0) {
                    balance += deposit;
                    printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", deposit, balance);
                } else {
                    printf("Dear Old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear Old Dad: Thinks Student has enough cash ($%d)\n", balance);
            }

            // Update shared memory
            shared_data[0] = balance;
            shared_data[1] = 1; // Student’s turn
        }

        // Wait for child to finish
        wait(&status);
        printf("Dear Old Dad: Waited for Child to Complete\n");

        // Clean up shared memory
        shmdt(shared_data);
        shmctl(shm_id, IPC_RMID, NULL);

        printf("Shared memory released. Program finished.\n");
        exit(0);
    }
}
