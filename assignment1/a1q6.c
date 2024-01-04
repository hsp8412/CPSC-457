#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 10

// declare mutex lock
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// declare buffer, full and empty
int buffer[BUFFER_SIZE];
int full = 0;
int empty = BUFFER_SIZE;

// initialize buffer array
void initializeBuffer()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 0;
    }
}

// lock the mutex
void lock_mutex()
{
    int result = pthread_mutex_lock(&mutex);
    if (result != 0)
    {
        fprintf(stderr, "Failed to lock mutex: %s\n", strerror(result));
        exit(EXIT_FAILURE);
    }
}

// unlock the mutex
void unlock_mutex()
{
    int result = pthread_mutex_unlock(&mutex);
    if (result != 0)
    {
        fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(result));
        exit(EXIT_FAILURE);
    }
}

// check mutex availability
int checkMutexAvail()
{
    int result = pthread_mutex_trylock(&mutex);
    if (result == 0)
    {
        pthread_mutex_unlock(&mutex);
        printf("Mutex is available\n");
        return 1;
    }
    else if (result == EBUSY)
    {
        printf("Mutex is not available\n");
        return 0;
    }
    // if error occurs, print error message and exit
    else
    {
        fprintf(stderr, "Failed to check mutex availability: %s\n", strerror(result));
        exit(EXIT_FAILURE);
    }
}

// producer function
void producer()
{
    // lock mutex
    lock_mutex();

    printf("Producer is producing...\n");
    printf("Full: %d ; Empty: %d\n", full, empty);

    // check if buffer is full
    if (empty == 0)
    {
        printf("Buffer is full, producer cannot produce...\n");
        // if buffer is full, unlock mutex and return
        unlock_mutex();
        return;
    }

    // produce to buffer
    buffer[full] = 1;

    // update full and empty
    full++;
    empty--;

    printf("Producer has produced..\n");
    printf("Updated Full: %d ; Updated Empty: %d\n", full, empty);

    // unlock mutex and return
    unlock_mutex();
    return;
}

// consumer function
void consumer()
{
    // lock mutex
    lock_mutex();

    printf("Consumer is consuming...\n");
    printf("Full: %d ; Empty: %d\n", full, empty);

    // check if buffer is empty
    if (full == 0)
    {
        printf("Buffer is empty, consumer cannot consume...\n");
        // if buffer is empty, unlock mutex and return
        unlock_mutex();
        return;
    }

    // update full and empty
    full--;
    empty++;

    // consume from buffer
    buffer[full] = 0;

    printf("Consumer has consumed...\n");
    printf("Updated Full: %d ; Updated Empty: %d\n", full, empty);

    // unlock mutex and return
    unlock_mutex();
    return;
}

// function for printing information: buffer, full, empty and mutex availability
void information()
{
    // print buffer array
    printf("Buffer: ");
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        printf("[%d] -> [%d] ;;; ", i, buffer[i]);
    }

    // print full and empty
    printf("FULL SLOTS = %d AND EMPTY SLOTS = %d ;;; ", full, empty);

    // print mutex availability
    checkMutexAvail();

    return;
}

// main function
int main()
{
    // initialize buffer
    initializeBuffer();

    // print menu
    printf("1. Press 1 for Producer\n");
    printf("2. Press 2 for Consumer\n");
    printf("3. Press 3 for Information\n");
    printf("4. Press 4 for Exit\n");

    // infinite loop for getting user input
    while (1)
    {
        // prompt user for input
        printf("\nEnter your choice: ");
        int choice;
        int result = scanf("%d", &choice);

        // check if input is a valid integer
        if (result == 0)
        {
            printf("Invalid choice, please enter 1, 2, 3 or 4\n");
            // clear the input buffer to prevent infinite loop
            while (getchar() != '\n')
                ;
            continue;
        }

        // execute functions based on user input
        switch (choice)
        {
        case 1:
            producer();
            // clear the input buffer
            while (getchar() != '\n')
                ;
            break;
        case 2:
            consumer();
            // clear the input buffer
            while (getchar() != '\n')
                ;
            break;
        case 3:
            information();
            // clear the input buffer
            while (getchar() != '\n')
                ;
            break;
        case 4:
            // destroy mutex and exit
            pthread_mutex_destroy(&mutex);
            return 0;
        default:
            // for invalid input, print error message and prompt user for input again
            printf("Invalid choice, please enter 1, 2, 3 or 4\n");
            while (getchar() != '\n')
                ;
        }
    }
    return 0;
}