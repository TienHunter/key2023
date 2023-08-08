#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int main()
{
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR*)&saddr, sizeof(SOCKADDR));
    listen(s, 10);
    int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);
    const char* welcome = "Hello TCP Server\n";
    int sent = send(c, welcome, strlen(welcome), 0);
    printf("Sent: %d bytes\n", sent);
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int received = recv(c, buffer, sizeof(buffer) - 1, 0);
    printf("Received: %d bytes\n", received);
    close(c);
    close(s);
}