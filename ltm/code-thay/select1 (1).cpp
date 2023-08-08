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

    struct timeval to;
    to.tv_sec = 0;
    to.tv_usec = 0;

    int result = 0;
    do 
    {
        FD_SET(s, &rset);
        result = select(FD_SETSIZE, &rset, NULL, NULL, &to);
        if (FD_ISSET(s, &rset))
        {
            printf("CONNECTED\n");
            break;
        }else
            printf("Waiting...\n");
        usleep(10000); // Tranh CPU len 100%
    }while (0 == 0);

    int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);
    const char* msg = "Hello";
    send(c, msg, strlen(msg), 0);
    printf("Socket %d connected\n", c);
    close(c);
        
}