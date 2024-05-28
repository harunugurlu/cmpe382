#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void* read(void* args);
void* write(void* args);

int main(int argc, char *argv) {

    if(argc != 2) {
        perror("Not enough arguments");
        return EXIT_FAILURE;
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    int num_threads = (num_readers + num_writers)*2; // *2 to create dummy readers and writers

    pthread_t th[num_threads]; // Temporarily 2 readers 2 writers and 2 dummy readers, 2 dummy writers


    for(int i = 0; i < (num_writers*2); i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        
        pthread_create(&th[i], NULL, read, id);
    }
    for(int i = 0; i < (num_writers*2); i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        
        pthread_create(&th[i], NULL, read, id);
    }
}