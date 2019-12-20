#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
struct fromKernel
{
    long mtype;
    char operation;
    char text[64];
};
struct toKernel
{
    long mtype;
    int freeSpace;
};
int clock = 0;
char storge[10][64];
bool isFree[10];
key_t keyFromKernel;
key_t keyToKernel;
struct toKernel toKernel_Q;
struct fromKernel fromKernel_Q;

void Add(char *message)
{
    for (int i = 0; i < 10; i++)
    {
        if (isFree[i])
        {
            isFree[i] = false;
            toKernel_Q.freeSpace--;
            strcpy(storge[i], message);
            break;
        }
    }
}
void Remove(int ID)
{
    isFree[ID] = true;
    toKernel_Q.freeSpace++;
    memset(storge[ID], ' ', 64 * sizeof(char));
}

void clockInc() // handler for siguser2 signal
{
    clock++;
}
void countFree()
{
    int send_val;
    // IPC_NOWAIT (Queue is full return)
    send_val = msgsnd(keyToKernel, &toKernel_Q, sizeof(toKernel_Q.freeSpace), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send");
}

int main(int argc, char *argv[])
{
    int temp;
    sscanf(argv[1], "%d", &temp);
    keyToKernel = temp;
    sscanf(argv[2], "%d", &temp);
    keyFromKernel= temp;

    memset(isFree, true, 10 * sizeof(bool));
    signal(SIGUSR2, clockInc);
    signal(SIGUSR1, countFree);
    while (1)
    {
        struct msqid_ds buf;
        msgctl(keyFromKernel, IPC_STAT, &buf);
        int num_messages = buf.msg_qnum;
        if (num_messages > 0)
        {
            int rec_val;
            rec_val = msgrcv(keyFromKernel, &fromKernel_Q, sizeof(fromKernel_Q) - sizeof(fromKernel_Q.mtype), 0, !IPC_NOWAIT);
            if (rec_val == -1)
                perror("Error in receive");
            if (fromKernel_Q.operation == 'A')
            {
                Add(fromKernel_Q.text);
            }
            else
            {
                int temp;
                sscanf(fromKernel_Q.text, "%d", &temp);
                Remove(temp);
            }
        }
        else
            pause();
    }
}