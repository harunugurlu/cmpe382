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
int curr_size = 0; // The current number of elements in the BUFFER
int count = 0; // Will be used as the current index of the "records" in the PASSWORDS table
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

PASSWORDS passwd_table;

void* reader(void* args);
void* writer(void* args);

void generate_passwords(int num, int *valid_passwords);
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

    valid_passwords = (int*)malloc(10 * sizeof(int));
    if(valid_passwords == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    passwd_table.records = (record*)malloc(num_valid_threads * sizeof(record)); // Allocating memory for the password table.
    passwd_table.size = num_valid_threads;

    if(passwd_table.records == NULL) {
        perror("Error allocating memory for the passwords table");
        return EXIT_FAILURE;
    }

    for(int i = 0; i < 10; i++) {
        generate_passwords(10, valid_passwords);
    }
    for(int i = 0; i < 10; i++) {
        printf("password[%d]: %d\n", i, valid_passwords[i]);
    }

    pthread_t th[num_threads];

    for(int i = 0; i < (num_readers*2); i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = generate_n_digit_rand_num(5);
        
        record* rc = malloc(sizeof(record));
        rc->th_id = *id;
        if(rc == NULL) {
            perror("Error allocating memory for thread record");
            return EXIT_FAILURE;
        }

        if(i % 2 == 0) { // It assigns passwords to the real readers here
            // printf("real reader creation\n");
            //assign_passwd(&count, *id, &passwd_table);
            rc->passwd = valid_passwords[idx];
            printf("real reader %d created, got the passwd %d\n", rc->th_id, rc->passwd);
            int isCreated = pthread_create(&th[i], NULL, reader, rc);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }
            idx++;
        }
        else {
            // printf("dummy reader creation\n");
            int passwd = generate_n_digit_rand_num(6);
            rc->passwd = passwd;
            printf("dummy reader %d created, got the passwd %d\n", rc->th_id, rc->passwd);
            int isCreated = pthread_create(&th[i], NULL, reader, rc);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }
        }
    }
    for(int i = num_readers*2; i < num_threads; i++) {
        // printf("adasdasdasd\n");
        int* id = (int*)malloc(sizeof(int));
        *id = generate_n_digit_rand_num(5);
        
        record* rc = malloc(sizeof(record));
        rc->th_id = *id;
        if(rc == NULL) {
            perror("Error allocating memory for thread record");
            return EXIT_FAILURE;
        }

        if(i % 2 == 0) { // It assigns passwords to the real readers here
            // printf("real reader creation\n");
            //assign_passwd(&count, *id, &passwd_table);
            rc->passwd = valid_passwords[idx];
            printf("real writer %d created, got the passwd %d\n", rc->th_id, rc->passwd);
            int isCreated = pthread_create(&th[i], NULL, writer, rc);
            if(isCreated != 0) {
                perror("Error creating writer threads");
                return EXIT_FAILURE;
            }
            idx++;
        }
        else {
            // printf("dummy reader creation\n");
            int passwd = generate_n_digit_rand_num(6);
            rc->passwd = passwd;
            printf("dummy writer %d created, got the passwd %d\n", rc->th_id, rc->passwd);
            int isCreated = pthread_create(&th[i], NULL, writer, rc);
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
    record rc = *((record*)args);

    printf("reader %d got passwd %d\n", rc.th_id, rc.passwd);

    int isAuthorized = access_resource(rc.passwd);
    if(isAuthorized) {
        printf("reader %d is authorized\n", rc.passwd);
    }
}

void* writer(void* args) {
    record rc = *((record*)args);

    printf("writer %d got passwd %d\n", rc.th_id, rc.passwd);

    int isAuthorized = access_resource(rc.passwd);
    if(isAuthorized) {
        printf("reader %d is authorized\n", rc.passwd);
    }
}

// Fills up the PASSWORDS table with "num" number of 5 digit passwords for each thread
void assign_passwd(int* num, int id, PASSWORDS* passwd_table) {
    // printf("thread %d is getting assigned\n", id);
    // passwd_table->records[*num].th_id = id;
    // printf("thread %d is assigned with --> %d\n", id);
    // (*num)++;
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

// int access_resource(int passwd) {
//     for(int i = 0; i < passwd_table.size; i++) {
//         if(passwd == passwd_table.records[i].passwd) {
//             return 1;
//         }
//     }
//     return -1;
// }
int access_resource(int passwd) {
    for(int i = 0; i < 10; i++) {
        if(passwd == valid_passwords[i]) {
            return 1;
        }
    }
    return -1;
}

void generate_passwords(int num, int *valid_passwords) {
    for(int i = 0; i < num; i++) {
        int passwd = generate_n_digit_rand_num(6);
        valid_passwords[i] = passwd;
    }
}