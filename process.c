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
char file_name[15]="process0.txt";
struct job jobs[100];
struct toKernel toKernel_Q;


int comparator(const void *p, const void *q)
{
   // return ((struct job *)p)->clock < ((struct job *)q)->clock;
   return (*(struct job *)p->clock -*(struct job *)q->clock);
   //return (*(struct job *)p-*(struct*)q);
}

void read_all_instructions(const char *file_name)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen("process0.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int j=0;
    while ((read = getline(&line, &len, fp)) != -1) {
       /* int x=-1;
        int i;
        j+=1;
        int temp;
        char tmp[20];
        for( i=0;i<read;i++)
        {
            if(line[i]=='A'||line[i]=='D')
            {
                break;
            }
            else
            tmp[i]=line[i];
        }
        printf("%d",i);
        sscanf(line,"%d",temp);*/
        //printf("%d",temp);
       /* for(i=0;i<read;++i){
            if(line[i]-'0'>=0 && line[i]-'0'<=9){
                if(x==-1){
                  x=line[i]-'0';
                }
                else
                {
                    x*=10;
                    x+=line[i];
                }
            } 
            else if(line[i]=='A' || line[i]=='D'){
                jobs[j].operation=line[i];
                break;
            }
        }
        for(int k=i;k<read;++k){
                /*if(!(line[k]==' '))
                    strcat(jobs[j].text,line[k]);*/
           // }
         /*jobs[j].clock=x;
         */
    }

    fclose(fp);
    size = j;
   /* for(int l=0;l<j;++l){
        printf("%d/n",jobs[l].clock);
        printf("%c/n",jobs[l].operation);
        //printf("%s/n",jobs[l].text);
    }*/

}
void sortJobs()
{
    qsort((void*)jobs, size, sizeof(struct job), comparator);
}

void clockInc() // handler for siguser2 signal
{
    clock++;
}

//int main(/*int argc, char *argv[]*/)
int main()
{
 //   printf("here %d",1);
    signal(SIGUSR2, clockInc);
    int temp, i = 0;
    // sscanf(argv[1], "%d", &temp);
    // KeyToKernel = temp;
    // strcpy(process_number,argv[2]);

    // strcat(file_name,process_number);
    // strcat(file_name,".txt");
   // read_all_instructions(file_name);
   jobs[0].clock=2;
   jobs[1].clock=1;
   jobs[2].clock=3;
   jobs[0].operation='A';
   jobs[1].operation='A';
   jobs[2].operation='D';
    strcpy(jobs[0].text, "im ahmed");
    strcpy(jobs[1].text, "im asccc");
    strcpy(jobs[2].text, "im ahvfv");
    sortJobs();
    printf("clock %d",jobs[1].clock);
    printf("operation %d",jobs[1].operation);
    printf("tect %s",jobs[1].text);
    while (size > 0)
    {
        if (clock == jobs[i].clock)
        {
            //int send_val = msgsnd(KeyToKernel, &toKernel_Q, sizeof(toKernel_Q) - sizeof(toKernel_Q.mtype), !IPC_NOWAIT);
            //if (send_val == -1)
            //    perror("Errror in send");
            // printf("clock %d",jobs[i].clock);
            // printf("operation %d",jobs[i].operation);
            // printf("tect %s",jobs[i].text);
        size--;
        i++;
        }
        else
        pause();
    }
    return 0;
}