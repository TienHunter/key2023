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
#include <sys/ioctl.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
int* g_clients = NULL;
int* g_status = NULL;
int g_count = 0;

int IsBKCAReady()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &(saddr.sin_addr));

    unsigned long argp = 1;
    int result = ioctl(client, FIONBIO, &argp);
    if (result == -1)
    {
        return 0;
    }
    if (!connect(client, (struct sockaddr*)&saddr, sizeof(saddr)))
    {
        return 0;
    }
    argp = 0;
    result = ioctl(client, FIONBIO, &argp);
    if (result == -1)
    {
        return 0;
    }

    fd_set fdwrite, fderr;
    FD_ZERO(&fdwrite);
    FD_ZERO(&fderr);
    FD_SET(client, &fdwrite);
    FD_SET(client, &fderr);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    int r = select(0, NULL, &fdwrite, &fderr, &timeout);

    int error_code;
    int error_code_size = sizeof(error_code);
    getsockopt(client, SOL_SOCKET, SO_ERROR, &error_code, (socklen_t*)&error_code_size);
    printf("Error Code: %d\n", error_code);
    return (error_code == 0);
}

int main()
{
    if (IsBKCAReady())
        printf("Ready\n");
    else
        printf("Not ready\n");
}