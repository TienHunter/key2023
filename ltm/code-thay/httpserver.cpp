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
#include <dirent.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
char *rootPath = NULL;
char *html = NULL;

void Send(int c, char *data, int len)
{
    // Hàm này gửi dữ liệu qua socket theo từng phần.
    // Nó gửi 'len' byte của 'data' qua socket 'c'.
    int sent = 0;
    while (sent < len)
    {
        // Gửi dữ liệu từ 'data' tại vị trí 'sent' qua socket 'c'.
        int tmp = send(c, data + sent, len - sent, 0);

        // Nếu tmp > 0, có dữ liệu được gửi thành công.
        if (tmp > 0)
        {
            // Cập nhật số lượng byte đã gửi.
            sent += tmp;
        }
        else
            // Trường hợp tmp <= 0, có lỗi hoặc kết thúc kết nối, thoát khỏi vòng lặp.
            break;
    }
}

int my_filter(const struct dirent *arg)
{
    // Hàm này được sử dụng làm bộ lọc cho scandir().
    // Nó loại bỏ các mục không phải là tệp hoặc thư mục thường.
    // Kiểm tra kiểu của mục (d_type) để xác định xem nó là tệp hay thư mục.
    if (arg->d_type == DT_REG || arg->d_type == DT_DIR)
        return 1; // Trả về 1 nếu là tệp hoặc thư mục, cho phép mục này được lựa chọn.
    else
        return 0; // Trả về 0 nếu không phải là tệp hoặc thư mục, loại bỏ mục này khỏi danh sách.
}

int my_compare(const struct dirent **arg1, const struct dirent **arg2)
{
    // Hàm này được sử dụng làm bộ so sánh cho scandir().
    // Nó sắp xếp các mục theo kiểu và tên.
    // So sánh kiểu của hai mục để xác định thứ tự sắp xếp.
    if ((*arg1)->d_type == (*arg2)->d_type)
        return strcmp((*arg1)->d_name, (*arg2)->d_name); // Sắp xếp theo tên nếu cùng kiểu.
    else if ((*arg1)->d_type == DT_DIR)
        return -1; // Ưu tiên thư mục khi sắp xếp, trả về -1 nếu arg1 là thư mục và arg2 không phải.
    else
        return 1; // Trả về 1 nếu arg1 không phải thư mục và arg2 là thư mục.
}

void append(char **phtml, const char *str)
{
    // Hàm này nối thêm 'str' vào một chuỗi được cấp phát động 'phtml'.

    // Lấy con trỏ đến chuỗi được cấp phát động ('*phtml').
    char *localhtml = *phtml;

    // Xác định độ dài hiện tại của chuỗi 'localhtml'.
    int oldLen = localhtml == NULL ? 0 : strlen(localhtml);

    // Tính toán độ dài mới sau khi nối 'str' vào 'localhtml'.
    int newLen = oldLen + strlen(str) + 1;

    // Thực hiện việc cấp phát lại bộ nhớ cho 'localhtml' với độ dài mới.
    localhtml = (char *)realloc(localhtml, newLen * sizeof(char));

    // Đặt các byte mới cấp phát thành 0 để đảm bảo chuỗi kết thúc hợp lý.
    memset(localhtml + oldLen, 0, (newLen - oldLen) * sizeof(char));

    // Nối nội dung của chuỗi 'str' vào cuối của 'localhtml'.
    sprintf(localhtml + oldLen, "%s", str);

    // Cập nhật con trỏ đến chuỗi được cấp phát động '*phtml' để trỏ đến chuỗi đã nối.
    *phtml = localhtml;
}

