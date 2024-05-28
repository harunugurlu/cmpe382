#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define BUFF_SIZE 100

int *BUFFER; // The global database BUFFER
int curr_size = 0; // The current number of elements in the BUFFER

int *PASSWORDS; // Passwords table

void* reader(void* args);
void* writer(void* args);

void generate_passwords(int num);
int generate_n_digit_rand_num(int digits);

int main(int argc, char *argv[]) {
    printf("argc %d\n", argc);
    if(argc != 3) {
        perror("Invalid number of arguments");
        return EXIT_FAILURE;
    }

    int num_readers = atoi(argv[1]); // Get the number of readers from the command line
    int num_writers = atoi(argv[2]); // Get the number of writers from the command line
    int num_valid_threads = num_readers + num_writers; // Number of real reader and writer threads
    int num_threads = (num_valid_threads)*2; // Number of total threads, *2 to include dummy readers and writers

    printf("num_readers %d, num_writers %d, num_valid_threads %d, num_threads %d\n", num_readers, num_writers, num_valid_threads, num_threads);

    if(num_threads > 20) {
        perror("Too many threads, maximum allowed number of reader/writers is 10");
        return EXIT_FAILURE;
    }

    srand(time(NULL)); // To ensure we get different random numbers each time the program is run

    BUFFER = (int*)malloc(BUFF_SIZE * sizeof(int)); // Allocating memory for the buffer
    if(BUFFER == NULL) {
        perror("Error allocating memory for the buffer");
        return EXIT_FAILURE;
    }

    PASSWORDS = (int*)malloc(num_valid_threads * sizeof(int)); // Allocating memory for the password table.
    if(PASSWORDS == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    generate_passwords(num_valid_threads);

    for(int i = 0; i < num_valid_threads; i++) {
        printf("PASSWORDS[%d] = %d\n", i, PASSWORDS[i]);
    }

    pthread_t th[num_threads];

    for(int i = 0; i < (num_readers*2); i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        
        int isCreated = pthread_create(&th[i], NULL, reader, id);
        if(isCreated != 0) {
            perror("Error creating writer threads");
            return EXIT_FAILURE;
        }
    }
    for(int i = 0; i < (num_writers*2); i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        
        int isCreated = pthread_create(&th[i], NULL, writer, id);
        if(isCreated != 0) {
            perror("Error creating reader threads");
            return EXIT_FAILURE;
        }
    }

    // for(int i = 0; i < num_threads; i++) {
    //     pthread_join
    // }
}

void* reader(void* args) {
    
}

void* writer(void* args) {

}

// Fills up the PASSWORDS table with "num" number of 5 digit passwords
void generate_passwords(int num) {
    for(int i = 0; i < num; i++) {
        int pass = generate_n_digit_rand_num(5);
        PASSWORDS[i] = pass;
    }
}

// Generates a "digits" number of digits random integer
int generate_n_digit_rand_num(int digits) {
    
    if(digits <= 0) {
        printf("digits cannot be less than 0");
        return -1;
    }

    int lowerBound = pow(10, digits - 1); // 10000 for 5 digits
    int upperBound = pow(10, digits) - 1; // 99999 for 5 digits
    

    printf("upperBound %d, lowerBound %d\n", upperBound, lowerBound);

    return (rand() % (upperBound - lowerBound + 1)) + lowerBound;
}
