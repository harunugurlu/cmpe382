#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int nrDigits(int);
int isPrime(int);
void nrPrimes(int num, int* num_primes);

int main() {
    // Process id
    pid_t pid1;
    pid_t pid2;

    // File descriptors for the pipes
    int fd_p1_p2[2];
    int fd_p2_p1[2];
    int fd_p1_p3[2];
    int fd_p3_p1[2];

    int pipe1 = pipe(fd_p1_p2);
    int pipe2 = pipe(fd_p2_p1);
    int pipe3 = pipe(fd_p1_p3);
    int pipe4 = pipe(fd_p3_p1);

    if(pipe1 == -1 || pipe2 == -1 || pipe3 == -1 || pipe4 == -1) {
        perror("Error creating pipes");
        return EXIT_FAILURE;
    }

    // Creating the first child
    pid1 = fork();

    if(pid1 == -1) {
        perror("error forking process");
        return EXIT_FAILURE;
    }

    // In the P1 parent process
    if(pid1 > 0) {
        // Creating the second child.
        pid2 = fork();
        if(pid2 == -1) {
            perror("error forking process");
            return EXIT_FAILURE;
        }
        // In the P1 parent process
        if(pid2 > 0) {
            int fd_numbers = open("numbers.txt", O_RDONLY);

            if(fd_numbers == -1) {
                perror("error opening the file\n");
                
                return EXIT_FAILURE;
            }

            // Closing the unused pipe ends.
            close(fd_p1_p2[0]);
            close(fd_p2_p1[1]);

            while(1) {
                char number_str[6];
                int index = 0;
                int read_num_bytes = 0;
                // Reading 1 character at a time in order to handle different digit numbers
                while(1) {
                    char single_char;
                    read_num_bytes = read(fd_numbers, &single_char, 1);

                    if(read_num_bytes == -1) {
                        perror("error reading integer from the file");
                        return EXIT_FAILURE;
                    }
                    // End of the file
                    else if(read_num_bytes == 0) {
                        printf("End of the file\n");
                        break;
                    }

                    number_str[index] = single_char;
                    index++;

                    if(single_char == '\n')
                        break;

                }
                // End of the file
                if(read_num_bytes == 0) {
                    printf("end of the file %s\n", number_str);
                    break;
                }

                // read() does not add a null terminator '\0' at the end of the buffer, so manually adding it.
                number_str[index] = '\0';

                printf("read the number: %s", number_str);

                int number_int = atoi(number_str);
                printf("number int --> %d\n", number_int);

                write(fd_p1_p2[1], &number_int, sizeof(int));

                //sleep(1);
            }
        }
        // In P3 second child
        // else if(pid2 == 0) {
        //     close(fd_p1_p3[1]);
        //     close(fd_p3_p1[0]);

        //     while(1) {
        //         int num = 0;

        //         int read_num_bytes = read(fd_p1_p3[0], &num, sizeof(int));

        //         if(read_num_bytes == -1) {
        //             perror("P3 read pipe error");
        //             return EXIT_FAILURE;
        //         }

        //         int num_primes = isPrime(num);

        //     }
        // }
    }
    // In P2 first child
    else if(pid1 == 0) {

    }

    return EXIT_SUCCESS;
}

int nrDigits(int num) {

}

int isPrime(int num) {
    
    int num_primes = 0;

    nrPrimes(num, &num_primes);

    return num_primes;
}

void nrPrimes(int num, int* num_primes) {
    
    if(num == 1 || num == 0) {
        return;
    }

    int upper_bound = num / 2;

    for(int i = 2; i <= upper_bound; i++) {
        if(num % i == 0) {
            return;
        }
    }

    *num_primes += 1;
}