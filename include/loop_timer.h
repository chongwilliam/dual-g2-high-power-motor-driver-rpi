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
#include <time.h>

struct LoopTimer_s
{
    struct timespec start;
    struct timespec end;
    int frequency;
    double sample_time;
};

typedef struct LoopTimer_s LoopTimer_t;

LoopTimer_t* LoopTimer(int frequency);

void setLoopFrequency(LoopTimer_t* timer, int _frequency);
void setStartTime(LoopTimer_t* timer);
double getElapsedTime(LoopTimer_t* timer);
void waitUntilNextLoop(LoopTimer_t* timer);

#endif