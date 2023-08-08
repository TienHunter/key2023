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

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int* g_clients = NULL;
int g_count = 0;
int pid = 0;
void handler(int signum)
{
    if (signum == SIGINT)
    {
        free(g_clients);
        g_clients = NULL;
        exit(0);
    }else if (signum == SIGILL)
    {
        FILE* f = fopen("temp.dat","rb");
        fseek(f, 0, SEEK_END);
        int size = ftell(f);
        fseek(f, 0, SEEK_SET);
        char* buffer = (char*)calloc(size + 1, 1);
        fread(buffer, 1, size, f);
        fclose(f);
        for (int i = 0;i < g_count;i++)
        {
            send(g_clients[i], buffer, strlen(buffer), 0);
        }
        free(buffer);
        buffer = NULL;
    }
}
int main()
{
    pid = getpid();
    signal(SIGINT, handler);
    signal(SIGILL, handler);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(SOCKADDR_IN);
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr,sizeof(SOCKADDR_IN));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);
        g_clients = (int*)realloc(g_clients, (g_count + 1) * sizeof(int));
        g_clients[g_count] = c;
        g_count += 1;
        if (fork() == 0)
        {
            //CHILD PROCESS
            close(s);
            char buffer[1024];
            while (0 == 0)
            {
                memset(buffer, 0, sizeof(buffer));
                int r = recv(c, buffer, sizeof(buffer) - 1, 0);
                if (r > 0)
                {
                    printf("Socket %d received %d bytes: %s\n", c, r, buffer);
                    FILE* f = fopen("temp.dat", "wb");
                    fwrite(buffer, 1, strlen(buffer), f);
                    fclose(f);
                    kill(pid, SIGILL);
                }
                else
                    exit(0);
            }
        }
    }
}