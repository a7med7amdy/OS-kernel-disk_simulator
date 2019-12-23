#include <iostream>
#include<fstream>
#include<ostream>
#include <string>
#include<vector>
#include <algorithm>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
//#include <signal.h>
#include<sstream>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
using namespace std;
struct Instruction
{
    long mtype;
    char operation;
    string text;
    // Instruction(){
    //     mtype=0;
    // }
};
struct fromDisk
{
    long mtype;
    int freeSpace;
    // fromDisk(){
    //     mtype=0;
    // }
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
int clkMain = 0;
void terminates(int dummy)
{
    pid_t pid = wait(NULL);
    cout<<"proess"<<endl;
    if (diskID != pid)
    {
        cout<<"disk"<<endl;
        aliveProcesses--;
    }
       // signal(SIGCHLD,terminates);
}
void foralarm(int dummy){
    alarm(0);
    clkMain++;
    killpg(getpgrp(), SIGUSR2);

    alarm(1);
    //signal(SIGALRM,foralarm);
}
int main(int argc, char **argv)
{
    if(argc<2){
        cout<<"error in entering";
        return 0;
    }
    // will put num of process from arg
    string var1=argv[1];
    int numProcess=stoi(var1);
    aliveProcesses=numProcess;
    /////////////////////////////
    signal(SIGALRM,foralarm);
    signal(SIGUSR2,SIG_IGN);
    signal(SIGUSR1,SIG_IGN);
    signal(SIGCHLD,terminates);
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
       /* sprintf(temp, "%d",KeyFromDisk );
        sprintf(temp2, "%d",KeyToDisk );*/
        string temp,temp2;
        temp=to_string(KeyFromDisk);
        temp2=to_string(KeyToDisk);

        char sent1[temp.size()+1];
        char sent2[temp2.size()+1];
        strcpy(sent1,temp.c_str());
        strcpy(sent2,temp2.c_str());
        execl("Disk",sent1, sent2,(char*)0);
    }
    else
    {
        // intialising processes
        for (int i = 0; i < numProcess; i++)
        {
            int pid = fork();
            if (pid == 0)
            {

                // char cstr[s.size() + 1];
	            // strcpy(cstr, s.c_str());
                string temp,temp2;
                temp=to_string(i);
                temp2=to_string(KeyFromProcess);
                char sent1[temp.size()+1];
                char sent2[temp2.size()+1];
                strcpy(sent1,temp.c_str());
                strcpy(sent2,temp2.c_str());
                execl("process",sent1, sent2,(char*)0);
                // if it doesn't work we should break
            }
        }
    }
    alarm(1);
    // we have to log everything

    msgctl(KeyFromProcess, IPC_STAT, &buf);
    num_messages = buf.msg_qnum;
    msgctl(KeyToDisk, IPC_STAT, &buf);
    int num_disk_size=buf.msg_qnum;
    
    while ((aliveProcesses > 0 || num_messages > 0)||(num_disk_size>0)||(clkMain<finishTime))
    {
        // check if disk is available
        if (clkMain >= finishTime)
        {
            // check if queue from process is not empty
            if (num_messages > 0)
            {
                int rec_val;
                rec_val = msgrcv(KeyFromProcess, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), 0, !IPC_NOWAIT);
                if (rec_val == -1)
                    perror("Error in receive in kernel 1");
                if (Instruction_Q.operation == 'A')
                {
                    // send signal to check from size
                    kill(diskID, SIGUSR1);
                    rec_val = msgrcv(KeyFromDisk, &fromDisk_Q, sizeof(fromDisk_Q) - sizeof(fromDisk_Q.mtype), 0, !IPC_NOWAIT);
                    if (rec_val == -1)
                        perror("Error in receive in kernel 2");
                    if (fromDisk_Q.freeSpace > 0)
                    {
                        //Instruction_Q.mtype=40;
                        //sending ADD instruction

                        int send_val = msgsnd(KeyToDisk, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), !IPC_NOWAIT);
                        if (send_val == -1)
                            perror("Errror in send to addd");
                        finishTime = clkMain + 3;
                        printf("\n executing ADD operation sent to disk starting at %d\n", clkMain);
                    }
                    else
                        printf("\nCannot execute ADD operation at %d\n", clkMain);
                }
                else
                {
                    Instruction_Q.mtype=40;
                    //sending delete instruction
                    int send_val = msgsnd(KeyToDisk, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), !IPC_NOWAIT);
                    if (send_val == -1)
                        perror("Errror in send to delete");
                    finishTime = clkMain + 1;
                    printf("\n executing delete operation sent to disk starting at %d\n", clkMain);
                }
            }
        }
        msgctl(KeyFromProcess, IPC_STAT, &buf);
        num_messages = buf.msg_qnum;
        msgctl(KeyToDisk, IPC_STAT, &buf);
        num_disk_size = buf.msg_qnum;
        // last process to die sent after I check ****
    }
    //kill(SIGKILL, diskID);
    printf("\n disk is terminating at %d\n", clkMain);
    printf("\n kernel is terminating at %d\n", clkMain);
}
