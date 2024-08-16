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
int *dummy_passwords; // Dummy passwords
int curr_size = 0; // The current number of elements in the BUFFER
int count_valid = 0; // Will be used as the current index for the "records", and for the valid passwords.
int count_dummy = 0; // Will be used as the current index for the dummy passwords
int count_record = 0; // Will be used as the current index for the dummy passwords
int readcount = 0; // Integer to count the number of active readers
int in = 0; // Index for the next write operation

sem_t mutex; // Binary semaphore for mutual exclusion to the buffer
sem_t mutex_pass; // Binary semaphore for mutual exclusion to the password table
sem_t wrt; // Binary semaphore for mutual exclusion to the buffer for writers
pthread_barrier_t barrier_reader;
pthread_barrier_t barrier_writer;

typedef struct { // This represents a key-value pair in the registered threads table. Each valid thread will have a record here.
    int th_id;
    int passwd;
} record;

typedef struct { // This is the registered threads table. It has a record pointer to be used as an array.
    record* records;
    int size;
} passwd_table;

typedef struct { // To pass multiple arguments to threads
    int th_id;
    int is_valid;
} th_args;

passwd_table* PASSWORDS;

void* reader(void* args);
void* writer(void* args);

void generate_passwords(int num, int *valid_passwords);
void generate_dummy_passwords(int num, int *dummy_passwords);
void assign_passwd(int id, int isValid);
void buffer_operation_result(int th_id, int isValid, int role, int value);

int get_passwd(int id); // Returns a valid password if the given id has a record in the PASSWORDS table
int access_resource(int passwd); // Returns TRUE if the given password is valid
int generate_n_digit_rand_num(int digits);

