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
    int c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = inet_addr("172.22.86.63");
    connect(c, (SOCKADDR*)&saddr, sizeof(saddr));
    char buffer[1024];
    char data[16];
    while (0 == 0)
    {
        memset(buffer, 0, sizeof(buffer));
        printf("Command: ");
        fgets(buffer, sizeof(buffer) - 1, stdin);
        send(c, buffer, strlen(buffer), 0);
        memset(data, 0, sizeof(data));
        int tmp = 0;
        int received = 0;
        do
        {
            tmp = recv(c, data, sizeof(data) - 1, 0);
            printf("%s", data);
            received += tmp;
        } while (tmp == sizeof(data) - 1);

        printf("\n Total received: %d\n", received);
    }
}