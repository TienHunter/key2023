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

int main()
{
    SOCKADDR_IN myaddr, caddr;
    int clen = sizeof(caddr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(8888);
    myaddr.sin_addr.s_addr = 0;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));
    listen(s, 10);
    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(s, &rset);

    select(FD_SETSIZE, &rset, NULL, NULL, NULL);

    if (FD_ISSET(s, &rset))
    {
        int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);
        printf("Socket %d connected\n", c);
        close(c);
    }
}