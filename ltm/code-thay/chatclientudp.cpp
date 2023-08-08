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
    char message[1024];
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN toaddr;
    toaddr.sin_family = AF_INET;
    toaddr.sin_port = htons(5000);
    toaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    SOCKADDR_IN myaddr, sender;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(6000);
    myaddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));

    while (0 == 0)
    {
        memset(message, 0, sizeof(message));
        fgets(message, sizeof(message) - 1, stdin);
        sendto(s, message, strlen(message), 0, (SOCKADDR*)&toaddr, sizeof(toaddr));
        memset(message, 0, sizeof(message));
        int len = sizeof(sender);
        recvfrom(s, message, sizeof(message) - 1, 0, (SOCKADDR*)&sender, (socklen_t*)&len);
        printf("%s\n", message);
    }
}