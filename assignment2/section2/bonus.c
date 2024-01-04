#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#define NUM_ASTRONOMERS 10
#define NUM_ASYMMETRIC 3 // Number of asymmetric astronomers
#define NUM_GREEDY 3     // Number of greedy astronomers

#define AVG_EAT_TIME 1
#define MAX_WAIT_TIME 2

#define THINK_TIME 2
#define ASYM_TIME_GAP 1

int ordering[NUM_ASTRONOMERS]; // 0 means symmetric, 1 means asymmetric

// In case of using monitor, you can define it here and manipulate
// the code accordingly.
// typedef struct {
// ...

// States of astonomers
typedef enum
{
    THINKING,
    HUNGRY,
    // Asymmetric astronomers holding the right chopstick
    RIGHT_GRABBING,
    // Asymmetric astronomers holding the right chopstick after a period of time(2s) and is ready to release
    RIGHT_GRABBING_RELEASE,
    EATING,
} State;

// Syncronization structure associated with each astronomer
typedef struct
{
    // State of astronomer
    State state;

    // Semaphore for temporarily blocking the astronomer
    sem_t sem;

    // Count of astronomer blocking behind the semaphore, either 0 or 1
    int count;
} Astronomer;

// Semaphore mutex and next for blocking threads when needed
sem_t mutex, next;
int next_count = 0;

// Array keeping sychronization structures of astronomers
Astronomer astronomers[NUM_ASTRONOMERS];

// Array keeping track of the availibility of chopsticks; 1 is available, 0 is not available;
int chopsticksAvail[NUM_ASTRONOMERS];

// Flag for indicating greedy status for greedy astronomers
// When flag is 1, eat for a second time without putdown/pickup the chopsticks
int greedy[NUM_ASTRONOMERS];

// Helper function for printing the state of astronomer
const char *get_state_name(State state)
{
    switch (state)
    {
    case THINKING:
        return "THINKING";
    case HUNGRY:
        return "HUNGRY";
    case RIGHT_GRABBING:
        return "RIGHT";
    case RIGHT_GRABBING_RELEASE:
        return "RIGHT_TO_R";
    case EATING:
        return "EATING";
    default:
        return "UNKNOWN";
    }
}

void print_astronomers_type()
{
    printf("Type of astronomers:\n");
    printf("(0 - symmetric, 1 - asymmetric, 2 - greedy)\n");
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        printf("%-10d\t", i);
    }
    printf("\n");
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        printf("%-10d\t", ordering[i]);
    }

    printf("\n\n");
}

// Helper function for printing astronomers/chopsticks status as a table
void print_status()
{
    printf("Astronomers state:\n");
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        printf("%-10d\t", i);
    }
    printf("\n");
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        printf("%-10s\t", get_state_name(astronomers[i].state));
    }
    printf("\n");
    printf("Chopsticks availability(1 - yes, 0 - no):\n");
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        printf("%-10d\t", i);
    }
    printf("\n");
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        printf("%-10d\t", chopsticksAvail[i]);
    }
    printf("\n\n");
}

// Wake up a blocked astronomer for eating
void signal(int i)
{
    // If the astronomer is waiting behind its semaphore
    if (astronomers[i].count > 0)
    {
        next_count++;
        // Signal the astronomer's semaphore
        sem_post(&astronomers[i].sem);
        sem_wait(&next);
        next_count--;
    }
}

// Symmetric/Greedy astronomers try to pick up both chopsticks and eat
void test(int i)
{
    // Check availability of chopsticks
    // - its left neighbour should not be in the state EATING/RIGHT_GRABBING/RIGHT_GRABBING_RELEASE
    // - its right neighbour should not be EATING
    // The astronomer itself should be HUNGRY
    if ((astronomers[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS].state != EATING) &&
        (astronomers[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS].state != RIGHT_GRABBING) &&
        (astronomers[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS].state != RIGHT_GRABBING_RELEASE) &&
        (astronomers[i].state == HUNGRY) &&
        (astronomers[(i + 1) % NUM_ASTRONOMERS].state != EATING))
    {
        // Set state of current astronomer to EATING
        astronomers[i].state = EATING;
        chopsticksAvail[i] = 0;
        chopsticksAvail[(i + 1) % NUM_ASTRONOMERS] = 0;
        if (ordering[i] == 0)
        {
            printf("Symmetric astronomer #%d picks up both chopsticks and starts to eat\n", i);
        }
        else
        {
            printf("Greedy astronomer #%d picks up both chopsticks and starts to eat\n", i);
        }
        print_status();

        // Does not do anything during pickup()
        signal(i);
    }
}

// Asymmetric astronomer attempts to pick up right chopstick
void testRight(int i)
{
    // The right neighbour should not be EATING
    // The astronomer should be HUNGRY
    if (astronomers[(i + 1) % NUM_ASTRONOMERS].state != EATING &&
        astronomers[i].state == HUNGRY)
    {
        astronomers[i].state = RIGHT_GRABBING;
        chopsticksAvail[(i + 1) % NUM_ASTRONOMERS] = 0;
        printf("Asymmetric astronomer #%d picks up right chopstick\n", i);
        print_status();
        signal(i);
    }
}

