#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
 #include <arpa/inet.h>
 
const char* domain = "google.com";
int main()
{
    struct hostent* result = gethostbyname(domain);
    if (result != NULL)
    {
        int j = 0;
        while (result->h_addr_list[j] != NULL)
        {
            if (result->h_addrtype == AF_INET)
            {
                printf("%d.%d.%d.%d\n", (unsigned char)result->h_addr_list[j][0],
                                        (unsigned char)result->h_addr_list[j][1],
                                        (unsigned char)result->h_addr_list[j][2],
                                        (unsigned char)result->h_addr_list[j][3]);
            }else
                printf("Not an IPv4 address\n");

            j += 1;
        }
    }
    printf("...");
}