void MakeContent()
{
    // Hàm này tạo nội dung HTML cho phản hồi của máy chủ.
    // Nó sử dụng scandir() để liệt kê nội dung thư mục và tạo các liên kết HTML.
    // Nó cũng tạo một biểu mẫu tải tệp lên.
    struct dirent **result = NULL;
    int n = scandir(rootPath, &result, my_filter, my_compare);
    append(&html, "<html>");
    for (int i = 0; i < n; i++)
    {
        if (result[i]->d_type == DT_DIR)
        {
            append(&html, "<a href = \"");
            append(&html, rootPath);
            if (rootPath[strlen(rootPath) - 1] != '/')
            {
                append(&html, "/");
            }
            append(&html, result[i]->d_name);
            append(&html, "\"><b>");
            append(&html, result[i]->d_name);
            append(&html, "</b></a><br>\n");
        }
        else if (result[i]->d_type == DT_REG)
        {
            append(&html, "<a href = \"");
            append(&html, rootPath);
            if (rootPath[strlen(rootPath) - 1] != '/')
            {
                append(&html, "/");
            }
            append(&html, result[i]->d_name);
            append(&html, "*");
            append(&html, "\"><i>");
            append(&html, result[i]->d_name);
            append(&html, "</i></a><br>\n");
        }
        free(result[i]);
        result[i] = NULL;
    }
    append(&html, "<BR><BR>");
    append(&html, "<H>FILE UPLOADING FORM<BR>");
    append(&html, "<form action=\"");
    append(&html, rootPath);
    append(&html, "\" method=\"POST\" enctype=\"multipart/form-data\">");
    append(&html, "<label>Select file to upload: </label>");
    append(&html, "<input type=\"file\" name=\"file1\"><BR>");
    append(&html, "<label>Select file to upload: </label>");
    append(&html, "<input type=\"file\" name=\"file2\"><BR>");
    append(&html, "<input type=\"submit\" value=\"Upload\">");
    append(&html, "</form>");
    append(&html, "</html>");
}

char *FindBoundary(char *data, int length, char *boundary)
{
    // Hàm này tìm kiếm chuỗi biên trong 'data'.
    // Nó được sử dụng để tìm các biên trong yêu cầu POST multipart/form-data.
    for (int i = 0; i < length; i++)
    {
        if (data[i] == boundary[0])
        {
            int j = 0;
            for (j = 0; j < strlen(boundary); j++)
            {
                if (data[i + j] != boundary[j])
                {
                    break;
                }
            }
            if (j == strlen(boundary))
            {
                return data + i;
            }
        }
    }
    return NULL;
}

