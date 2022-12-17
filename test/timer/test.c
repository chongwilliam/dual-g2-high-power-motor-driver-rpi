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

#include "../../include/loop_timer.h"
#include <time.h>
#include <stdio.h>

int main() {

    LoopTimer_t* loop_timer = LoopTimer(2000);
    clock_t start, end;
    double avg_loop_time = 0;
    int n_samples = 1e4;
    int i = 0;

    while (i < n_samples) {
        start = clock();
        setStartTime(loop_timer);
        waitUntilNextLoop(loop_timer);        
        end = clock();
        avg_loop_time += ((double) (end - start)) / CLOCKS_PER_SEC;
        i++;
    }

    printf("Average loop time (s): %f\n", avg_loop_time / n_samples);
    printf("Average frequency (Hz): %f\n", n_samples / avg_loop_time);
}
