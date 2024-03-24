#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int fd[2];

    pipe(fd);

    pid = fork();

    if(pid > 0) {
        close(fd[0]);
        while(1) {
            int number = 0;

            printf("Enter a number: ");
            scanf("%d", &number);

            int num_written = write(fd[1], &number, sizeof(int));

            if(number < 0) {
                break;
            }

            if(num_written == -1) {
                printf("error writing to pipe\n");
            }
            sleep(1);
        }
        waitpid(pid, NULL, 0);
    }
    else if(pid == 0) {
        close(fd[1]);

        while(1) {
            int number = 0;

            read(fd[0], &number, sizeof(int));

            printf("Child received: %d\n", number);

            if(number < 0) {
                printf("terminating \n");
                break;
            }
        }
    }

    return 0;
}