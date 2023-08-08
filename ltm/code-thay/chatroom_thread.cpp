#include <pthread.h>
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

typedef struct 
{
    int index;
    int value;
}THREADARG;

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int* g_clients = NULL;
int* g_status = NULL;
int g_count = 0;

void* ClientThread(void* arg)
{
    THREADARG* tmp = (THREADARG*)arg;
    int c = tmp->value;
    int i = tmp->index;    
    free(tmp);
    char buffer[1024];
    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        int r = recv(c, buffer, sizeof(buffer) - 1, 0);
        if (r > 0)
        {
            printf("Received from %d: %s\n", c, buffer);
            for (int i = 0;i < g_count;i++)
            {
                if (g_clients[i] != c && g_status[i] == 1)
                {
                    send(g_clients[i], buffer, strlen(buffer), 0);
                }
            }        
        }else
            break;
    }
    printf("A client has disconnected\n");
    g_status[i] = 0;
    return NULL;
}

int main()
{
    SOCKADDR_IN myaddr, caddr;
    int clen = sizeof(caddr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(5000);
    myaddr.sin_addr.s_addr = 0;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);   
        printf("A new client connected: %d\n", c);     
        g_clients = (int*)realloc(g_clients, (g_count + 1) * sizeof(int));
        g_status = (int*)realloc(g_status, (g_count + 1) * sizeof(int));
        g_clients[g_count] = c;
        g_status[g_count] = 1;

        THREADARG* tmp = (THREADARG*)calloc(1, sizeof(THREADARG));
        tmp->index = g_count;
        tmp->value = c;
        g_count += 1;
        pthread_t tid;
        pthread_create(&tid, NULL, ClientThread, (void*)tmp);
    }
}