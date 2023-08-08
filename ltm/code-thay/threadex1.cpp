#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
typedef struct 
{
    int v1;
    int v2;
} THREADARG;

void* mythread(void* arg)
{
    THREADARG* tmp = (THREADARG*)arg;
    int i = tmp->v1;
    free(tmp);
    printf("Hello from worker thread (%x - %ld): %d\n", (unsigned int)pthread_self(), (unsigned long)tmp, i);
    return NULL;
}

int main()
{
    pthread_t tid[10];
    for (int i = 0;i < 10;i++)
    {
        THREADARG* tmp = (THREADARG*)calloc(1, sizeof(THREADARG));
        tmp->v1 = i;
        tmp->v2 = 0;

        pthread_create(&(tid[i]), NULL, mythread, (void*)tmp);
    }
    printf("Hello from main thread\n");
    int* status = NULL;
    for (int i = 0;i < 10;i++)
    {
        pthread_join(tid[i], (void**)&status);
    }
}