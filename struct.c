#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

typedef struct
{
    int id;
    char *message;
} str;

// Thread function that uses data passed as a struct
void *print_message(void *ptr)
{
    str *data = (str *)ptr; // Cast to correct type
    printf("Thread %d says %s\n", data->id, data->message);
    return NULL;
}

int main()
{
    pthread_t threads[2];
    str data[2];

    // Initialize data for each thread
    data[0].id = 1;
    data[0].message = "Hello from Thread 1";
    data[1].id = 2;
    data[1].message = "Hello from Thread 2";

    // Create two threads, each with different data
    for (int i = 0; i < 2; i++)
    {
        if (pthread_create(&threads[i], NULL, print_message, &data[i]) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
    }
}