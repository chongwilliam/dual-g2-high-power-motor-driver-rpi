/**
 * @file loop_timer.h
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-12-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _LOOPTIMER_H
#define _LOOPTIMER_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

typedef struct timespec timespec_t;

struct LoopTimer_s
{
	timespec_t t_start;
    timespec_t t_curr;
    timespec_t t_next;
	timespec_t t_loop;
    int frequency;
	double sample_time;
};

typedef struct LoopTimer_s LoopTimer_t;

LoopTimer_t* LoopTimer(int frequency);
double timespec_to_double(const timespec_t t_ret);
void initTimer(LoopTimer_t* timer, unsigned int initial_wait_nanoseconds);
void getCurrentTime(timespec_t* t);
double getElapsedTime(LoopTimer_t* timer);
double getLoopTime(LoopTimer_t* timer);
void nanoSleepUntil(const timespec_t t_next);
void waitUntilNextLoop(LoopTimer_t* timer);

#endif

