/**
 * @file test.c
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-11-23
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/*
    Each pi will be running 4 threads:
        - Motor channel 0 
            - Read position, and compute PID output ("speed") to the PWM
        - Motor channel 1
        - Encoder channel 0
            - Update position based on level change callback function 
        - Encoder channel 1                        
*/

#define _GNU_SOURCE

#include "../../include/encoder.h"
#include "../../include/motor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>
#include <sys/types.h>

#include <unistd.h>
#include <sched.h>

#include <signal.h>

// Globals 
const double ENC_SF = 1. / 6533;  // Pololu 64 CPR encoder (rev / count)
atomic_int pos_m0 = 0;  // need to do initialization w/ switch 
atomic_int pos_m1 = 0;
pthread_t t_id[2];  

volatile sig_atomic_t terminate_flag = 0;
void sighandler(int sig) { terminate_flag = 1; }

static int stick_this_thread_to_core(int core_id) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   if (core_id < 0 || core_id >= num_cores)
      return EINVAL;

   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);

   pthread_t current_thread = pthread_self();    
   return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

// Encoder callback
void callback_enc_m0(int way)
{
   pos_m0 += way;
}

void callback_enc_m1(int way)
{
    pos_m1 += way;
}

void* callback_m0(void* data)
{
    stick_this_thread_to_core(1);
    double pos;
    Motor_t* motor = (Motor_t*) data;
    setStartTime(motor->loop_timer);
    while (!terminate_flag) {
        pos = pos_m0 * ENC_SF;
        motor->curr_pos = pos;
        updateControl(motor);
    }
}

void* callback_m1(void* data)
{
    stick_this_thread_to_core(2);
    double pos;
    Motor_t* motor = (Motor_t*) data;
    setStartTime(motor->loop_timer);
    while (!terminate_flag) {
        pos = pos_m1 * ENC_SF;
        motor->curr_pos = pos;
        updateControl(motor);
    }
}

int main() {

	// Read previous motor position for continuity
	FILE* fp;
	char buff[255];
	double prev_pos_m0, prev_pos_m1;
	fp = fopen("/home/ubuntu/position.txt", "r");
	int i = fscanf(fp, "%lf %lf", &prev_pos_m0, &prev_pos_m1);	
	if (i != 2) {
		printf("Previous position can't be read\n");
		return -1;
	}
	fclose(fp);
	fp = fopen("/home/ubuntu/position.txt", "w");
	printf("Motor 0 previous position: %f\n", prev_pos_m0);
	printf("Motor 1 previous position: %f\n", prev_pos_m1);
	pos_m0 = (int)(prev_pos_m0 / ENC_SF);
	pos_m1 = (int)(prev_pos_m1 / ENC_SF);
	char motor_0_str[10];
	char motor_1_str[10];
	char motor_pos_str[21];

    // Initialize GPIO and signal handling
    // gpioCfgSetInternals(1<<10);
	if (gpioInitialise() < 0) return 1;
    signal(SIGINT, &sighandler); // intercept SIGINT
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
	signal(SIGSEGV, &sighandler);
	signal(SIGCONT, &sighandler);
    
	// Initialize encoders
	Pi_Renc_t* renc_0;
    Pi_Renc_t* renc_1;
    renc_0 = Pi_Renc(20, 21, callback_enc_m0);
    renc_1 = Pi_Renc(9, 10, callback_enc_m1);

    // Initialize motors 
    printf("Initializing Motors\n");
    Motor_t* motor_0 = Motor(0, 0, "127.0.0.1", 6379);
    Motor_t* motor_1 = Motor(1, 1, "127.0.0.1", 6379);
    setGains(motor_0, 100, 0, 0);
    setGains(motor_1, 0, 0, 0);

	// Update motor positions and desired positions with previous position
	motor_0->prev_pos = prev_pos_m0;
	motor_0->curr_pos = prev_pos_m0;
	motor_0->des_pos = prev_pos_m0;
	motor_1->prev_pos = prev_pos_m1;
	motor_1->curr_pos = prev_pos_m1;
	motor_1->des_pos = prev_pos_m1;
	writeValues(motor_0);
	writeValues(motor_1);
	writeDesiredValues(motor_0, prev_pos_m0);
	writeDesiredValues(motor_1, prev_pos_m1);

    // Start motor threads 
    printf("Starting Threads\n");
    pthread_create(&t_id[0], NULL, callback_m0, (void*) motor_0);
    pthread_create(&t_id[1], NULL, callback_m1, (void*) motor_1); 

    // // Execute termination lines in thread
    // pthread_join(t_id[0]);
    // pthread_join(t_id[1]);

    while (!terminate_flag) {}
    Pi_Renc_cancel(renc_0);
    Pi_Renc_cancel(renc_1);
    snprintf(motor_0_str, sizeof(char) * (10), "%f", motor_0->curr_pos);
	snprintf(motor_1_str, sizeof(char) * (10), "%f", motor_1->curr_pos);
	snprintf(motor_pos_str, sizeof(char) * (21), "%s%s%s", motor_0_str, " ", motor_1_str);
	printf("Final positions: %s\n", motor_pos_str);
	fprintf(fp, "%s", motor_pos_str);
	fclose(fp);
	stopMotor(motor_0);
	stopMotor(motor_1);
	gpioTerminate();
    printf("\n");
    printf("Terminating Process\n");
	return -1;       
 }

    