// Asymmetric astronomer holding right chopstick attempts to pick up left one
void testLeft(int i)
{
    // The left neighbour should not be EATING/RIGHT_GRABBING/RIGHT_GRABBING_RELEASE
    // The astronomer should be RIGHT_GRABBING
    if (astronomers[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS].state != EATING &&
        astronomers[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS].state != RIGHT_GRABBING &&
        astronomers[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS].state != RIGHT_GRABBING_RELEASE &&
        astronomers[i].state == RIGHT_GRABBING)
    {
        astronomers[i].state = EATING;
        chopsticksAvail[i] = 0;
        printf("Asymmetric astronomer #%d picks up left chopstick and starts to eat\n", i);
        print_status();
    }
}

// Block an astronomer when not being able to pick up both/right chopstick(s)
void wait(int i)
{
    astronomers[i].count++;
    if (next_count > 0)
    {
        sem_post(&next);
    }
    else
    {
        sem_post(&mutex);
    }
    sem_wait(&astronomers[i].sem);
    astronomers[i].count--;
}

// Block an asymmetric astronomer holding right chopstick when not able to pick up left chopstick immediately
int wait_left(int i)
{
    if (next_count > 0)
    {
        sem_post(&next);
    }
    else
    {
        sem_post(&mutex);
    }

    // set up a timer for 2s
    time_t startTime = time(NULL);

    // while loop that runs at most 2s
    while (1)
    {
        // break the loop if successfully pick up left chopstick and start eating
        if (astronomers[i].state == EATING)
        {
            break;
        }

        // break the loop if 2s has passed and still cannot get the left chopstick
        if (difftime(time(NULL), startTime) >= 2)
        {
            break;
        }
    }
}

// Greedy astronomers pick up chopsticks
void pickup_greedy(int i)
{
    sem_wait(&mutex);

    // if greedy flag is set to 1, eat for the second time without pick up again
    if (greedy[i] == 1)
    {
        printf("Greedy astronomer #%d eats for the second time\n", i);
        print_status();

        // set the greedy flag back to 0 for puting down the chopsticks after eating two times
        greedy[i] = 0;
    }

    // if greedy flag is 0, try picking up chopsticks and eat for the first time
    else
    {
        astronomers[i].state = HUNGRY;
        test(i);
        if (astronomers[i].state != EATING)
        {
            wait(i);
        }

        // set the greedy flag to 0 for eating the second time later
        greedy[i] = 1;
    }
    // If processes are waiting to enter the monitor
    if (next_count > 0)
    {
        // Signal the next process to come into the monitor
        sem_post(&next);
    }
    else
    {
        // Change mutex to original state
        sem_post(&mutex);
    }
}

// Symmetric astronomer picks up both chopsticks
void pickup(int i)
{
    sem_wait(&mutex);
    astronomers[i].state = HUNGRY;
    test(i);
    if (astronomers[i].state != EATING)
    {
        wait(i);
    }
    // If processes are waiting to enter the monitor
    if (next_count > 0)
    {
        // Signal the next process to come into the monitor
        sem_post(&next);
    }
    else
    {
        // Change mutex to original state
        sem_post(&mutex);
    }
}

// Asymmetric astronomer picks up right chopstick
void pickup_right(int i)
{
    sem_wait(&mutex);
    astronomers[i].state = HUNGRY;
    testRight(i);
    if (astronomers[i].state != RIGHT_GRABBING)
    {
        wait(i);
    }
    if (next_count > 0)
    {
        // Signal the next process to come into the monitor
        sem_post(&next);
    }
    else
    {
        // Change mutex to original state
        sem_post(&mutex);
    }
}

// Asymmetric astronomer holding the right chopstick picks up the left chopstick
void pickup_left(int i)
{
    sem_wait(&mutex);
    testLeft(i);
    if (astronomers[i].state != EATING)
    {
        wait_left(i);
    }
    else
    {
        if (next_count > 0)
        {
            sem_post(&next);
        }
        else
        {
            sem_post(&mutex);
        }
    }
}

// Asymmetric astronomer puts down right chopstick after waiting time if not being able to get the left one
void putdown_right(int i)
{
    sem_wait(&mutex);
    astronomers[i].state = THINKING;
    chopsticksAvail[(i + 1) % NUM_ASTRONOMERS] = 1;
    printf("Asymmetric astronomer #%d put down right chopsticks because he/she cannot pick up the left one in 2s\n", i);
    print_status();

    // if the right neighbour is symmetric /greedy
    if (ordering[(i + 1) % NUM_ASTRONOMERS] == 0 || ordering[(i + 1) % NUM_ASTRONOMERS] == 2)
    {
        test((i + 1) % NUM_ASTRONOMERS);
    }

    // if the right neighbour is asymmetric
    else
    {
        testLeft((i + 1) % NUM_ASTRONOMERS);
    }
    sem_post(&mutex);
}

