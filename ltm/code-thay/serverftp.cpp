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

typedef char USERNAME[1024];
typedef char PASSWORD[1024];
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
char *rootPath = NULL;
USERNAME *listUsers = NULL;
PASSWORD *listPsws = NULL;
int userCount = 0;
int pasvPort = 1024;

int checkUser(const char *username)
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(listUsers[i], username) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int checkUserPass(const char *username, const char *psw)
{
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(listUsers[i], username) == 0 &&
            strcmp(listPsws[i], psw) == 0)
        {
            return 1;
        }
    }
    return 0;
}

int Send(int c, char *data, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int tmp = send(c, data + sent, len - sent, 0);
        if (tmp > 0)
        {
            sent += tmp;
        }
        else
            return 0;
    }
    return sent;
}

int my_filter(const struct dirent *arg)
{
    if (arg->d_type == DT_REG || arg->d_type == DT_DIR)
        return 1;
    else
        return 0;
}

int my_compare(const struct dirent **arg1, const struct dirent **arg2)
{
    if ((*arg1)->d_type == (*arg2)->d_type)
        return strcmp((*arg1)->d_name, (*arg2)->d_name);
    else if ((*arg1)->d_type == DT_DIR)
        return -1;
    else
        return 1;
}

void append(char **phtml, const char *str)
{
    char *localhtml = *phtml;
    int oldLen = localhtml == NULL ? 0 : strlen(localhtml);
    int newLen = oldLen + strlen(str) + 1;
    localhtml = (char *)realloc(localhtml, newLen * sizeof(char));
    memset(localhtml + oldLen, 0, (newLen - oldLen) * sizeof(char));
    sprintf(localhtml + oldLen, "%s", str);
    *phtml = localhtml;
}

char *MakeContent()
{
    char *data = NULL;
    struct dirent **result = NULL;
    int n = scandir(rootPath, &result, my_filter, my_compare);
    for (int i = 0; i < n; i++)
    {
        char tmp[1024] = {0};
        if (result[i]->d_type == DT_DIR)
        {
            sprintf(tmp, "type=dir;modify=20220612103114.863;perms=cplem; %s\r\n", result[i]->d_name);
        }
        else if (result[i]->d_type == DT_REG)
        {
            sprintf(tmp, "type=file;size=0;modify=20230713005830.028;perms=awr; %s\r\n", result[i]->d_name);
        }
        append(&data, tmp);
        free(result[i]);
        result[i] = NULL;
    }
    return data;
}

