/**
 * @file timer.c
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-12-03
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../include/loop_timer.h"

LoopTimer_t* LoopTimer(int frequency)
{
    LoopTimer_t* timer;
    timer = malloc(sizeof(LoopTimer_t));
    timer->frequency = frequency;
    timer->sample_time = 1. / frequency;
    return timer;
}

void setLoopFrequency(LoopTimer_t* timer, int _frequency)
{
    timer->frequency = _frequency;
    timer->sample_time = 1. / _frequency;
}

void setStartTime(LoopTimer_t* timer)
{
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

double getElapsedTime(LoopTimer_t* timer)
{
    double time_taken = (timer->end.tv_sec - timer->start.tv_sec) * 1e9;
    time_taken = (time_taken + (timer->end.tv_nsec - timer->start.tv_nsec)) * 1e-9;
    return time_taken;
}

void waitUntilNextLoop(LoopTimer_t* timer)
{
    double time_taken;
    while (1) {
        clock_gettime(CLOCK_MONOTONIC, &timer->end);
        time_taken = (timer->end.tv_sec - timer->start.tv_sec) * 1e9;
        time_taken = (time_taken + (timer->end.tv_nsec - timer->start.tv_nsec)) * 1e-9;
        if (time_taken > timer->sample_time) {
            break;
        }
    }
}