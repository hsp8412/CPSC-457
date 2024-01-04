#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define NUM_CHILD 3
#define PAGESIZE 4096
#define BUFFER_SIZE 100

// the scanning interval of parent
#define SCAN_INTERVAL 2

// maximum sleep time for child processes
#define MAX_SLEEP_TIME 4

// child's status
enum Status
{
    PENDING,
    SENT,
};

typedef struct
{
    char buffer[NUM_CHILD][BUFFER_SIZE];
    enum Status status[NUM_CHILD];

    // for blocking child after sending a new message
    sem_t child[NUM_CHILD];

    // for exclusive access
    sem_t mutex;
} SharedMemory;

int main(int argc, char *argv[])
{
    srand(time(NULL));

    // create shared memory
    SharedMemory *shared_memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("Failed to create shared memory");
        exit(1);
    }

    // initialize mutex
    if (sem_init(&shared_memory->mutex, 1, 1) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    // initialize semaphores for child processes
    for (int i = 0; i < NUM_CHILD; i++)
    {
        shared_memory->status[i] = PENDING;
        if (sem_init(&shared_memory->child[i], 1, 0) == -1)
        {
            perror("sem_init");
            exit(1);
        }
    }

    // create child processes
    pid_t children[NUM_CHILD];
    for (int i = 0; i < NUM_CHILD; i++)
    {
        srand(time(NULL));
        children[i] = fork();
        if (children[i] < 0)
        {
            perror("Failed to create child process");
            exit(1);
        }

        // for child
        else if (children[i] == 0)
        {
            srand(getpid());
            while (1)
            {
                sem_wait(&shared_memory->mutex);

                // write the message to buffer
                snprintf(shared_memory->buffer[i], BUFFER_SIZE, "Message from child %d", i);
                printf("Child %d sent a new message\n", i);
                shared_memory->status[i] = SENT;

                sem_post(&shared_memory->mutex);

                // block the process, wake up when parent reads the message and signal
                sem_wait(&shared_memory->child[i]);

                // sleep for a random time before trying to send a new message
                int sleep_time = (rand() % MAX_SLEEP_TIME) + 1;
                sleep(sleep_time);
            }
        }
    }

    // for parent
    while (1)
    {
        sem_wait(&shared_memory->mutex);
        printf("Parent is scanning...\n");

        // scan the buffers for new messages
        for (int i = 0; i < NUM_CHILD; i++)
        {
            // read only when new message is sent (with the status "SENT")
            if (shared_memory->status[i] == SENT)
            {
                printf("Parent reading: %s\n", shared_memory->buffer[i]);
                shared_memory->status[i] = PENDING;

                // wake up the process whose new message is just read by parent
                sem_post(&shared_memory->child[i]);
            }
        }
        printf("Scanning completed...\n");
        sem_post(&shared_memory->mutex);

        // sleep for a fixed period of time
        sleep(SCAN_INTERVAL);
    }
}