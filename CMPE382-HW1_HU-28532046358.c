#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

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

    // Pipes for communication between the processes
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
            close(fd_p1_p3[0]);
            close(fd_p3_p1[1]);

            int num_count = 0;
            while(1) {
                char number_str[6];
                int index = 0;
                int read_num_bytes = 0;
                // Reading 1 character at a time in order to handle different numbers with different number of digits.
                while(1) {
                    char single_char;
                    read_num_bytes = read(fd_numbers, &single_char, 1);

                    if(read_num_bytes == -1) {
                        perror("error reading integer from the file");
                        return EXIT_FAILURE;
                    }
                    // End of the file
                    else if(read_num_bytes == 0) {
                        // printf("End of the file\n");
                        break;
                    }

                    number_str[index] = single_char;
                    index++;

                    if(single_char == '\n')
                        break;
                    
                }
                // End of the file

                // read() does not add a null terminator '\0' at the end of the buffer, so manually adding it.
                number_str[index] = '\0';

                int number_int = atoi(number_str);
                num_count++;

                int write_number = write(fd_p1_p3[1], &number_int, sizeof(int));
                int write_number2 = write(fd_p1_p2[1], &number_int, sizeof(int));

                if(read_num_bytes == 0) {
                    number_int = -1;
                    write(fd_p1_p3[1], &number_int, sizeof(int));
                    write(fd_p1_p2[1], &number_int, sizeof(int));
                    break;
                }

                if(write_number == -1 || write_number2 == -1) {
                    perror("P1 parent write error");
                    return EXIT_FAILURE;
                }

                // sleep(1);

            }

            int fd_output = open("output.txt", O_CREAT | O_WRONLY, 0644);

            if(fd_output == -1) {
                perror("P1 parent error creating the output file");
                return EXIT_FAILURE;
            }

            // Wait for P3 to finish
            waitpid(pid1, NULL, 0);

            int num_by_digits[5] = {0};

            
            read(fd_p2_p1[0], num_by_digits, sizeof(num_by_digits));

            for(int i = 0; i < 5; i++) {
                printf("%d digits - %d\n", (i+1), num_by_digits[i]);
                dprintf(fd_output, "%d digits - %d\n", (i+1), num_by_digits[i]);
            }

            waitpid(pid2, NULL, 0);
            
            int num_primes = 0;
            read(fd_p3_p1[0], &num_primes, sizeof(int));

            int num_non_primes = num_count - num_primes;
            
            printf("Primes - %d\n", num_primes);
            printf("Nonprimes - %d\n", num_non_primes);

            dprintf(fd_output, "Primes - %d\n", num_primes);
            dprintf(fd_output, "Nonprimes - %d\n", num_non_primes);
        }
        // In P3 second child
        else if(pid2 == 0) {
            close(fd_p1_p3[1]);
            close(fd_p3_p1[0]);
            int num_primes = 0;
            while(1) {
                int num = 0;

                int read_num_bytes = read(fd_p1_p3[0], &num, sizeof(int));

                if(read_num_bytes == -1) {
                    perror("P3 read pipe error");
                    return EXIT_FAILURE;
                }
                if(num == -1) {
                    break;
                }

                nrPrimes(num, &num_primes);
            }

            int write_number = write(fd_p3_p1[1], &num_primes, sizeof(int));
            if(write_number == -1) {
                perror("P3 child write error");
                return EXIT_FAILURE;
            }
        }
    }
    // In P2 first child
    else if(pid1 == 0) {
        close(fd_p1_p2[1]);
        close(fd_p2_p1[0]);

        int num_by_digits[5] = {0};

         while(1) {
                int num = 0;

                int read_num_bytes = read(fd_p1_p2[0], &num, sizeof(int));

                if(read_num_bytes == -1) {
                    perror("P3 read pipe error");
                    return EXIT_FAILURE;
                }
                if(num == -1) {
                    break;
                }

                int nr_digits = nrDigits(num);

                switch(nr_digits) {
                    case 1:
                    num_by_digits[0] += 1;
                    break;
                    case 2:
                    num_by_digits[1] += 1;
                    break;
                    case 3:
                    num_by_digits[2] += 1;
                    break;
                    case 4:
                    num_by_digits[3] += 1;
                    break;
                    case 5:
                    num_by_digits[4] += 1;
                    break;
                }

            }

            int write_number = write(fd_p2_p1[1], num_by_digits, sizeof(num_by_digits));
            if(write_number == -1) {
                perror("P3 child write error");
                return EXIT_FAILURE;
            }  
    }

    return EXIT_SUCCESS;
}

int nrDigits(int num) {
    if (num == 0) return 1;

    int nr_digits = 0;
    while (num != 0) {
        num /= 10;
        nr_digits++;
    }

    return nr_digits;
}

int isPrime(int num) {
    
    if(num == 1 || num == 0) {
        return 0;
    }

    int upper_bound = num / 2;

    //printf("num is: %d, upper_bound is: %d\n", num, upper_bound);

    for(int i = 2; i <= upper_bound; i++) {
       // printf("i --> %d, num --> %d\n", i, num);
        if(num % i == 0) {
            return 0;
        }
    }

    return 1;
}

void nrPrimes(int num, int* num_primes) {
    
    int is_num_prime = isPrime(num);

    *num_primes += is_num_prime;
    //printf("num_primes --> %d\n", *num_primes);
}