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

    LoopTimer_t* loop_timer = LoopTimer(3);
    clock_t start, end;
    double avg_loop_time = 0;
    int n_samples = 10;
    int i = 0;
	initTimer(loop_timer, 0);
	double last_time = getElapsedTime(loop_timer);
    while (i < n_samples) {
//        start = clock();
        // setStartTime(loop_timer);
//		printf("Start Time: %f\n", timespec_to_double(loop_timer->t_curr));
//		printf("Next Time:  %f\n", timespec_to_double(loop_timer->t_next));
        waitUntilNextLoop(loop_timer);        
		double curr_time = getElapsedTime(loop_timer);
		double loop_dt = curr_time - last_time;
//		printf("loop dt: %f\n", loop_dt);
		last_time = curr_time;
//        end = clock();
//        avg_loop_time += ((double) (end - start)) / CLOCKS_PER_SEC;
		avg_loop_time += loop_dt;
        i++;
    }

    printf("Average loop time (s): %f\n", avg_loop_time / n_samples);
    printf("Average frequency (Hz): %f\n", n_samples / avg_loop_time);
}
