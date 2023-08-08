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

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int* g_clients = NULL;
int* g_status = NULL;
int g_count = 0;

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
    fd_set rset;
    while (0 == 0)
    {
        FD_ZERO(&rset);
        FD_SET(s, &rset);
        for (int i = 0;i < g_count;i++)
        {
            if (g_status[i] == 1)
            {
                FD_SET(g_clients[i], &rset);
            }
        }
        int result = select(FD_SETSIZE, &rset, NULL, NULL, NULL);
        if (result > 0)
        {
            if (FD_ISSET(s, &rset))
            {
                int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);
                g_clients = (int*)realloc(g_clients, (g_count + 1) * sizeof(int));
                g_clients[g_count] = c;
                g_status = (int*)realloc(g_status, (g_count + 1) * sizeof(int));
                g_status[g_count] = 1;
                g_count += 1;
                printf("A new client has connected: %d\n", c);
            }
            for (int i = 0;i < g_count;i++)
            {
                if (FD_ISSET(g_clients[i], &rset))
                {
                    char buffer[1024] = {0};
                    int r = recv(g_clients[i], buffer, sizeof(buffer) - 1, 0);
                    if (r > 0)
                    {
                        printf("Received from %d: %s\n", g_clients[i], buffer);
                        for (int k = 0;k < g_count;k++)
                        {
                            if (k != i)
                            {
                                send(g_clients[k], buffer, strlen(buffer), 0);
                            }
                        }
                    }else
                        g_status[i] = 0;
                }
            }
        }
    }
}