#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <ctype.h>
#include <semaphore.h>

#define MAX_MSG_SIZE 100

// helper function to convert message to upper case
void to_uppercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = toupper((unsigned char)str[i]);
    }
}

struct msg
{
    long int msg_type;
    char msg_text[MAX_MSG_SIZE];
} message;

int main()
{

    char original_message[] = "hello world";

    // distinguish between message to child and parent
    long int msg_to_child = 1;
    long int msg_to_parent = 2;

    // initialize message queue
    key_t key;
    int msg_id;

    key = ftok("q2n", 1);

    msg_id = msgget(key, 0666 | IPC_CREAT);
    if (msg_id == -1)
    {
        perror("msgget");
        return -1;
    }

    // create child
    pid_t child_pid;
    if ((child_pid = fork()) == -1)
    {
        perror("Failed to create child process");
        exit(1);
    }

    // for child
    if (child_pid == 0)
    {
        // read message from parent
        if (msgrcv(msg_id, &message, sizeof(message), msg_to_child, 0) == -1)
        {
            perror("msgrcv");
            exit(1);
        }
        printf("Data received from message queue by child: %s\n", message.msg_text);

        // convert message to upper case
        to_uppercase(message.msg_text);

        // send message back to parent
        message.msg_type = msg_to_parent;
        printf("Data sent to message queue by child: %s\n", message.msg_text);
        if (msgsnd(msg_id, &message, sizeof(message), 0) == -1)
        {
            perror("msgsnd");
            exit(1);
        }
        exit(0);
    }

    // for parent
    else
    {
        // send original message to child
        strncpy(message.msg_text, original_message, MAX_MSG_SIZE);
        message.msg_type = msg_to_child;
        if (msgsnd(msg_id, &message, sizeof(message), 0) == -1)
        {
            perror("msgsnd");
        }
        else
        {
            printf("Data sent to message queue by parent: %s\n", message.msg_text);
        }

        // receive converted message from child
        msgrcv(msg_id, &message, sizeof(message), msg_to_parent, 0);
        printf("Data received from message queue by parent: %s\n", message.msg_text);

        // destroy message queue
        msgctl(msg_id, IPC_RMID, 0);
    }
}