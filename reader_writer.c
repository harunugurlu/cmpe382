#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define BUFFER_SIZE 100

sem_t mutex;
sem_t wrt;
int readCount = 0;

int buffer[BUFFER_SIZE] = {0};
int in = 0;
int out = 0;

void *write(void *args);
void *read(void *args);

int main() {
    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);
    pthread_t th[6];

    srand(time(NULL));

    for(int i = 0; i < 6; i++) {
        if(i % 2 == 0) {
            int *rank = malloc(sizeof(int));
            *rank = i;
            pthread_create(&th[i], NULL, write, rank);
        }
        else {
            int *rank = malloc(sizeof(int));
            *rank = i;
            pthread_create(&th[i], NULL, read, rank);
        }
    }
    
    for(int i = 0; i < 6; i++) {
        pthread_join(th[i], NULL);
    }

    return 0;
}

void *write(void *args) {
    while(1) {
        sem_wait(&wrt);
        int item = rand() % 100;
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        sem_post(&wrt);
    }
}

void *read(void *args) {
    while(1) {
        sem_wait(&mutex);
        readCount++;
        if(readCount == 1) sem_wait(&wrt);
        sem_post(&mutex);
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        sem_wait(&mutex);
        readCount--;
        if(readCount == 0) sem_post(&wrt);
        sem_post(&mutex);
    }
}