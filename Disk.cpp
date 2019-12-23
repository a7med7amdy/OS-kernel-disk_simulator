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
bool full[10];
key_t keyFromKernel;
key_t keyToKernel;
struct toKernel toKernel_Q;
struct fromKernel fromKernel_Q;

void solve()
{
    while(1);
}
 
void Add(string message)
{
    
    for (int i = 0; i < 10; i++)
    {
 
        if (!full[i])
        {
            full[i] = true;
            toKernel_Q.freeSpace--;
            storge[i]=message;
            cout<<" about to go out"<<endl;
            break;
        }
    }
    cout<<"ana 5argt"<<endl;
}
 
void Remove(int ID)
{
    if(ID>=0 && ID<=9){
        full[ID] = false;
        toKernel_Q.freeSpace++;
        storge[ID]=' ';
    }
}
 
void clockInc(int dummy) // handler for siguser2 signal
{
    clk++;
    //signal(SIGUSR2,clockInc);
}
 
void countFree(int dummy)
{
    int send_val;
    // IPC_NOWAIT (Queue is full return)
    toKernel_Q.mtype=50;
    //cout<<toKernel_Q.freeSpace<<endl;
    send_val = msgsnd(keyToKernel, &toKernel_Q, sizeof(toKernel_Q)-sizeof(toKernel_Q.mtype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send  Disk");

    //signal(SIGUSR1,countFree);
}
 
 
int main(int argc,char** argv)
{
    toKernel_Q.freeSpace=10;
    // for(int i=0;i<argc;++i){
    //     cout<<argv[i]<<endl;
    //     return 0;
    // }
    // if(argc>3){
    //     cerr<<"error";
    //     return 0;
    // }
    string var1=argv[0];
    string var2=argv[1];
    keyFromKernel=stoi(var2);
    keyToKernel=stoi(var1);

    signal(SIGUSR2, clockInc);
    signal(SIGUSR1, countFree);
    // while (1)
    // {
    //     struct msqid_ds buf;
    //     //3dd el messages in queue to sleep or work on
    //     msgctl(keyFromKernel, IPC_STAT, &buf);
    //     int num_messages = buf.msg_qnum;
    //     //cout<<num_messages<<endl;
    //     if (num_messages > 0)
    //     {
    //         int rec_val;
    //         //fromkernel is struct from the kernel which contains the message
    //         //tokernel is struct from the kernel which contains the number of free spaces
    //         rec_val = msgrcv(keyFromKernel, &fromKernel_Q, sizeof(fromKernel_Q) - sizeof(fromKernel_Q.mtype), 0, !IPC_NOWAIT);
    //         if (rec_val == -1)
    //             perror("Error in receive in disk");
            
    //         if (fromKernel_Q.operation == 'A')
    //         {
    //             Add(fromKernel_Q.text);
    //             cout<<"done adding "<<fromKernel_Q.text<<endl;
    //         }
    //         else
    //         {
    //             int ID = stoi(fromKernel_Q.text);
    //             Remove(ID);
    //             cout<<"done removing "<<fromKernel_Q.text<<endl;
    //         }
    //     }
    //     //else
    //       //  pause();
    // }
    while(true);
 }