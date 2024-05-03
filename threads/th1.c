#include <stdio.h>
#include <pthread.h>

void *threadFun1(void *arg) {
    while(1)
    printf("0");
}
void *threadFun2(void *arg) {
    while(1)
    printf("1");
}

int main() {
    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, threadFun1, NULL);
    pthread_create(&thread2, NULL, threadFun2, NULL);
    pthread_join(thread1, NULL); // It sends the thread back to the ready queue
    pthread_join(thread2, NULL);
    return 0;
}