void *ClientThread(void *arg)
{
    // Đây là hàm luồng xử lý cho mỗi kết nối khách hàng.
    // Nó nhận và xử lý các yêu cầu HTTP, phục vụ nội dung và xử lý việc tải tệp lên.
    int c = *((int *)arg);
    free(arg);
    arg = NULL;
    char *buffer = NULL;
    while (0 == 0)
    {
        char tmp;
        int r = recv(c, &tmp, 1, 0);
        if (r > 0)
        {
            int size = buffer == NULL ? 0 : strlen(buffer);
            buffer = (char *)realloc(buffer, size + 2);
            buffer[size] = tmp;
            buffer[size + 1] = 0;
            if (strstr(buffer, "\r\n\r\n") != NULL)
            {
                break;
            }
        }
        else
            break;
    }
    if (buffer != NULL && strlen(buffer) > 0)
    {
        char method[16] = {0};
        char path[1024] = {0};
        sscanf(buffer, "%s%s", method, path);

        // methods get
        if (strstr(method, "GET"))
        {
            while (strstr(path, "%20") != NULL)
            {
                char tmp[1024] = {0};
                char *start = strstr(path, "%20");
                strncpy(tmp, path, start - path);
                sprintf(tmp + strlen(tmp), " %s", start + 3);
                strcpy(path, tmp);
            }

            if (strcmp(path, "/") == 0)
            {
                const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                Send(c, (char *)header, strlen(header));
                MakeContent();
                Send(c, html, strlen(html));
                free(html);
                html = NULL;
                close(c);
            }
            else
            {
                if (strcmp(path, "/favicon.ico") == 0)
                {
                    FILE *f = fopen("/mnt/e/WSL/fav.ico", "rb");
                    if (f != NULL)
                    {
                        const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                        Send(c, (char *)header, strlen(header));
                        fseek(f, 0, SEEK_END);
                        int size = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        char *data = (char *)calloc(size, sizeof(char));
                        fread(data, 1, size, f);
                        Send(c, data, size);
                        close(c);
                        fclose(f);
                    }
                    else
                    {
                        const char *header = "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
                        Send(c, (char *)header, strlen(header));
                        const char *notfound = "<html>RESOURCE NOT FOUND</html>";
                        Send(c, (char *)notfound, strlen(notfound));
                        close(c);
                    }
                }
                else if (path[strlen(path) - 1] == '*')
                {
                    path[strlen(path) - 1] = 0;
                    FILE *f = fopen(path, "rb");
                    if (f != NULL)
                    {
                        char header[1024] = {0};
                        if (strstr(path, ".txt") != NULL)
                            strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
                        else if (strstr(path, ".wav") != NULL)
                            strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: audio/wav\r\n\r\n");
                        else if (strstr(path, ".pdf") != NULL)
                            strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\n\r\n");
                        else
                            strcpy(header, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n");

                        Send(c, (char *)header, strlen(header));
                        fseek(f, 0, SEEK_END);
                        int size = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        char *data = (char *)calloc(size, sizeof(char));
                        fread(data, 1, size, f);
                        Send(c, data, size);
                        close(c);
                        fclose(f);
                    }
                    else
                    {
                        const char *header = "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n";
                        Send(c, (char *)header, strlen(header));
                        const char *notfound = "<html>RESOURCE NOT FOUND</html>";
                        Send(c, (char *)notfound, strlen(notfound));
                        close(c);
                    }
                }
                else
                {
                    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
                    Send(c, (char *)header, strlen(header));
                    rootPath = (char *)realloc(rootPath, strlen(path) + 1);
                    strcpy(rootPath, path);
                    MakeContent();
                    Send(c, html, strlen(html));
                    free(html);
                    html = NULL;
                    close(c);
                }
            }
        }
        else if (strstr(method, "POST"))
        {
            while (strstr(path, "%20") != NULL)
            {
                char tmp[1024] = {0};
                char *start = strstr(path, "%20");
                strncpy(tmp, path, start - path);
                sprintf(tmp + strlen(tmp), " %s", start + 3);
                strcpy(path, tmp);
            }

            int length = 0;
            char *ct = strstr(buffer, "Content-Length: ");
            sscanf(ct + strlen("Content-Length: "), "%d", &length);
            char *data = (char *)calloc(length, 1);
            int r = 0;
            while (r < length)
            {
                int tmp = recv(c, data + r, length - r, 0);
                if (tmp > 0)
                {
                    r += tmp;
                }
                else
                    break;
            }
            char *bd = strstr(buffer, "boundary=");
            char boundary[1024] = {0};
            sscanf(bd + strlen("boundary="), "%s", boundary);
            char *start = data;
            while (0 == 0)
            {
                int offset = data - start;
                start = FindBoundary(start, length - offset, boundary);
                if (start != NULL)
                {
                    char *fnStart = strstr(start, "filename=\"");
                    if (fnStart != NULL)
                    {
                        fnStart += strlen("filename=\"");
                        char *fnEnd = strstr(fnStart, "\"");
                        char filename[1024] = {0};
                        strncpy(filename, fnStart, fnEnd - fnStart);
                        char *fileDataStart = strstr(start, "\r\n\r\n") + 4;
                        char *fileDataEnd = FindBoundary(start + 1, length - offset - 1, boundary);
                        while (strncmp(fileDataEnd, "\r\n", 2) != 0)
                        {
                            fileDataEnd -= 1;
                        }
                        char absoluePath[4096] = {0};
                        if (path[strlen(path) - 1] != '/')
                            sprintf(absoluePath, "%s/%s", path, filename);
                        else
                            sprintf(absoluePath, "%s%s", path, filename);

                        FILE *f = fopen(absoluePath, "wb");
                        fwrite(fileDataStart, 1, fileDataEnd - fileDataStart, f);
                        fclose(f);

                        start = fileDataEnd;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            free(data);
            data = NULL;

            const char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            Send(c, (char *)header, strlen(header));
            const char *okay = "<html>UPLOADING COMPLETED</html>";
            Send(c, (char *)okay, strlen(okay));
            close(c);
        }
        free(buffer);
        buffer = NULL;
    }
    return NULL;
}
int main()
{
    // Đây là hàm chính thiết lập và chạy máy chủ HTTP.
    // Nó lắng nghe các kết nối đến, tạo luồng để xử lý khách hàng, và nhiều hơn nữa.
    rootPath = (char *)calloc(2, sizeof(char));
    strcpy(rootPath, "/");

    // lưu địa chỉ client
    SOCKADDR_IN myaddr, caddr;
    int clen = sizeof(caddr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(5000);
    myaddr.sin_addr.s_addr = 0;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // bidn địa chỉ
    bind(s, (SOCKADDR *)&myaddr, sizeof(myaddr));
    listen(s, 10);

    //  nhận kết nối
    while (0 == 0)
    {
        // khai báo socket c
        int c = accept(s, (SOCKADDR *)&caddr, (socklen_t *)&clen);
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = c;
        pthread_t tid;
        pthread_create(&tid, NULL, ClientThread, (void *)arg);
    }
}