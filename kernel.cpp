#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unordered_map>
using namespace std;
struct Instruction {
    long mtype;
    char operation;
    char text[64];
};
struct fromDisk {
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
int clkMain = 0;
char beingserved = 'Z';
unordered_map<int, int> childrenPid;
void terminates(int dummy)
{
    int status;
    pid_t pid = wait(NULL);
    if (pid != diskID) {
        aliveProcesses--;
        cout << "PROCESS NUM " << childrenPid[pid] + 1 << " TERMINATES" << endl;
    }
}

void foralarm(int dummy)
{
    clkMain++;
    cout << "\t\t\t\tTime now is " << clkMain << endl;
    killpg(getpgrp(), SIGUSR2);
    alarm(1);
    //signal(SIGALRM,foralarm);
}
int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "error in entering";
        return 0;
    }
    // will put num of process from arg
    string var1 = argv[1];
    int numProcess = stoi(var1);
    aliveProcesses = numProcess;
    /////////////////////////////
    signal(SIGALRM, foralarm);
    signal(SIGUSR2, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGCHLD, terminates);
    KeyFromDisk = msgget(IPC_PRIVATE, 0644);
    KeyToDisk = msgget(IPC_PRIVATE, 0644);
    KeyFromProcess = msgget(IPC_PRIVATE, 0644);
    if (KeyFromDisk == -1 || KeyToDisk == -1 || KeyFromProcess == -1) {
        perror("Error in create");
        exit(-1);
    }
    // Disk intialization
    diskID = fork();
    if (diskID == 0) {
        string temp, temp2;
        temp = to_string(KeyFromDisk);
        temp2 = to_string(KeyToDisk);
        char sent1[temp.size() + 1];
        char sent2[temp2.size() + 1];
        strcpy(sent1, temp.c_str());
        strcpy(sent2, temp2.c_str());
        execl("disk", sent1, sent2, (char*)0);
    }
    else {
        // intialising processes
        for (int i = 0; i < numProcess; i++) {
            pid_t pid = fork();
            if (pid == 0) {
                string temp, temp2;
                temp = to_string(i);
                temp2 = to_string(KeyFromProcess);
                char sent1[temp.size() + 1];
                char sent2[temp2.size() + 1];
                strcpy(sent1, temp.c_str());
                strcpy(sent2, temp2.c_str());
                execl("process", sent1, sent2, (char*)0);
            }
            childrenPid[pid] = i;
        }
    }

    // we have to log everything

    msgctl(KeyFromProcess, IPC_STAT, &buf);
    num_messages = buf.msg_qnum;
    msgctl(KeyToDisk, IPC_STAT, &buf);
    int num_disk_size = buf.msg_qnum;
    alarm(1);
    finishTime = -1;
    bool exits = false;
    // indicator for add and delete
    while (aliveProcesses > 0 && beingserved == 'Z') {
        while (1) {
            int rec_val;
            if (clkMain == finishTime) {
                if (beingserved == 'A')
                    cout << "Adding Operation ends at Time " << clkMain << endl;
                else if (beingserved == 'D')
                    cout << "Deleting Operation ends at Time " << clkMain << endl;
                beingserved = 'Z';
            }
            if (clkMain < finishTime)
                continue;
            // terminating condition
            if (aliveProcesses <= 0 && clkMain >= finishTime) {
                rec_val = msgrcv(KeyFromProcess, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), 1, IPC_NOWAIT);
                if (rec_val < 0) {
                    break;
                }
                else
                    exits = true;
            }
            if (!exits)
                rec_val = msgrcv(KeyFromProcess, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), 1, IPC_NOWAIT);
            if (rec_val > 0) {
                alarm(0);
                if (Instruction_Q.operation == 'A') {
                    // send signal to check from size
                    kill(diskID, SIGUSR1);
                    rec_val = msgrcv(KeyFromDisk, &fromDisk_Q, sizeof(fromDisk_Q) - sizeof(fromDisk_Q.mtype), 1, !IPC_NOWAIT);
                    if (rec_val == -1)
                        perror("Error in receive in kernel");
                    if (fromDisk_Q.freeSpace > 0) {
                        //sending ADD instruction
                        int send_val = msgsnd(KeyToDisk, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), !IPC_NOWAIT);
                        beingserved = 'A';
                        cout << "Add operation sent to disk to be served at time " << clkMain << endl;
                        if (send_val == -1)
                            perror("Errror in send to addd");
                        finishTime = clkMain + 3;
                    }
                }
                else {
                    //sending delete instruction
                    int send_val = msgsnd(KeyToDisk, &Instruction_Q, sizeof(Instruction_Q) - sizeof(Instruction_Q.mtype), !IPC_NOWAIT);
                    if (send_val == -1)
                        perror("Errror in send to delete");
                    beingserved = 'D';
                    cout << "Delete operation sent to disk to be served at time " << clkMain << endl;
                    finishTime = clkMain + 1;
                }
                alarm(1);
            }
        }
    }
    kill(diskID, SIGKILL);
    cout << "disk is terminating at " << clkMain << endl;
    cout << "kernel is terminating at " << clkMain << endl;
}
