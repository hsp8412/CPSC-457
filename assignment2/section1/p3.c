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

//helper function to 
void to_uppercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Page size in memory
#define PAGESIZE 4096

// Define shared memory struct
enum Flag{
    INIT,
    SENT,
};

typedef struct {
    char message[100];
    enum Flag flag;
} Shared_Memory;


int main(int argc, char *argv[])
{
    // create shared memory
    Shared_Memory *shared_memory = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED)
    {
        perror("Failed to create shared memory");
        exit(1);
    }

    // initialize the flag to init
    shared_memory->flag = INIT;

    // create child process
    pid_t child_pid;
    char message[12] = "hello world";
    if ((child_pid = fork()) < 0)
    {
        perror("Failed to create child process");
        exit(1);
    }

    if (child_pid == 0)
    {
        // child code
        // child wait until parent write to shared memory
        while(shared_memory->flag == INIT);
        printf("Child reading from shared memory: %s\n", shared_memory);

        // child convert the message to uppercase
        to_uppercase(shared_memory->message);
        printf("Child writing to shared memory: %s\n", shared_memory);

        // child set the flag back to init
        shared_memory->flag = INIT;
    }
    else
    {
        // parent code
        // parent write to the shared memory andINIT change the flag
        printf("Parent writing to shared memory: %s\n", message);
        strncpy(shared_memory->message, message, sizeof(message));

        shared_memory->flag = SENT;
        
        // parent wait until child complete and set the flag back to init
        while(shared_memory->flag == SENT);
        printf("Parent reading from shared memory: %s\n", shared_memory);

        // clean up shared memory
        if (munmap(shared_memory, PAGESIZE) == -1)
        {
            perror("Failed to unmap shared memory");
            exit(1);
        }
    }
    return 0;
}