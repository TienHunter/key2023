#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int main()
{
    SOCKADDR* clients = NULL;
    int count = 0;
    char message[1024];
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN myaddr, sender;
    int len = sizeof(sender);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(5000);
    myaddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));

    while (0 == 0)
    {
        memset(message, 0, sizeof(message));
        recvfrom(s, message, sizeof(message) - 1, 0, (SOCKADDR*)&sender, (socklen_t*)&len);
        clients = (SOCKADDR*)realloc(clients, (count + 1) * sizeof(SOCKADDR));
        memcpy(&clients[count], (SOCKADDR*)&sender, sizeof(SOCKADDR));
        count += 1;
        printf("%s\n", message);
        for (int i = 0;i < count;i++)
        {
            if (((SOCKADDR_IN*)&(clients[i]))->sin_addr.s_addr != sender.sin_addr.s_addr)
            {
                sendto(s, message, strlen(message), 0, (SOCKADDR*)&clients[i], sizeof(SOCKADDR));
            }
        }
    }
}