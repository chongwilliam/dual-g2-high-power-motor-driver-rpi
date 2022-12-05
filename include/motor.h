/**
 * @file motor.h
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-11-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _PI_MOTOR_H
#define _PI_MOTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pigpio.h>
#include <hiredis/hiredis.h>

#include "../include/filter.h"
#include "../include/loop_timer.h"

struct Motor_s {
    LoopTimer_t* loop_timer;
    redisContext* redis_context;
    redisReply* redis_reply;
    BWLowPass* low_pass_filter;
    double kp;
    double kv;
    double ki;
    double prev_pos;
    double curr_pos;
    double curr_vel;
    double des_pos;
    double des_vel;
    double int_err;
    double t_prev;
    double t_curr;
    int pin_flt;
    int pin_pwm;
    int pin_en;
    int pin_dir;
    char* ip;
    int port;
    int motor_id;
    char* get_des_pos_str;
    char* get_des_vel_str;
    char* set_pos_str;
    char* set_vel_str;
};
typedef struct Motor_s Motor_t;

Motor_t* Motor(int channel, int motor_id, char* ip, int port);
void setGains(Motor_t* motor, double kp, double kv, double ki);
void setTarget(Motor_t* motor, double pos, double vel);
void readValues(Motor_t* motor);
void writeValues(Motor_t* motor);
void updateVel(Motor_t* motor, double dt);
void updateControl(Motor_t* motor);
void stopMotor(Motor_t* motor);

#endif