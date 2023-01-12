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

static inline timespec_t subtract(const timespec_t a, const timespec_t b)
{
	timespec_t dt;
	if (a.tv_nsec - b.tv_nsec < 0) {
		dt.tv_sec = a.tv_sec - b.tv_sec - 1;
		dt.tv_nsec = a.tv_nsec - b.tv_nsec + 1e9;
	} else {
		dt.tv_sec = a.tv_sec - b.tv_sec;
		dt.tv_nsec = a.tv_nsec - b.tv_nsec;
	}
	return dt;
}

static inline int lessThan(const timespec_t lhs, const timespec_t rhs) {
	if (lhs.tv_sec == rhs.tv_sec) {
		return lhs.tv_nsec < rhs.tv_nsec;
	}
	return lhs.tv_sec < rhs.tv_sec;
}

static inline timespec_t addTo(timespec_t t, unsigned int nsecs) {
	while (nsecs >= 1e9) {
		t.tv_sec++;
		nsecs -= 1e9;
	}
	t.tv_nsec += nsecs;
	return t;
}

double timespec_to_double(const timespec_t t) {
	return t.tv_sec + 1e-9 * (double)(t.tv_nsec);
}

LoopTimer_t* LoopTimer(int frequency)
{
    LoopTimer_t* timer;
    timer = malloc(sizeof(LoopTimer_t));
    timer->frequency = frequency;
    timer->sample_time = 1e9 / frequency;  // ns
    return timer;
}

void initTimer(LoopTimer_t* timer, unsigned int initial_wait_nanoseconds)
{
	getCurrentTime(&timer->t_next);
//	timer->t_next = addTo(timer->t_next, initial_wait_nanoseconds);
	timer->t_start = timer->t_next;
}

void getCurrentTime(timespec_t* t)
{
	clock_gettime(CLOCK_MONOTONIC_RAW, t);
}

double getElapsedTime(LoopTimer_t* timer)
{
 //   clock_gettime(CLOCK_MONOTONIC, &timer->end);
 //   double time_taken = (timer->end.tv_sec - timer->start.tv_sec) * 1e9;
 //   time_taken = (time_taken + (timer->end.tv_nsec - timer->start.tv_nsec)) * 1e-9;
 //   return 
	struct timespec t_now;
	getCurrentTime(&t_now);
	timespec_t t = subtract(t_now, timer->t_start);
	return timespec_to_double(t);
}

double getLoopTime(LoopTimer_t* timer)
{
	return timespec_to_double(timer->t_loop);
}

// Absolute time w/ reference to CLOCK_MONOTONIC 
void nanoSleepUntil(const timespec_t t_next) 
{
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t_next, NULL);
}

void waitUntilNextLoop(LoopTimer_t* timer)
{
	clock_gettime(CLOCK_MONOTONIC, &timer->t_curr);
	timespec_t dt = subtract(timer->t_next, timer->t_curr);
	printf("dt at start: %f\n", timespec_to_double(dt));
	printf("t_next: %f\n", timespec_to_double(timer->t_next));

	if (lessThan(timer->t_curr, timer->t_next)) {
		printf("Wait\n");
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timer->t_next, NULL);
	}
	timer->t_next = addTo(timer->t_next, timer->sample_time);
}
