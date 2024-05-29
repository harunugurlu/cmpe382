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
int count = 0; // Will be used as the current index of the "records" in the PASSWORDS table

//int *PASSWORDS; // Passwords table

typedef struct { // this represents a key-value pair in the passwords table. Each valid thread will have a record here.
    int th_id;
    int passwd;
} record;

typedef struct { // this is the PASSWORDS table. It has a record pointer to be used as an array.
    record* records;
    int size;
} PASSWORDS;

PASSWORDS passwd_table;

void* reader(void* args);
void* writer(void* args);

// void generate_passwords(int num, pthread_t id, PASSWORDS* passwd_table);
void assign_passwd(int* num, int id, PASSWORDS* passwd_table);

int get_passwd(int id); // Returns a valid passwd if the given id has a record in the PASSWORDS table
int access_resource(int passwd); // Returns TRUE if the given passwd is valid
int generate_n_digit_rand_num(int digits);


// TODO: Create the password dictionary and assign password to threads

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

    passwd_table.records = (record*)malloc(num_valid_threads * sizeof(record)); // Allocating memory for the password table.
    passwd_table.size = num_valid_threads;

    if(passwd_table.records == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    pthread_t th[num_threads];

    for(int i = 0; i < (num_readers*2); i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = generate_n_digit_rand_num(5);
        
        if(i % 2 == 0) { // It assigns passwords to the real readers here
            assign_passwd(&count, *id, &passwd_table);
            int isCreated = pthread_create(&th[i], NULL, reader, id);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }

        }
        else {
            int isCreated = pthread_create(&th[i], NULL, reader, NULL);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }
        }
    }
    for(int i = num_readers*2; i < num_threads; i++) {
        printf("adasdasdasd\n");
        int* id = (int*)malloc(sizeof(int));
        *id = generate_n_digit_rand_num(5);
        
        if(i % 2 == 0) { // It assigns passwords to the real writers here
            assign_passwd(&count, *id, &passwd_table);
            int isCreated = pthread_create(&th[i], NULL, writer, id);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }

        }
        else {
            int isCreated = pthread_create(&th[i], NULL, writer, NULL);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }
        }
    }

    // for(int i = 0; i < num_threads; i++) {
    //     pthread_join
    // }
}

void* reader(void* args) {
    int rank = *((int*)args);

    int passwd = get_passwd(rank);
    printf("reader %d got passwd %d\n", rank, passwd);

}

void* writer(void* args) {
    int rank = *((int*)args);

    int passwd = get_passwd(rank);
    printf("writer %d got passwd %d\n", rank, passwd);

}

// Fills up the PASSWORDS table with "num" number of 5 digit passwords for each thread
void assign_passwd(int* num, int id, PASSWORDS* passwd_table) {
        int pass = generate_n_digit_rand_num(5);
        passwd_table->records[*num].th_id = id;
        passwd_table->records[*num].passwd = pass;
        printf("thread %d is assigned with --> %d\n", id, pass);
        (*num)++;
}

// Generates a "digits" number of digits random integer
int generate_n_digit_rand_num(int digits) {
    
    if(digits <= 0) {
        printf("digits cannot be less than 0");
        return -1;
    }

    int lowerBound = pow(10, digits - 1); // 10000 for 5 digits
    int upperBound = pow(10, digits) - 1; // 99999 for 5 digits
    

    //printf("upperBound %d, lowerBound %d\n", upperBound, lowerBound);

    return (rand() % (upperBound - lowerBound + 1)) + lowerBound;
}

int get_passwd(int id) {
    printf("thread %d trying to get password\n", id);
    if(id == -1) {
        return -1;
    }
    for(int i = 0; i < passwd_table.size; i++) {
        if(id == passwd_table.records[i].th_id) {
            return passwd_table.records[i].passwd;
        }
    }
    return -1;
}

int access_resource(int passwd) {
    for(int i = 0; i < passwd_table.size; i++) {
        if(passwd == passwd_table.records[i].passwd) {
            return 1;
        }
    }
    return -1;
}