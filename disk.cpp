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

struct fromKernel {
    long mtype;
    char operation;
    char text[64];
};
struct toKernel {
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
struct msqid_ds buf;

void inserting(string message)
{
    for (int i = 0; i < 10; i++) {
        if (!full[i]) {
            full[i] = true;
            toKernel_Q.freeSpace--;
            storge[i] = message;
            return;
        }
    }
}

void removing(int ID)
{
    if (ID >= 0 && ID <= 9) {
        full[ID] = false;
        toKernel_Q.freeSpace++;
        storge[ID] = ' ';
    }
}

void clockInc(int dummy) // handler for siguser2 signal
{
    clk++;
}

void countFree(int dummy)
{
    int send_val;
    toKernel_Q.mtype = 1;
    send_val = msgsnd(keyToKernel, &toKernel_Q, sizeof(toKernel_Q) - sizeof(toKernel_Q.mtype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send  Disk");
}

int main(int argc, char** argv)
{
    toKernel_Q.freeSpace = 10;
    string var1 = argv[0];
    string var2 = argv[1];
    keyFromKernel = stoi(var2);
    keyToKernel = stoi(var1);
    signal(SIGUSR2, clockInc);
    signal(SIGUSR1, countFree);
    for (int i = 0; i < 10; i++)
        full[i] = false;
    while (1) {

        int rec_val;
        //fromkernel is struct from the kernel which contains the message
        //tokernel is struct from the kernel which contains the number of free spaces
        rec_val = msgrcv(keyFromKernel, &fromKernel_Q, sizeof(fromKernel_Q) - sizeof(fromKernel_Q.mtype), 1, IPC_NOWAIT);

        if (rec_val > 0) {
            if (fromKernel_Q.operation == 'A') {
                inserting(fromKernel_Q.text);
                cout << "Disk starts to add at time " << clk << endl;
		pause();
            }
            else {
                string tempStr(fromKernel_Q.text);
                int ID = stoi(tempStr);
                removing(ID);
                cout << "Disk starts to delete at time " << clk << endl;
		pause();
            }
        }
    }
}
