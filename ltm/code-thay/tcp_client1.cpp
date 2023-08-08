#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

const char* ip = "111.65.250.2";
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int main()
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = inet_addr(ip);
    connect(s, (SOCKADDR*)&addr, sizeof(SOCKADDR));
    char buffer[1024];
    memset(buffer, 0 ,sizeof(buffer));
    const char* welcome = "GET / HTTP/1.1\r\nHost: vnexpress.net\r\n\r\n";
    int sent = send(s, welcome, strlen(welcome), 0);
    printf("Sent %d bytes\n", sent);
    int received = recv(s, buffer, sizeof(buffer) - 1, 0);
    printf("Received %d bytes: %s\n", received, buffer);
    close(s);
}