#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <semaphore.h>

#define BUFF_SIZE 100

int *BUFFER; // The global database BUFFER
int *valid_passwords; // PASSWORDS table
int *dummy_passwords; // dummy passwords table
int curr_size = 0; // The current number of elements in the BUFFER
int count_valid = 0; // Will be used as the current index of the "records" in the PASSWORDS table
int count_dummy = 0; // Will be used as the current index of the "records" in the PASSWORDS table
int idx = 0;

sem_t mutex;
sem_t wrt;
int readcount;

typedef struct { // this represents a key-value pair in the registered threads table. Each valid thread will have a record here.
    int th_id;
    int passwd;
} record;

typedef struct { // this is the registered threads table. It has a record pointer to be used as an array.
    record* records;
    int size;
} PASSWORDS;

typedef struct {
    int th_id;
    int is_valid;
} th_args;

PASSWORDS* passwd_table;

void* reader(void* args);
void* writer(void* args);

void generate_passwords(int num, int *valid_passwords);
void generate_dummy_passwords(int num, int *dummy_passwords);
void assign_passwd(int id, int isValid);

int get_passwd(int id); // Returns a valid passwd if the given id has a record in the PASSWORDS table
int access_resource(int passwd); // Returns TRUE if the given passwd is valid
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

    valid_passwords = (int*)malloc(10 * sizeof(int));
    if(valid_passwords == NULL) {
        perror("Error allocating memory for the valid passwords table");
        return EXIT_FAILURE;
    }

    dummy_passwords = (int*)malloc(10 * sizeof(int));
    if(valid_passwords == NULL) {
        perror("Error allocating memory for the dummy passwords table");
        return EXIT_FAILURE;
    }

    passwd_table = (PASSWORDS*)malloc(sizeof(PASSWORDS));
    if(passwd_table == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    passwd_table->records = (record*)malloc(num_threads * sizeof(record)); // Allocating memory for the password table.
    if(passwd_table->records == NULL) {
        perror("Error allocating memory for the passwords table records");
        return EXIT_FAILURE;
    }

    passwd_table->size = num_valid_threads;

    if(passwd_table->records == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    // for(int i = 0; i < 10; i++) {
    //     generate_passwords2(10);
    // }
    for(int i = 0; i < 10; i++) {
        generate_passwords(10, valid_passwords);
    }
    for(int i = 0; i < 10; i++) {
        generate_dummy_passwords(10, dummy_passwords);
    }
    for(int i = 0; i < 10; i++) {
        printf("valid password[%d]: %d\n", i, valid_passwords[i]);
    }
    for(int i = 0; i < 10; i++) {
        printf("dummy password[%d]: %d\n", i, dummy_passwords[i]);
    }

    pthread_t th[num_threads];
    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);

    for(int i = 0; i < num_readers * 2; i++) {
        th_args* my_args = malloc(sizeof(th_args));
        if(my_args == NULL) {
            perror("Error allocating memory for thread arguments");
            return EXIT_FAILURE;
        }
        // printf("reader thread has id %d\n", i);
        my_args->th_id = i;
        my_args->is_valid = (i % 2 == 0) ? 1 : 0; // Alternate between valid and dummy

        if(pthread_create(&th[i], NULL, reader, my_args) != 0) {
            perror("Error creating reader threads");
            return EXIT_FAILURE;
        }
    }

    // Create writer threads
    for(int i = num_readers * 2; i < num_threads; i++) {
        th_args* my_args = malloc(sizeof(th_args));
        if(my_args == NULL) {
            perror("Error allocating memory for thread arguments");
            return EXIT_FAILURE;
        }
        // printf("writer thread has id %d\n", i);
        my_args->th_id = i;
        my_args->is_valid = (i % 2 == 0) ? 1 : 0; // Alternate between valid and dummy

        if(pthread_create(&th[i], NULL, writer, my_args) != 0) {
            perror("Error creating writer threads");
            return EXIT_FAILURE;
        }
    }

    // The main thread waits for all threads to complete
    for(int i = 0; i < num_threads; i++) {
        pthread_join(th[i], NULL);
    }

    free(passwd_table->records);
    free(BUFFER);
    free(valid_passwords);
    free(dummy_passwords);
    sem_destroy(&mutex);
    sem_destroy(&wrt);
}

