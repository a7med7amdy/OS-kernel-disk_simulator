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

struct toKernel
{
    long mtype;
    char operation;
    char text[64];
};
struct job
{
    int clock;
    char operation;
    char text[64];
};
int size = 0;
int clock = 0;
key_t KeyToKernel;
char process_number[10];
struct job jobs[100];
struct toKernel toKernel_Q;


int comparator(const void *p, const void *q)
{
    return ((struct job *)p)->clock < ((struct job *)q)->clock;
}

void read_all_instructions(const char *file_name)
{
    FILE *file = fopen(file_name, "r");
    if (file == NULL)
    {
        printf("Cannot Open File");
        perror("program");
    }
    int i = 0;
    while (fscanf(file, "%d", &jobs[i].clock))
    {
        char buf[70];
        fgets(buf, sizeof(buf), file);

        //We need to Check if file pointer is increented automatically
        jobs[i].operation = buf[0];
        strcpy(jobs[i].text, buf + 1);
        i++;
    }
    size = i;
    fclose(file);
}
void sortJobs()
{
    qsort(jobs, size, sizeof(struct job), comparator);
}

void clockInc() // handler for siguser2 signal
{
    clock++;
}

int main(int argc, char *argv[])
{
    signal(SIGUSR2, clockInc);
    int temp, i = 0;
    sscanf(argv[1], "%d", &temp);
    KeyToKernel = temp;
    strcpy(process_number,argv[2]);

    read_all_instructions(strcat("process ", process_number));
    sortJobs();

    while (size > 0)
    {
        if (clock == jobs[i].clock)
        {
            int send_val = msgsnd(KeyToKernel, &toKernel_Q, sizeof(toKernel_Q) - sizeof(toKernel_Q.mtype), !IPC_NOWAIT);
            if (send_val == -1)
                perror("Errror in send");
        size--;
        i++;
        }
        else
        pause();
    }
    return 0;
}