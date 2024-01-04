#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{
    pid_t child;
    int status;

    // create child
    child = fork();
    if (child == -1)
    {
        perror("Error when creating child process");
        exit(1);
    }

    // for child process
    if (child == 0)
    {
        pid_t pid_child, ppid_child;
        pid_child = getpid();
        ppid_child = getppid();
        printf("The child process with pid %d, its parent's pid is %d\n", pid_child, ppid_child);
        exit(0);
    }

    // for parent process
    else
    {
        pid_t pid_parent;
        pid_parent = getpid();
        printf("The parent process with pid %d\n", pid_parent);
        exit(0);
    }
}