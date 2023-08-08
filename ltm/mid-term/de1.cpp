#include <stdio.h>      /* fgets(), printf() */
#include <unistd.h>     /* socket(), connect(), fork(), bind(), sendto(), recvfrom(), sleep() */
#include <stdlib.h>     /* malloc() */
#include <sys/socket.h> /* sockaddr */
#include <sys/types.h>  /* inet_addr() */
#include <netdb.h>      /* inet_addr() */
#include <arpa/inet.h>  /* inet_addr() */
#include <string.h>     /* strlen(), memset() */
#include <signal.h>     /* signal() */
#include <sys/wait.h>   /* wait() */

typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;

void *ClientThread(void *arg)
{
  int *tnp = (int *)arg;
  int c = *tmp;
  free(tmp);

  char buffer[1024];
  while (0 == 0)
  {
    memset(buffer, 0, sizeof(buffer));
    int r = recv(c, buffer, sizeof(buffer) - 1, 0);
    if (r > 0)
    {
      printf("Received:%s\n", buffer);
    }
    else
    {
      break;
    }
  }
  close(c);
}

int main()
{
  // Create a child process to broadcast periodically
  if (folk() == 0)
  {
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    const char *msg = "SERVER?";

    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, on);
    while (0 == 0)
    {
      sendto(s, msg, strlen(msg), (SOCKADDR *)&saddr, sizeof(saddr), 0);
      sleep(5);
    }
  }
  else
  {
    // Parent process
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SOCKADDR_IN saddr, caddr;

    int clen = sizeof(caddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(6000);
    saddr.sin_addr.s_addr = 0;
    bind(s, (SOCKADDR *)&saddr, sizeof(saddr));

    while (0 == 0)
    {
      r = recvfrom(s, buffer, sizeof(buffer) - 1, 0, (SOCKADDR *)&caddr, (socklen_t *)&clen);
      if (r > 0 && strstr(buffer, "ACK;") != NULL)
        break;
    }

    // Extract the port number from the server's message

    short port = (short)atoi(buffer + 4);
    caddr.sin_port = htons(port);

    // Establish a TCP connection to the server
    int c = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(c, (SOCKADDR *)&caddr, sizeof(caddr));

    // Create a thread to receive messages from the server and print out to the display
    pthread_t tid;
    int *tmp = (int *)calloc(1, sizeof(int));
    *tmp = c;
    pthread_create(&tid, NULL, ClientThread, (void *)tmp);

    // Wait for the user to type in and then send the message to the server
    while (0 == 0)
    {
      memset(buffer, sizeof(buffer), 0);
      fgets(buffer, sizeof(buffer) - 1, stdin);
      send(c, buffer, strlen(buffer), 0);
    }
  }
}