void *ClientThread(void *arg)
{
    int c = *((int *)arg);
    free(arg);
    arg = NULL;
    char username[1024] = {0};
    char psw[1024] = {0};
    int dataSocket = -1;
    char *buffer = NULL;
    const char *welcome = "220 Ok I am ready\r\n";
    Send(c, (char *)welcome, strlen(welcome));
    while (0 == 0)
    {
        int succeeded = 1;
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
                if (strstr(buffer, "\r\n") != NULL)
                {
                    break;
                }
            }
            else
            {
                succeeded = 0;
                break;
            }
        }
        printf("%s\n", buffer);
        if (succeeded == 1 && buffer != NULL && strlen(buffer) > 0)
        {
            if (strncmp(buffer, "USER ", 5) == 0)
            {
                sscanf(buffer + 5, "%s", username);
                if (checkUser(username))
                {
                    const char *resp = "331 Username Okay\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
                else
                {
                    const char *resp = "430 Invalid username\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
            }
            else if (strncmp(buffer, "PASS ", 5) == 0)
            {
                sscanf(buffer + 5, "%s", psw);
                if (checkUserPass(username, psw))
                {
                    const char *resp = "230 Login Completed\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
                else
                {
                    const char *resp = "430 Invalid username or password\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
            }
            else if (strncmp(buffer, "SYST", 4) == 0)
            {
                const char *resp = "215 UNIX emulated by LATHEVINH\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "FEAT", 4) == 0)
            {
                const char *resp = "211-Features:\r\nMDTM\r\n REST STREAM\r\n SIZE\r\n MLST type*;size*;modify*;perm*;\r\n MLSD\r\n AUTH SSL\r\n AUTH TLS\r\n PROT\r\n PBSZ\r\n UTF8\r\n TVFS\r\n EPSV\r\n EPRT\r\n MFMT\r\n211 End\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "OPTS", 4) == 0)
            {
                const char *resp = "202 OKAY\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "PWD", 3) == 0)
            {
                char resp[1024] = {0};
                sprintf(resp, "257 \"%s\" is the current path\r\n", rootPath);
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "TYPE", 4) == 0)
            {
                const char *resp = "200 OKAY\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "PASV", 4) == 0)
            {
                SOCKADDR_IN myaddr, caddr;
                int clen = sizeof(caddr);
                myaddr.sin_family = AF_INET;
                myaddr.sin_port = htons(pasvPort);
                myaddr.sin_addr.s_addr = 0;
                int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (bind(s, (SOCKADDR *)&myaddr, sizeof(myaddr)) == 0)
                {
                    listen(s, 10);
                    char resp[1024] = {0};
                    sprintf(resp, "227 Passive Mode (172,22,86,63,%d,%d)\r\n", (pasvPort & 0xFF00) >> 8, (pasvPort & 0x00FF));
                    pasvPort += 1;
                    Send(c, (char *)resp, strlen(resp));
                    dataSocket = accept(s, (SOCKADDR *)&caddr, (socklen_t *)&clen);
                }
                else
                {
                    const char *resp = "500 Command not supported\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
            }
            else if (strncmp(buffer, "MLSD", 4) == 0)
            {
                if (dataSocket > 0)
                {
                    const char *resp150 = "150 Start transferring\r\n";
                    Send(c, (char *)resp150, strlen(resp150));
                    char *respData = MakeContent();
                    // DUNG HAM SCANDIR DE TAO respData dung voi RootPath
                    Send(dataSocket, (char *)respData, strlen(respData));
                    close(dataSocket);
                    dataSocket = -1;
                    const char *resp226 = "226 Transferring completed\r\n";
                    Send(c, (char *)resp226, strlen(resp226));
                }
                else
                {
                    const char *resp = "425 Data connection has not been established\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
            }
            else if (strncmp(buffer, "CWD ", 4) == 0)
            {
                char *folder = buffer + 4;
                while (folder[strlen(folder) - 1] == '\r' || folder[strlen(folder) - 1] == '\n')
                {
                    folder[strlen(folder) - 1] = 0;
                }
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(rootPath + strlen(rootPath), "%s", folder);
                }
                else
                {
                    sprintf(rootPath + strlen(rootPath), "/%s", folder);
                }
                const char *resp = "200 CWD Completed\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "CDUP", 4) == 0)
            {
                // không phải thư mục root
                if (strlen(rootPath) > 1)
                {
                    if (rootPath[strlen(rootPath) - 1] == '/')
                    {
                        rootPath[strlen(rootPath) - 1] = 0;
                    }
                    while (rootPath[strlen(rootPath) - 1] != '/')
                    {
                        rootPath[strlen(rootPath) - 1] = 0;
                    }
                }
                const char *resp = "200 CDUP Completed\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "SIZE ", 5) == 0)
            {
                char *filename = buffer + 5;
                while (filename[strlen(filename) - 1] == '\r' || filename[strlen(filename) - 1] == '\n')
                {
                    filename[strlen(filename) - 1] = 0;
                }
                char absPath[1024] = {0};
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(absPath, "%s%s", rootPath, filename);
                }
                else
                {
                    sprintf(absPath, "%s/%s", rootPath, filename);
                }
                FILE *f = fopen(absPath, "rb");
                int size = 0;
                fseek(f, 0, SEEK_END);
                size = ftell(f);
                fclose(f);
                char resp[1024] = {0};
                sprintf(resp, "213 %d\r\n", size);
                Send(c, (char *)resp, strlen(resp));
            }
            else if (strncmp(buffer, "RETR ", 5) == 0)
            {
                char *filename = buffer + 5;
                while (filename[strlen(filename) - 1] == '\r' || filename[strlen(filename) - 1] == '\n')
                {
                    filename[strlen(filename) - 1] = 0;
                }
                char absPath[1024] = {0};
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(absPath, "%s%s", rootPath, filename);
                }
                else
                {
                    sprintf(absPath, "%s/%s", rootPath, filename);
                }
                const char *resp150 = "150 Start transferring\r\n";
                Send(c, (char *)resp150, strlen(resp150));
                FILE *f = fopen(absPath, "rb");
                fseek(f, 0, SEEK_END);
                int size = ftell(f);
                fseek(f, 0, SEEK_SET);
                int r = 0;
                while (r < size)
                {
                    char block[1024] = {0};
                    int tmp = fread(block, 1, sizeof(block), f);
                    if (tmp > 0)
                    {
                        r += tmp;
                        if (Send(dataSocket, block, tmp) <= 0)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                fclose(f);
                close(dataSocket);
                const char *resp226 = "226 Transferring completed\r\n";
                Send(c, (char *)resp226, strlen(resp226));
            }
            else if (strncmp(buffer, "STOR ", 5) == 0)
            {
                char *filename = buffer + 5;
                while (filename[strlen(filename) - 1] == '\r' || filename[strlen(filename) - 1] == '\n')
                {
                    filename[strlen(filename) - 1] = 0;
                }
                char absPath[1024] = {0};
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(absPath, "%s%s", rootPath, filename);
                }
                else
                {
                    sprintf(absPath, "%s/%s", rootPath, filename);
                }
                FILE *f = fopen(absPath, "wb");
                if (f == NULL)
                {
                    const char *resp = "550 Cannot create file\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
                else
                {
                    const char *resp150 = "150 Start transferring\r\n";
                    Send(c, (char *)resp150, strlen(resp150));
                    while (0 == 0)
                    {
                        char block[1024] = {0};
                        int tmp = recv(dataSocket, block, sizeof(block), 0);
                        if (tmp > 0)
                        {
                            fwrite(block, 1, tmp, f);
                        }
                        else
                            break;
                    }
                    close(dataSocket);
                    fclose(f);
                    const char *resp226 = "226 Transferring completed\r\n";
                    Send(c, (char *)resp226, strlen(resp226));
                }
            }
            else if (strncmp(buffer, "DELE ", 5) == 0)
            {
                char *filename = buffer + 5;
                while (filename[strlen(filename) - 1] == '\r' || filename[strlen(filename) - 1] == '\n')
                {
                    filename[strlen(filename) - 1] = 0;
                }
                char absPath[1024] = {0};
                if (rootPath[strlen(rootPath) - 1] == '/')
                {
                    sprintf(absPath, "%s%s", rootPath, filename);
                }
                else
                {
                    sprintf(absPath, "%s/%s", rootPath, filename);
                }

                if (remove(absPath) == 0)
                {
                    const char *resp = "200 File deleted\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
                else
                {
                    const char *resp = "550 Cannot delete file\r\n";
                    Send(c, (char *)resp, strlen(resp));
                }
            }
            else if (strncmp(buffer, "QUIT", 4) == 0)
            {
                break;
            }
            else
            {
                const char *resp = "500 Command not supported\r\n";
                Send(c, (char *)resp, strlen(resp));
            }
            free(buffer);
            buffer = NULL;
        }
        else
        {
            break;
        }
    }
    close(c);
    return NULL;
}

void loadUsers()
{
    FILE *f = fopen("ftpusers.txt", "rt");
    while (!feof(f))
    {
        listUsers = (USERNAME *)realloc(listUsers, (userCount + 1) * sizeof(USERNAME));
        listPsws = (PASSWORD *)realloc(listPsws, (userCount + 1) * sizeof(PASSWORD));
        fscanf(f, "%s%s", listUsers[userCount], listPsws[userCount]);
        userCount += 1;
    }
    fclose(f);
}

int main()
{
    loadUsers();
    rootPath = (char *)calloc(2, sizeof(char));
    strcpy(rootPath, "/");

    SOCKADDR_IN myaddr, caddr;
    int clen = sizeof(caddr);
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(5000);
    myaddr.sin_addr.s_addr = 0;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(s, (SOCKADDR *)&myaddr, sizeof(myaddr));
    listen(s, 10);
    while (0 == 0)
    {
        int c = accept(s, (SOCKADDR *)&caddr, (socklen_t *)&clen);
        int *arg = (int *)calloc(1, sizeof(int));
        *arg = c;
        pthread_t tid;
        pthread_create(&tid, NULL, ClientThread, (void *)arg);
    }
}