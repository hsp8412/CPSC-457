#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// buffer size
#define BUFFER_SIZE 10

// sleep time for threads
#define BUFFER_INFO_SLEEP 4
#define PRODUCER_SLEEP_LOW 1
#define PRODUCER_SLEEP_HIGH 10
#define CONSUMER_SLEEP_LOW 5
#define CONSUMER_SLEEP_HIGH 15

// declare mutex lock
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// flag for mutex availability, 1 for available, 0 for not available
int mutex_avail = 1;

// declare buffer, full and empty
int buffer[BUFFER_SIZE];
int full = 0;
int empty = BUFFER_SIZE;

// declare object counter A and B
// A keeps track of the number of object 1 in the buffer
// B keeps track of the number of object 2 in the buffer
int A = 0;
int B = 0;

// initialize buffer array
void initializeBuffer()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = 0;
    }
}

/**Helper functions**/

// lock the mutex
void lock_mutex()
{
    // set flag to 0 to indicate mutex is not available anymore
    mutex_avail = 0;
    int result = pthread_mutex_lock(&mutex);

    // handle error if mutex lock fails
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

    // handle error if mutex unlock fails
    if (result != 0)
    {
        fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(result));
        exit(EXIT_FAILURE);
    }
    // set flag to 1 to indicate mutex is available again
    mutex_avail = 1;
}

// find the first empty slot in the buffer and produce the input object(1 or 2)
void produce(int object)
{
    // loop through buffer array and find the first empty slot
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (buffer[i] == 0)
        {
            // put object in the empty slot
            buffer[i] = object;
            break;
        }
    }
    full++;
    empty--;
    if (object == 1)
    {
        A++;
    }
    else
    {
        B++;
    }
}

// find the last specified object(1 or 2) in the buffer and consume it
void consume(int object)
{
    // loop through buffer array and find the last object
    for (int i = BUFFER_SIZE - 1; i >= 0; i--)
    {
        if (buffer[i] == object)
        {
            // consume object
            buffer[i] = 0;
            break;
        }
    }
    full--;
    empty++;
    if (object == 1)
    {
        A--;
    }
    else
    {
        B--;
    }
}

// make a thread sleep for random seconds between the input low and high values
void delay(int low, int high)
{
    // sleep for a random value between low and high
    int delay = rand() % (high - low + 1) + low;
    sleep(delay);
}

/**Thread Functions**/

// buffer information thread
void *buffer_Info(void *arg)
{
    // infinite loop
    while (1)
    {

        printf("Buffer Information:\n");

        // print buffer array
        printf("Buffer: ");

        // loop through buffer array and print each element
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            printf("[%d] -> [%d] ;;; ", i, buffer[i]);
        }

        // print full and empty
        printf("FULL SLOTS = %d AND EMPTY SLOTS = %d ;;; ", full, empty);

        // print mutex availability
        if (mutex_avail == 1)
        {
            printf("Mutex is available\n\n");
        }
        else
        {
            printf("Mutex is not available\n\n");
        }

        // sleep for 4 seconds
        sleep(BUFFER_INFO_SLEEP);
    }
}

// producer A thread
void *producer_A(void *arg)
{
    // infinite loop
    while (1)
    {
        // lock mutex
        lock_mutex();

        printf("Producer A is producing...\n");
        printf("Full: %d ; Empty: %d\n", full, empty);

        // if buffer is full, producer A cannot produce
        if (empty == 0)
        {
            printf("Buffer is full, producer A cannot produce...\n\n");

            // unlock mutex
            unlock_mutex();

            // sleep for 1-10 seconds before next iteration
            delay(PRODUCER_SLEEP_LOW, PRODUCER_SLEEP_HIGH);
            continue;
        }

        produce(1);

        printf("Producer A has produced..\n");
        printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

        // unlock mutex
        unlock_mutex();

        // sleep for 1-10 seconds before next iteration
        delay(PRODUCER_SLEEP_LOW, PRODUCER_SLEEP_HIGH);
    }
}

// producer B thread
void *producer_B(void *arg)
{
    // infinite loop
    while (1)
    {
        // lock mutex
        lock_mutex();

        printf("Producer B is producing...\n");
        printf("Full: %d ; Empty: %d\n", full, empty);

        // if buffer is full, producer B cannot produce
        if (empty == 0)
        {
            printf("Buffer is full, producer B cannot produce...\n\n");

            // unlock mutex
            unlock_mutex();

            // sleep for 1-10 seconds before next iteration
            delay(PRODUCER_SLEEP_LOW, PRODUCER_SLEEP_HIGH);
            continue;
        }

        produce(2);

        printf("Producer B has produced..\n");
        printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

        // unlock mutex
        unlock_mutex();

        // sleep for 1-10 seconds before next iteration
        delay(PRODUCER_SLEEP_LOW, PRODUCER_SLEEP_HIGH);
    }
}

