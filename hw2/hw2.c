#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>    // Include for directory handling
#include <string.h>    // Include for string manipulation
#include <errno.h>     // Include for error handling
#include <ctype.h>
#include <unistd.h>

struct dirent *entry; // Represents a directory entry. In our case it will represent txt files.
DIR *dir; // Represents the directory we will read (myDir)
sem_t sem; // The semaphore that will restrict the number of simultaneously active threads
sem_t mutex; // A mutual exclusion semaphore
char **file_names; // Will use as a dynamic array for file names
int file_count;
int *file_descriptors;
int current_file_index = 0;

struct file_data {
    int fd; // File descriptor
    int index; // Index can be used for debugging or further management
};

void *check_prime(void *args);
int is_prime(int num);

int main(int argc, char *argv[]) {

    // Checking if the user has entered the right number of arguments
    if(argc != 3) {
        fprintf(stderr, "Usage: <directory_name> <number_of_threads>\n");
        return EXIT_FAILURE;
    }

    // Reading the input and converting the string representing the number of threads to an integer
    int thread_num = atoi(argv[2]);
    if (thread_num <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        return EXIT_FAILURE;
    }

    // Declaring thread identifiers
    pthread_t th[thread_num];

    // Initialize the semaphore
    if(sem_init(&sem, 0, thread_num) != 0) {
        perror("Error initializing the semaphore");
        return EXIT_FAILURE;
    }
    // Initialize the mutex semaphore
    if(sem_init(&mutex, 0, 1) != 0) {
        perror("Error initializing the semaphore");
        return EXIT_FAILURE;
    }
    
    // Reading the input, representing the directory name
    char *myDir = argv[1];

    dir = opendir(myDir);

    if(dir == NULL) {
        perror("error opening the file\n");
        return EXIT_FAILURE;
    }


    file_count = 0;
    file_descriptors = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // If the files are regular files, we will add them to the array
            char *file_path = malloc(strlen(myDir) + strlen(entry->d_name) + 2); // Declaring a string to form file path, the +2 is for the slash and the null terminator
            if (file_path == NULL) {
                perror("Failed to allocate memory for file path");
                return EXIT_FAILURE;
            }

            // Constructing the full file path (myDir/file_name)
            snprintf(file_path, strlen(myDir) + strlen(entry->d_name) + 2, "%s/%s", myDir, entry->d_name);

            // Store the file path in the array
            file_names = realloc(file_names, (file_count + 1) * sizeof(char*));
            if (file_names == NULL) {
                perror("Memory reallocation failed");
                return EXIT_FAILURE;
            }

            file_names[file_count] = strdup(entry->d_name);

            int fd_num = open(file_path, O_RDONLY); // Opening the files as we read them so the threads can access.
            if(fd_num == -1) {
                perror("error opening the file\n");
                close(fd_num);
                return EXIT_FAILURE;
            }

            file_descriptors = realloc(file_descriptors, (file_count + 1) * sizeof(int));
            if (file_descriptors == NULL) {
                perror("Failed to allocate memory for file descriptors");
                return EXIT_FAILURE;
            }
            file_descriptors[file_count++] = fd_num;
            free(file_path); // Free file_path after use
        }

    }

    closedir(dir);

    for(int i = 0; i < thread_num; i++) {
        int *rank = malloc(sizeof(int));
        *rank = i;
        int rd = pthread_create(&th[i], NULL, check_prime, rank);
        if(rd != 0) {
            perror("Error creating threads");
            return EXIT_FAILURE;
        }
    }

    for(int i = 0; i < thread_num; i++) {
        int rd = pthread_join(th[i], NULL);
        if(rd != 0) {
            perror("Error joining threads");
            return EXIT_FAILURE;
        }
    }

        // Cleanup
    for (int i = 0; i < file_count; i++) {
        close(file_descriptors[i]); // Close all file descriptors
        free(file_names[i]);
    }

    free(file_descriptors);
    sem_destroy(&sem);

    return EXIT_SUCCESS;
}

void *check_prime(void *args) {
    int rank = *((int*)args);
    int prime_count = 0;
    int local_file_index = 0;
    sem_wait(&sem);
    while(1) {
        sem_wait(&mutex); // current_file_index is a critical section, restrict the access to only 1 thread at a time
        if (current_file_index >= file_count) { // It means there are no more files to process
            sem_post(&mutex);
            break; 
        }
        local_file_index = current_file_index++; // Each thread fetches the next available file index
        sem_post(&mutex);

        int fd = file_descriptors[local_file_index];

        // Process the file
        int num = 0;
        int count = 0;

        FILE *stream = fdopen(fd, "r");
        if (!stream) {
            perror("fdopen failed");
            close(fd);
        return NULL;
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        char *token;
        const char *delim = " \t\r\n"; // Delimiters include space, tab, carriage return, and newline

        while ((read = getline(&line, &len, stream)) != -1) {
            token = strtok(line, delim);
            while (token != NULL) {
                int num = atoi(token);
                if (is_prime(num)) {
                    prime_count++;
                }
                token = strtok(NULL, delim);  // Continue tokenizing the same line
            }
        }

        close(fd); // Close file descriptor when done
        printf("Thread %d has found %d primes in %s\n", rank, prime_count, file_names[local_file_index]);
    }
    sem_post(&sem);

    free(args);
    return NULL;
}

// Check if the given number is prime
int is_prime(int num) {
    if(num == 1 || num == 0) {
        return 0;
    }

    int upper_bound = num / 2;

    for(int i = 2; i <= upper_bound; i++) {
        if(num % i == 0) {
            return 0;
        }
    }

    return 1;
}