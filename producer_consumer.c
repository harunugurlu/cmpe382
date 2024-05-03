#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_THREADS 6
#define BUFFER_SIZE 10

sem_t full;
sem_t empty;
sem_t mutex;

int in = 0;
int out = 0;

int buffer[10] = {0};

void *produce(void *args);
void *consume(void *args);

int main(int argc, char *argv) {
    pthread_t th[NUM_THREADS];
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&mutex, 0, 1);

    srand(time(NULL));

    for(int i = 0; i < NUM_THREADS; i++) {
        if(i % 2 == 0) {
            int *rank = malloc(sizeof(int));
            *rank = i;
            int rc = pthread_create(&th[i], NULL, produce, rank);
            if(rc != 0) {
                perror("Error creating thread");
                return 1;
            }
        }
        if(i % 2 == 1) {
            int *rank = malloc(sizeof(int));
            *rank = i;
            int rc = pthread_create(&th[i], NULL, consume, rank);
            if(rc != 0) {
                perror("Error creating thread");
                return 1;
            }
        }
    }

    for(int i = 0; i < NUM_THREADS; i++) {
        int rc = pthread_join(th[i], NULL);
        if(rc != 0) {
            perror("Error joining thread");
            return 1;
        }
    }

    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&mutex);

    return 0;
}

void *produce(void *args) {
    int rank = *((int*)args);
    while(1) {
        int item = rand() % 1000;
        sem_wait(&empty);
        sem_wait(&mutex);
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE;
        sem_post(&mutex);
        printf("Producer %d produced %d\n", rank, item);
        sem_post(&full);
    }
}

void *consume(void *args) {
    int rank = *((int*)args);
    while(1) {
        sem_wait(&full);
        sem_wait(&mutex);
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        printf("Consumer %d consumed %d\n", rank, item);
        sleep(1);
        sem_post(&mutex);
        sem_post(&empty);
    }
}