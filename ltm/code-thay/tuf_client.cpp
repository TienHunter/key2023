#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <stdlib.h>
#include <sys/wait.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
char name[1024] = { 0 };
pid_t pid1, pid2;

void handler(int signum)
{
    if (signum == SIGCHLD)
    {
        int status = 0;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) 
        {
            printf("A child process (%d) has terminated\n", pid);
        }
    }
}
void process1()
{
    printf("Process 1 is running\n");
    int on = 1;
    SOCKADDR_IN baddr;
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));     
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(6000);
    baddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    while (0 == 0)
    {
        sendto(s, name, strlen(name), 0, (SOCKADDR*)&baddr, sizeof(baddr));
        sleep(5);
    }  
}

void process2()
{
    sleep(1);
    printf("Process 2 is running\n");
    char buffer[1024] = { 0 };
    SOCKADDR_IN myaddr, saddr;
    int slen = sizeof(saddr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(6000);
    myaddr.sin_addr.s_addr = 0;
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));
    recvfrom(s, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&saddr, (socklen_t*)&slen);
    printf("Server IP: %s\n", inet_ntoa(saddr.sin_addr));
}

int main()
{
    printf("Client name: ");
    fgets(name, sizeof(name) - 1, stdin);
    if (name[strlen(name) - 1] == '\r' || 
        name[strlen(name) - 1] == '\n')
    {
        name[strlen(name) - 1] = 0;    
    }
    signal(SIGCHLD, handler);
    if ((pid1 = fork()) == 0)
    { //Child process 1
        process1();
    }else
    {
        if ((pid2 = fork()) == 0)
        { //Child process 2
            process2();
        }
    }
    getchar();
    printf("Terminating process: %d\n", pid1);
    kill(pid1, SIGKILL);
    printf("Terminating process: %d\n", pid2);
    kill(pid2, SIGKILL);
}