// Astronomer puts down chopstick after eating
void putdown(int i)
{

    // Secure Critical Section
    sem_wait(&mutex);

    // Set philosopher back to resting state (THINKING)
    astronomers[i].state = THINKING;
    chopsticksAvail[i] = 1;
    chopsticksAvail[(i + 1) % NUM_ASTRONOMERS] = 1;
    if (ordering[i] == 0)
    {
        printf("Symmetric astronomer #%d put down both chopsticks\n", i);
    }
    else if (ordering[i] == 1)
    {
        printf("Asymmetric astronomer #%d put down both chopsticks\n", i);
    }
    else
    {
        printf("Greedy astronomer #%d put down both chopsticks\n", i);
    }
    print_status();

    // Check neighbours
    // If left neighbour is symmetric or greedy
    if (ordering[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS] == 0 || ordering[(i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS] == 2)
    {

        test((i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS);
    }

    // If left neighbour is asymmetric
    else
    {
        testRight((i + NUM_ASTRONOMERS - 1) % NUM_ASTRONOMERS);
    }

    // If right neighbour is symmetric or greedy
    if (ordering[(i + 1) % NUM_ASTRONOMERS] == 0 || ordering[(i + 1) % NUM_ASTRONOMERS] == 2)
    {
        test((i + 1) % NUM_ASTRONOMERS);
    }

    // If right neighbour is asymmetric
    else
    {
        testLeft((i + 1) % NUM_ASTRONOMERS);
    }
    sem_post(&mutex);
}

void think(int i)
{
    // Only putdown if the astronomer is holding the chopsticks and eating
    if (astronomers[i].state == EATING)
    {
        putdown(i);
    }
    sleep(THINK_TIME);
}

void eat(int i)
{
    // random eating time
    int eating_time = rand() % ((2 * AVG_EAT_TIME) + 1);
    if (eating_time == 0)
    {
        eating_time = 1;
    }

    // for symmetric astronomers
    if (ordering[i] == 0)
    {
        pickup(i);
        sleep(eating_time);
    }

    // for greedy astronomers
    else if (ordering[i] == 2)
    {
        pickup_greedy(i);
        sleep(eating_time);
    }

    // for asymmetric astronomers
    else
    {
        pickup_right(i);
        sleep(ASYM_TIME_GAP);
        pickup_left(i);

        // If not eating after pick up left -> astronomer waited but not able to get the left chopstick -> release the chopstick
        if (astronomers[i].state != EATING)
        {
            astronomers[i].state = RIGHT_GRABBING_RELEASE;
            putdown_right(i);
        }
        else
        {
            sleep(eating_time);
        }
    }
}

// template for the philosopher thread
// it is ran by each thread
void *philosopher(void *arg)
{
    int id = *(int *)arg;

    // indefinate loop
    while (1)
    {
        // for greedy astronomers
        if (ordering[id] == 2)
        {
            // think(put down the chopsitcks) only when the greedy flag is set to 0
            if (greedy[id] == 0)
            {
                think(id);
            }
            eat(id);
        }

        // for symmetric/asymmetric astronomers
        else
        {
            think(id);
            eat(id);
        }
    }
}

// a function to generate random ordering of 0 and 1
// 0 means symmetric, 1 means asymmetric
void place_astronomers(int *ordering)
{
    // generate random ordering
    // 0 means symmetric, 1 means asymmetric
    for (int i = 0; i < NUM_ASYMMETRIC; i++)
    {
        ordering[i] = 1;
    }
    for (int i = 0; i < NUM_GREEDY; i++)
    {
        ordering[NUM_ASYMMETRIC + i] = 2;
    }

    for (int i = NUM_ASYMMETRIC + NUM_GREEDY; i < NUM_ASTRONOMERS; i++)
    {
        ordering[i] = 0;
    }

    // shuffle the ordering
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        int j = rand() % NUM_ASTRONOMERS;
        int temp = ordering[i];
        ordering[i] = ordering[j];
        ordering[j] = temp;
    }
}

int main()
{

    // initialize chopsticks
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        chopsticksAvail[i] = 1;
    }

    // initialize philosophers
    srand(time(NULL));

    place_astronomers(ordering);

    // for (int i = 0; i < NUM_ASTRONOMERS; i++)
    // {
    //     printf("%d, %d\n", i, ordering[i]);
    // }

    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        astronomers[i].state = THINKING;
        sem_init(&astronomers[i].sem, 0, 0);
    }

    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        greedy[i] = 0;
    }

    print_astronomers_type();

    if (sem_init(&mutex, 0, 1) == -1)
    {
        perror("sem_init");
        exit(1);
    }
    if (sem_init(&next, 0, 0) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    // start philosophers
    pthread_t threads[NUM_ASTRONOMERS];
    int thread_ids[NUM_ASTRONOMERS];
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, philosopher, &thread_ids[i]) == -1)
        {
            perror("Failed to create thread");
            exit(1);
        }
    }

    // call join on philosophers
    for (int i = 0; i < NUM_ASTRONOMERS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return 0;
}