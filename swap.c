#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


int global_count = 0;
int lock = 0;
pthread_mutex_t mutex;

void *count(void *args);
int swap(int* a, int* b);


int main() {
    pthread_t th[2];
    pthread_mutex_init(&mutex, NULL);

    for(int i = 0; i < 2; i++) {
        int rc = pthread_create(&th[i], NULL, count, NULL);
        if(rc != 0) {
            perror("Error creating thread");
            return 1;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        int rc = pthread_join(th[i], NULL);
        if(rc != 0) {
            perror("Error joining thread");
            return 1;
        }
    }
    
    printf("Count is %d\n", global_count);

    return 0;
}

void *count(void *args) {
    int j = 0;
    do {
        int key = 1;
        while(key == 1) {
            pthread_mutex_lock(&mutex);
            swap(&lock, &key);
            pthread_mutex_unlock(&mutex);
        };
        global_count++;
        lock = 0;
        j++;
    } while(j < 10000);
}

int swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}