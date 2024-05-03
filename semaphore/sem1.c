#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

sem_t mutex;
int sum = 0;

void *th_sum(void *args)
{
    int i;
    int local_sum = 0;

    for (int i = 0; i < 1000000; i++)
    {
        local_sum += 1;
    }
    sem_wait(&mutex);
    sum += local_sum;
    sem_post(&mutex);
}

int main()
{
    pthread_t th[10];
    sem_init(&mutex, 0, 1);
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&th[i], NULL, th_sum, NULL);
    }
    for (int i = 0; i < 10; i++)
    {
        pthread_join(th[i], NULL);
    }
    printf("the result is %d\n", sum);
}