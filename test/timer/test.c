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
	initTimer(loop_timer, 0);
	double last_time = getElapsedTime(loop_timer);
    while (i < n_samples) {
        waitUntilNextLoop(loop_timer);        
		double curr_time = getElapsedTime(loop_timer);
		double loop_dt = curr_time - last_time;
		last_time = curr_time;
        if (i > 0) { avg_loop_time += loop_dt; }
        i++;
    }

    printf("Average loop time (s): %f\n", avg_loop_time / (n_samples - 1));
    printf("Average frequency (Hz): %f\n", (n_samples - 1) / avg_loop_time);
}
