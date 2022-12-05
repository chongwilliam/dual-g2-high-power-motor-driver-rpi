/**
 * @file test.c
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <signal.h>

pthread_t t_id[2];
volatile sig_atomic_t terminate_flag = 0;

void sighandler(int sig) { terminate_flag = 1; }

void* func1(void* args)
{
    while (1) {
        if (terminate_flag) {
            printf("Thread 1 Terminate Motor\n");
        }
    }
}

void* func2(void* args)
{
    while (1) {
        if (terminate_flag) {
            printf("Thread 2 Terminate Motor\n");
        }
    }
}

int main() {

    signal(SIGINT, &sighandler);

    pthread_create(&t_id[0], NULL, func1, NULL);
    pthread_create(&t_id[1], NULL, func2, NULL);

    // pthread_join(t_id[0], NULL);
    while (1) {
        if (terminate_flag) {
            printf("Terminate Motor 1\n");
            printf("Terminate Motor 2\n");
            break;
        }
    }
}