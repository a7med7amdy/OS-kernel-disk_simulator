#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
using namespace std;
 
struct fromKernel
{
    long mtype;
    char operation;
    string text;
};
struct toKernel
{
    long mtype;
    int freeSpace;
};
int clk = 0;
string storge[10];
bool isFree[10];
key_t keyFromKernel;
key_t keyToKernel;
struct toKernel toKernel_Q;
struct fromKernel fromKernel_Q;
 
 
void Add(string message)
{
    for (int i = 0; i < 10; i++)
    {
        if (!isFree[i])
        {
            isFree[i] = false;
            toKernel_Q.freeSpace--;
            storge[i]=message;
            break;
        }
    }
}
 
void Remove(int ID)
{
    isFree[ID] = true;
    toKernel_Q.freeSpace++;
    storge[ID]=' ';
}
 
void clockInc(int dummy) // handler for siguser2 signal
{
    clk++;
}
 
void countFree(int dummy)
{
    int send_val;
    // IPC_NOWAIT (Queue is full return)
    send_val = msgsnd(keyToKernel, &toKernel_Q, sizeof(toKernel_Q.freeSpace), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send");
}
 
 
int main()
{
    keyToKernel = 6;
    keyFromKernel= 5;
    signal(SIGUSR2, clockInc);
    signal(SIGUSR1, countFree);
    int msqid = msgget(IPC_PRIVATE, 0644);
    // fromKernel_Q.text = "1231231";
    // fromKernel_Q.operation='A';
    // fromKernel_Q.mtype=40;
    // int rec_val = msgsnd(msqid, &fromKernel_Q, sizeof(fromKernel_Q) - sizeof(fromKernel_Q.mtype), !IPC_NOWAIT);
    // if (rec_val == -1)
    //     cout<<"LA222222222222A"<<endl;
 
    struct fromKernel mohamed;
 
    int rec_val = msgrcv(msqid, &mohamed, sizeof(fromKernel_Q) - sizeof(fromKernel_Q.mtype), 0, IPC_NOWAIT);
    cout<<mohamed.text<<endl;
    struct msqid_ds buf;
    msgctl(msqid, IPC_STAT, &buf);
    int num_messages = buf.msg_qnum;
    cout<<num_messages<<endl;
   
 }
 
 