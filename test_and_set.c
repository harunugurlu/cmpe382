#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int global_count = 0;
int targ = 0;

void *count(void *args);
int TestAndSet(int *target);

int main() {
    pthread_t th[2];

    for(int i = 0; i < 2; i++) {
        // int *rank = malloc(sizeof(int));
        // *rank = i;
        int rc = pthread_create(&th[i], NULL, count, NULL);
        if(rc != 0) {
            perror("Error creating thread");
            return 1;
        }
    }

    for(int i = 0; i < 2; i++) {
        int rc = pthread_join(th[i], NULL);
        if(rc != 0) {
            perror("Error joining thread");
        }
    }

    printf("global_count is %d\n", global_count);
}

void *count(void *args) {
    int j = 0;
    do {
        while(TestAndSet(&targ));
        global_count++;
        targ = 0;
        j++;
    }while(j < 10000);
}

int TestAndSet(int *target) {
    int rv = *target;
    *target = 1;
    return rv;
}