int main(int argc, char *argv[]) {
    if(argc != 3) {
        perror("Invalid number of arguments");
        return EXIT_FAILURE;
    }

    int num_readers = atoi(argv[1]); // Get the number of readers from the command line
    int num_writers = atoi(argv[2]); // Get the number of writers from the command line
    int num_valid_threads = num_readers + num_writers; // Number of real reader and writer threads
    int num_threads = (num_valid_threads)*2; // Number of total threads, *2 to include dummy readers and writers

    if(num_threads > 20) {
        perror("Too many threads, maximum allowed number of reader/writers is 10");
        return EXIT_FAILURE;
    }

    srand(time(NULL)); // To ensure we get different random numbers each time the program is run

    // Necessary memory allocations for the shared variables
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
    if(dummy_passwords == NULL) {
        perror("Error allocating memory for the dummy passwords table");
        return EXIT_FAILURE;
    }

    PASSWORDS = (passwd_table*)malloc(sizeof(passwd_table));
    if(PASSWORDS == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    PASSWORDS->records = (record*)malloc(num_threads * sizeof(record)); // Allocating memory for the password table.
    if(PASSWORDS->records == NULL) {
        perror("Error allocating memory for the passwords table records");
        return EXIT_FAILURE;
    }

    PASSWORDS->size = num_threads;

    // Generate real and dummy passwords
    generate_passwords(10, valid_passwords);
    generate_dummy_passwords(10, dummy_passwords);

    // Thread and semaphore identifier initialization
    pthread_t th[num_threads];
    pthread_barrier_init(&barrier_reader, NULL, num_readers);
    pthread_barrier_init(&barrier_writer, NULL, num_writers);
    sem_init(&mutex, 0, 1);
    sem_init(&mutex_pass, 0, 1);
    sem_init(&wrt, 0, 1);

    printf("Thread No   Validity(real/dummy)   Role(reader/writer)   Value read/written\n");

    for(int i = 0; i < num_readers * 2; i++) {
        th_args* my_args = malloc(sizeof(th_args)); // Allocate memory for thread arguments
        if(my_args == NULL) {
            perror("Error allocating memory for thread arguments");
            return EXIT_FAILURE;
        }

        my_args->th_id = i;
        my_args->is_valid = (i % 2 == 0) ? 1 : 0; // Switch between valid and dummy readers

        if(pthread_create(&th[i], NULL, reader, my_args) != 0) {
            perror("Error creating reader threads");
            return EXIT_FAILURE;
        }
    }

    // Create writer threads
    for(int i = num_readers * 2; i < num_threads; i++) {
        th_args* my_args = malloc(sizeof(th_args)); // Allocate memory for thread arguments
        if(my_args == NULL) {
            perror("Error allocating memory for thread arguments");
            return EXIT_FAILURE;
        }

        my_args->th_id = i;
        my_args->is_valid = (i % 2 == 0) ? 1 : 0; // Switch between valid and dummy writers

        if(pthread_create(&th[i], NULL, writer, my_args) != 0) {
            perror("Error creating writer threads");
            return EXIT_FAILURE;
        }
    }

    // The main thread waits for all threads to complete
    for(int i = 0; i < num_threads; i++) {
        pthread_join(th[i], NULL);
    }

    // Free the allocated memories and destroy semaphores
    free(PASSWORDS->records);
    free(PASSWORDS);
    free(BUFFER);
    free(valid_passwords);
    free(dummy_passwords);
    sem_destroy(&mutex);
    sem_destroy(&mutex_pass);
    sem_destroy(&wrt);

    return EXIT_SUCCESS;
}

void* reader(void* args) {
    th_args my_args = *((th_args*)args); // Get the arguments

    // Assign a password to the thread. Access to the PASSWORDS table is mutually exclusive
    sem_wait(&mutex_pass);
    assign_passwd(my_args.th_id, my_args.is_valid);
    sem_post(&mutex_pass);

    // pthread_barrier_wait(&barrier_reader);
    int i = 0;
    while (i < 5) { // Each reader performs 5 read operations
        // Get its assigned password
        i++;
        sleep(1);
        sem_wait(&mutex_pass);
        int my_passwd = get_passwd(my_args.th_id);
        sem_post(&mutex_pass);

        // sem_wait(&mutex_pass);
        int isAuthorized = access_resource(my_passwd); // Check if the thread can access the BUFFER
        // sem_post(&mutex_pass);
        if(isAuthorized) {
            // printf("*--------------reader is authorized\n");
            // Update the active reader count
            sem_wait(&mutex);
            readcount++;
            if(readcount == 1) {
                sem_wait(&wrt); // Prevent writers from writing when readers are active
            }
            sem_post(&mutex);

            // Read from the BUFFER when there is at least 1 item.
            if (curr_size > 0) {
                int read_from = rand() % curr_size; // Generate a valid random index
                int item = BUFFER[read_from];
                buffer_operation_result(my_args.th_id, my_args.is_valid, 1, item);
            } else {
                buffer_operation_result(my_args.th_id, my_args.is_valid, 1, -1); // BUFFER is empty
            }

            sem_wait(&mutex);
            readcount--;
            if(readcount == 0) {
                sem_post(&wrt);
            }
            sem_post(&mutex);
        }
        else {
            buffer_operation_result(my_args.th_id, my_args.is_valid, 1, -1);
        }
    }
    return NULL;
}

void* writer(void* args) {
    th_args my_args = *((th_args*)args); // Get the arguments

    // Assign a password to the thread. Access to the PASSWORDS table is mutually exclusive
    sem_wait(&mutex_pass);
    assign_passwd(my_args.th_id, my_args.is_valid);
    sem_post(&mutex_pass);

    // pthread_barrier_wait(&barrier_writer);
    int i = 0;
    while (i < 5) { // Each writer performs 5 write operations
        // Get its assigned password
        i++;
        sleep(1);
        sem_wait(&mutex_pass);
        int my_passwd = get_passwd(my_args.th_id);
        // printf("writer %d isValid %d got the passwd %d\n", my_args.th_id, my_args.is_valid, my_passwd);
        sem_post(&mutex_pass);

        // sem_wait(&mutex_pass);
        int isAuthorized = access_resource(my_passwd); // Check if the thread can access the BUFFER
        // sem_post(&mutex_pass);
        if(isAuthorized) {
            // Mutual exclusion when writing. Only 1 writer at a time
            sem_wait(&wrt);
            int item = generate_n_digit_rand_num(4);
            BUFFER[in] = item;
            in = (in + 1) % BUFF_SIZE;
            if (curr_size < BUFF_SIZE) {
                curr_size++;
            }
            buffer_operation_result(my_args.th_id, my_args.is_valid, 0, item);
            sem_post(&wrt);
        }
        else {
            buffer_operation_result(my_args.th_id, my_args.is_valid, 0, -1);
        }
    }
    return NULL;
}

// Fills up the PASSWORDS table with "num" number of 5 digit passwords for each thread
void assign_passwd(int id, int isValid) {
    if(isValid) {
        PASSWORDS->records[count_record].th_id = id;
        PASSWORDS->records[count_record].passwd = valid_passwords[count_valid];
        count_valid++;
        count_record++;
    } else {
        PASSWORDS->records[count_record].th_id = id;
        PASSWORDS->records[count_record].passwd = dummy_passwords[count_dummy];
        count_dummy++;
        count_record++;
    }
}

// Generates a "digits" number of digits random integer
int generate_n_digit_rand_num(int digits) {
    if(digits <= 0) {
        printf("Digits cannot be less than or equal to 0\n");
        return -1;
    }

    int lowerBound = pow(10, digits - 1); 
    int upperBound = pow(10, digits) - 1; 

    return (rand() % (upperBound - lowerBound + 1)) + lowerBound;
}

// Get the associated password with the thread with "id"
int get_passwd(int id) {
    for(int i = 0; i < PASSWORDS->size; i++) {
        if(PASSWORDS->records[i].th_id == id) {
            return PASSWORDS->records[i].passwd;
        }
    }
    return -1; 
}

// Tries to access to the BUFFER with the given password
int access_resource(int passwd) {
    for(int i = 0; i < 10; i++) {
        if(passwd == valid_passwords[i]) {
            return 1; 
        }
    }
    return 0; 
}

// Generates valid passwords
void generate_passwords(int num, int *valid_passwords) {
    for(int i = 0; i < num; i++) {
        int passwd = generate_n_digit_rand_num(6);
        valid_passwords[i] = passwd;
    }
}

// Generates dummy passwords
void generate_dummy_passwords(int num, int *dummy_passwords) {
    for(int i = 0; i < num; i++) {
        int passwd = generate_n_digit_rand_num(6);
        dummy_passwords[i] = passwd;
    }
}

// Prints out the operation when the buffer is accessed
void buffer_operation_result(int th_id, int isValid, int role, int value) {
    char* role_str = role == 1 ? "reader" : "writer";
    char* isValid_str = isValid == 1 ? "real" : "dummy";
    printf("%d      %s      %s      %d\n", th_id, isValid_str, role_str, value);
}
