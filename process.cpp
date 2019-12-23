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
struct toKernel
{
    long mtype;
    char operation;
    string text;
};
struct job
{
    int clk;
    char operation;
    string text;
};
key_t KeyToKernel;
int clkMain = 0;
char process_number[10];
vector<job>jobs(100);
struct toKernel toKernel_Q;

int iter=0;
void read_all_instructions(int id)
{
    string tempstr="process"+to_string(id)+".txt";
    char cstr[tempstr.size() + 1];
	strcpy(cstr, tempstr.c_str());
   freopen(cstr,"r",stdin);
   string s;
   int f;
   char op;
   while(cin>>f){
       cin>>op>>s;
       s.resize(64);
       jobs[iter].clk=f;
       jobs[iter].operation=op;
       jobs[iter].text=s;
       iter++;
   }
}
bool compareInterval(job const &i1, job const &i2) 
{ 
    return (i1.clk < i2.clk); 
} 

void clockInc(int signum) // handler for siguser2 signal
{
    clkMain++;
}
int main(int argc,char** argv)
{

    // if(argc<4){
    //     cerr<<"error";
    //     return 0;
    // }
    string var1=argv[0];
    string var2=argv[1];
    int id=stoi(var1);
    int KeyToKernel=stoi(var2);
    
    signal(SIGUSR2, clockInc);
    signal(SIGUSR1,SIG_DFL);
    read_all_instructions(id);
    jobs.resize(iter);
    sort(jobs.begin(),jobs.end(),compareInterval);
    //kill(SIGUSR1,getppid());  //sent to the kernel that im ready now to start
    int i=0;
    while (iter > 0)
    {
        if (clkMain == jobs[i].clk)
        {
            toKernel_Q.mtype=40;
            toKernel_Q.operation=jobs[i].operation;
            toKernel_Q.text=jobs[i].text;

            //keytokernel is key of the message queue between process and the kernel
            //tokernel_Q is struct of the message sent
            int send_val = msgsnd(KeyToKernel, &toKernel_Q, sizeof(toKernel_Q) - sizeof(toKernel_Q.mtype), !IPC_NOWAIT);
            if (send_val == -1)
                perror("Error in send in process");
            //we need to send sigusr1 for sync. here
        iter--;
        i++;
        }
        else
            pause();
    }
    return 0;
}
