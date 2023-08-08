#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
 #include <arpa/inet.h>
 
const char* domain = "vnexpress.net";
int main()
{
    struct addrinfo* tmp = NULL;
    struct addrinfo* result = NULL;
    getaddrinfo(domain, NULL, NULL, &result);
    if (result != NULL)
    {
        tmp = result;
        while (tmp != NULL)
        {
            if (tmp->ai_family == AF_INET)
            {
                struct sockaddr_in* addr = (struct sockaddr_in*)tmp->ai_addr;      
                printf("%s\n", inet_ntoa(addr->sin_addr));
            }else if (tmp->ai_family == AF_INET6)
            {
                printf("IPv6");
            }
            tmp = tmp->ai_next;
        }
    }
}