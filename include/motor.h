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

struct Motor;
typedef struct Motor Motor_t;

Motor_t* Motor(int channel, char* ip, int port, int motor_id);
void setGains(Motor_t* motor, double kp, double kv, double ki);
void setTarget(Motor_t* motor, double pos, double vel);
void updateControl(Motor_t* motor);


#endif