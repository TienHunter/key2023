#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
int S = 0;
int TRUTH = 0;
pthread_mutex_t* mutex = NULL;

void* AddingThread(void* arg)
{
    int* tmp = (int*)arg;
    int i = *tmp;
    pthread_mutex_lock(mutex);
    S = S + i;
    pthread_mutex_unlock(mutex);
    free(tmp);
    return NULL;
}

int main()
{
    int N;
    printf("N = ");
    scanf("%d", &N);
    pthread_t* tid = (pthread_t*)calloc(N, sizeof(pthread_t));
    mutex = (pthread_mutex_t*)calloc(1, sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);

    for (int i = 0;i < N;i++)
    {
        TRUTH += i + 1;
        int* tmp = (int*)calloc(1, sizeof(int));
        *tmp = i + 1;
        pthread_create(&(tid[i]), NULL, AddingThread, tmp);
    }

    int* status = NULL;
    for (int i = 0;i < N;i++)
    {
        pthread_join(tid[i], (void**)&status);
    }
    printf("%d == %d\n", TRUTH, S);
    pthread_mutex_destroy(mutex);
}