// consumer A thread
void *consumer_A(void *arg)
{
    // infinite loop
    while (1)
    {
        // lock mutex
        lock_mutex();

        printf("Consumer A is consuming...\n");
        printf("Full: %d ; Empty: %d\n", full, empty);

        // if buffer does not contain object 1, consumer A cannot consume
        if (A == 0)
        {
            printf("Buffer does not contain object 1, consumer A cannot consume...\n\n");

            // unlock mutex
            unlock_mutex();

            // sleep for 5-15 seconds before next iteration
            delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);

            continue;
        }

        consume(1);

        printf("Consumer A has consumed...\n");
        printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

        // unlock mutex
        unlock_mutex();

        // sleep for 5-15 seconds before next iteration
        delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
    }
}

// consumer B thread
void *consumer_B(void *arg)
{
    // infinite loop
    while (1)
    {
        // lock mutex
        lock_mutex();

        printf("Consumer B is consuming...\n");
        printf("Full: %d ; Empty: %d\n", full, empty);

        // if buffer does not contain object 2, consumer B cannot consume
        if (B == 0)
        {
            printf("Buffer does not contain object 2, consumer B cannot consume...\n\n");

            // unlock mutex
            unlock_mutex();

            // sleep for 5-15 seconds before next iteration
            delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
            continue;
        }

        consume(2);

        printf("Consumer B has consumed...\n");
        printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

        // unlock mutex
        unlock_mutex();

        // sleep for 5-15 seconds before next iteration
        delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
    }
}

// consumer AB thread
void *consumer_AB(void *arg)
{
    // infinite loop
    while (1)
    {
        // lock mutex
        lock_mutex();

        printf("Consumer AB is consuming...\n");
        printf("Full: %d ; Empty: %d\n", full, empty);

        // if buffer does not contain object 1 and 2, consumer AB cannot consume
        if (A == 0 && B == 0)
        {
            printf("Buffer does not contain object 1 or 2, consumer AB cannot consume...\n\n");

            // unlock mutex
            unlock_mutex();

            // sleep for 5-15 seconds before next iteration
            delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
            continue;
        }

        // if buffer contains object 1 but not object 2, consumer AB can only consume object 1
        if (A != 0 && B == 0)
        {
            printf("Buffer does not contain object 2...\n");

            consume(1);

            printf("Consumer AB has consumed a 1...\n");
            printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

            // unlock mutex
            unlock_mutex();

            // sleep for 5-15 seconds before next iteration
            delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
            continue;
        }

        // if buffer contains object 2 but not object 1, consumer AB can only consume object 2
        if (A == 0 && B != 0)
        {
            printf("Buffer does not contain object 1...\n");

            consume(2);

            printf("Consumer AB has consumed a 2...\n");
            printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

            // unlock mutex
            unlock_mutex();

            // sleep for 5-15 seconds before next iteration
            delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
            continue;
        }

        // if buffer contains both object 1 and 2, consumer AB can consume both
        printf("Buffer contains both objects 1 and 2...\n");

        consume(1);
        consume(2);

        printf("Consumer AB has consumed a 1 and a 2...\n");
        printf("Updated Full: %d ; Updated Empty: %d\n\n", full, empty);

        // unlock mutex
        unlock_mutex();

        // sleep for 5-15 seconds before next iteration
        delay(CONSUMER_SLEEP_LOW, CONSUMER_SLEEP_HIGH);
    }
}

/**main function**/

int main()
{
    // initialize buffer
    initializeBuffer();

    // seed random number generator
    srand(time(NULL));

    // declare threads and functions
    pthread_t threads[6];
    void *(*functions[6])(void *) = {producer_A, producer_B, consumer_A, consumer_B, consumer_AB, buffer_Info};

    // Create threads using a loop
    for (int i = 0; i < 6; i++)
    {
        int result = pthread_create(&threads[i], NULL, functions[i], NULL);

        // handle error if thread creation fails
        if (result != 0)
        {
            fprintf(stderr, "Failed to create thread: %s\n", strerror(result));
            exit(EXIT_FAILURE);
        }
    }

    // main thread run for 45 seconds then terminate
    sleep(45);

    // cancel threads using a loop
    for (int i = 0; i < 6; i++)
    {
        pthread_cancel(threads[i]);
    }

    // destroy mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}