void* reader(void* args) {
    th_args my_args = *((th_args*)args);
    //int id = *((int*)args);

    sem_wait(&mutex);
    assign_passwd(my_args.th_id, my_args.is_valid);
    sem_post(&mutex);
    

    sem_wait(&mutex);
    int my_passwd = get_passwd(my_args.th_id);
    sem_post(&mutex);

    int isAuthorized = access_resource(my_passwd);
    printf("reader %d with passwd %d authorization %d\n", my_args.th_id, my_passwd, isAuthorized);
    if(isAuthorized) {
        printf("reader %d is AUTHORIZED\n", my_args.th_id);
    }
    else {
        printf("reader %d is UNAUTHORIZED\n", my_args.th_id);
    }
}

void* writer(void* args) {
    th_args my_args = *((th_args*)args);
    //int id = *((int*)args);

    sem_wait(&mutex);
    assign_passwd(my_args.th_id, my_args.is_valid);
    sem_post(&mutex);
    

    sem_wait(&mutex);
    int my_passwd = get_passwd(my_args.th_id);
    sem_post(&mutex);

    int isAuthorized = access_resource(my_passwd);
    printf("writer %d with passwd %d authorization %d\n", my_args.th_id, my_passwd, isAuthorized);
    if(isAuthorized) {
        printf("writer %d is AUTHORIZED\n", my_args.th_id);
    }
    else {
        printf("writer %d is UNAUTHORIZED\n", my_args.th_id);
    }
}

// Fills up the PASSWORDS table with "num" number of 5 digit passwords for each thread
void assign_passwd(int id, int isValid) {
    // printf("thread %d is getting assigned\n", id);
    if(isValid) {
        passwd_table->records[count_valid].th_id = id;
        passwd_table->records[count_valid].passwd = valid_passwords[count_valid];
        // printf("thread %d is assigned with --> %d\n", id, passwd_table->records[count_valid].passwd);
        count_valid++;
    }
    else {
        passwd_table->records[count_valid].th_id = id;
        passwd_table->records[count_valid].passwd = dummy_passwords[count_dummy];
        // printf("thread %d is assigned with --> %d\n", id, passwd_table->records[count_valid].passwd);
        count_dummy++;
        count_valid++;
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
    

    //printf("upperBound %d, lowerBound %d\n", upperBound, lowerBound);

    return (rand() % (upperBound - lowerBound + 1)) + lowerBound;
}

int get_passwd(int id) {
    if(id == -1) {
        return -1;
    }
    for(int i = 0; i < 10; i++) {
        if(id == passwd_table->records[i].th_id) {
            return passwd_table->records[i].passwd;
        }
    }
    return -1;
}

int access_resource2(int passwd) {
    for(int i = 0; i < passwd_table->size; i++) {
        if(passwd == passwd_table->records[i].passwd) {
            return 1;
        }
    }
    return -1;
}
int access_resource(int passwd) {
    for(int i = 0; i < 10; i++) {
        if(passwd == valid_passwords[i]) {
            return 1;
        }
    }
    return 0;
}

void generate_passwords(int num, int *valid_passwords) {
    for(int i = 0; i < num; i++) {
        int passwd = generate_n_digit_rand_num(6);
        valid_passwords[i] = passwd;
    }
}

void generate_dummy_passwords(int num, int *dummy_passwords) {
    for(int i = 0; i < num; i++) {
        int passwd = generate_n_digit_rand_num(6);
        dummy_passwords[i] = passwd;
    }
}

void generate_passwords2(int num) {
    for(int i = 0; i < num; i++) {
        int passwd = generate_n_digit_rand_num(6);
        passwd_table->records[i].passwd = passwd;
    }
}