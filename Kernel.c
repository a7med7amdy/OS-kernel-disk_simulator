#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>

struct Instruction
{
    long mtype;
    char operation;
    char text[64];
};
struct fromDisk
{
    long mtype;
    int freeSpace;
};
struct Instruction Instruction_Q;
struct fromDisk fromDisk_Q;
key_t KeyFromDisk;
key_t KeyToDisk;
key_t KeyFromProcess;
int aliveProcesses;
pid_t diskID;
struct msqid_ds buf;
int num_messages;
int finishTime;
int clock = 0;
void terminate()
{
    pid_t pid = wait(NULL);
    if (diskID != pid)
    {
        aliveProcesses--;
    }
}

int main(int argc, char *argv[])
{
    // will put num of process from arg
    int temp;
    sscanf(argv[1], "%d", &temp);
    int numProcess=temp;
    /////////////////////////////
    signal(SIGUSR2,SIG_IGN);
    KeyFromDisk = msgget(IPC_PRIVATE, 0644);
    KeyToDisk = msgget(IPC_PRIVATE, 0644);
    KeyFromProcess = msgget(IPC_PRIVATE, 0644);
    if (KeyFromDisk == -1 || KeyToDisk == -1 || KeyFromProcess == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    // Disk intialization
    diskID = fork();
    if (diskID == 0)
    {
        char temp[10],temp2[10];
        sprintf(temp, "%d",KeyFromDisk );
        sprintf(temp2, "%d",KeyToDisk );
        execl("Disk", "Disk", temp, temp2, NULL);
    }
    else
    {
        // intialising processes
        for (int i = 0; i < numProcess; i++)
        {
            int pid = fork();
            if (pid == 0)
            {
                char temp[10],temp2[10];
                sprintf(temp, "%d", i);
                sprintf(temp2, "%d",KeyFromProcess );
                execl("process", "process", temp2, temp, NULL);
                // if it doesn't work we should break
            }
        }
    }

    // we have to log everything

    msgctl(KeyFromProcess, IPC_STAT, &buf);
    num_messages = buf.msg_qnum;

    while (aliveProcesses > 0 || num_messages > 0)
    {
        // check if disk is available
        if (clock >= finishTime)
        {
            // check if queue from process is not empty
            if (num_messages > 0)
            {
                int rec_val;
                rec_val = msgrcv(KeyFromProcess, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), 0, !IPC_NOWAIT);
                if (rec_val == -1)
                    perror("Error in receive");
                if (Instruction_Q.operation == 'A')
                {
                    // send signal to check from size
                    kill(diskID, SIGUSR1);
                    rec_val = msgrcv(KeyFromDisk, &fromDisk_Q, sizeof(fromDisk_Q) - sizeof(fromDisk_Q.mtype), 0, !IPC_NOWAIT);
                    if (rec_val == -1)
                        perror("Error in receive");
                    if (fromDisk_Q.freeSpace > 0)
                    {
                        //sending ADD instruction
                        int send_val = msgsnd(KeyToDisk, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), !IPC_NOWAIT);
                        if (send_val == -1)
                            perror("Errror in send");
                        finishTime = clock + 3;
                        printf("\n executing ADD operation sent to disk starting at %d\n", clock);
                    }
                    else
                        printf("\nCannot execute ADD operation at %d\n", clock);
                }
                else
                {
                    //sending delete instruction
                    int send_val = msgsnd(KeyToDisk, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), !IPC_NOWAIT);
                    if (send_val == -1)
                        perror("Errror in send");
                    finishTime = clock + 1;
                    printf("\n executing delete operation sent to disk starting at %d\n", clock);
                }
            }
        }
        clock++;
        killpg(getpgrp(), SIGUSR2);
        msgctl(KeyFromProcess, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
    }
    kill(SIGKILL, diskID);
    printf("\n disk is terminating at %d\n", clock);
    printf("\n kernel is terminating at %d\n", clock);
}
