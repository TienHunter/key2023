#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
struct sigaction newaction, oldaction;
void handler(int signum)
{
    if (signum == SIGINT)
    {
        printf("Ctrl+C pressed\n");
        sigaction(signum, &oldaction, NULL);
    }
    exit(0);
}
int main()
{
    newaction.sa_handler = handler;
    sigemptyset(&newaction.sa_mask);
    newaction.sa_flags = 0;
    sigaction(SIGINT, &newaction, &oldaction);
    while (0 == 0)
    {
        sleep(1);
    }
}