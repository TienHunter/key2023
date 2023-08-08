/* forksharingclient.c: Minh họa xử lý đa tiến trình (máy trạm) */
#include <stdio.h>           /* fprintf(), fgets(), printf() */
#include <unistd.h>          /* socket(), connect(), fork(), bind(), sendto(), recvfrom(), sleep() */
#include <stdlib.h>          /* malloc() */
#include <sys/socket.h>      /* sockaddr */
#include <sys/types.h>       /* inet_addr() */
#include <netdb.h>           /* inet_addr() */
#include <arpa/inet.h>       /* inet_addr() */
#include <string.h>          /* strlen(), memset() */
#include <signal.h>          /* signal() */
#include <sys/wait.h>        /* wait() */
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

int child = 0;  // Số hiệu tiến trình con
void kill_child(int signum) {
    if (child != 0)
        kill(child, SIGTERM);
    exit(signum);
}
int main() {
    child = fork();
    if (child == 0) {
        /* Tiến trình con: Nhận các yêu cầu dò tìm từ máy khác. */
        
        /* Socket nhận dò tìm. */
        int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        SOCKADDR_IN myaddr; // Địa chỉ nhận
        myaddr.sin_family = AF_INET;   // IPv4
        myaddr.sin_port = htons(6000); // Cổng 6000
        myaddr.sin_addr.s_addr = 0;    // Địa chỉ bất kỳ của máy
        bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));

        SOCKADDR_IN sender;        // Địa chỉ người gửi
        int clen = sizeof(sender); // Kích thước địa chỉ trên
        char buffer[1024];         // Nhận yêu cầu dò tìm gồm tên máy
        while (0 == 0) {
            memset(buffer, 0, sizeof(buffer));
            /* Nhận một yêu cầu dò tìm bất kỳ. */
            recvfrom(s, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&sender, (socklen_t*)&clen);   
            /* Cắt ký tự xuống dòng, nếu có. */
            if (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r') {
                buffer[strlen(buffer) - 1] = 0;
            }
            /* Ghi lại tên và địa chỉ của máy vừa rồi. */
            FILE* f = fopen("clients.txt","at");
            fprintf(f, "%s %s\n", buffer, inet_ntoa(sender.sin_addr));
            fclose(f);     
            /* Phản hồi bên gửi. */
            sendto(s, "ACK", 3, 0, (SOCKADDR*)&sender, sizeof(sender));
        }
    } else {
        /* Tiến trình cha: Trả lời với danh sách các máy trạm trên mạng. */

        /* Xem thêm giải thích trong forksharingclient.c */
        signal(SIGINT, kill_child);

        int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        SOCKADDR_IN myaddr;
        myaddr.sin_family = AF_INET;
        myaddr.sin_port = htons(5000);
        myaddr.sin_addr.s_addr = 0;
        bind(s, (SOCKADDR*)&myaddr, sizeof(myaddr));
        listen(s, 10);

        SOCKADDR_IN client;
        int clen = sizeof(client);
        while (0 == 0) {
            int c = accept(s, (SOCKADDR*)&client, (socklen_t*)&clen);
            /* clients.txt: danh sách máy trạm đã biết. */
            FILE* f = fopen("clients.txt", "rb");
            fseek(f, 0, SEEK_END);
            int size = ftell(f);
            fseek(f, 0, SEEK_SET);
            char* data = (char*)calloc(size + 1, 1);
            fread(data, 1, size, f);
            fclose(f);
            int sent = 0;
            while (sent < size) {
                int tmp = send(c, data + sent, size - sent, 0);
                if (tmp > 0)
                    sent += tmp;
                else
                    break;
            }
            close(c);
            free(data);
            data = NULL;
        }
        close(s);
        kill_child(0);
    }
}
