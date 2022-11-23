/**
 * @file motor.c
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-11-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../include/filter.h"
#include "../include/motor.h"

// Constants
// const double IN_TO_M = 0.0254;  
const double ENC_SF = 0.0254 * 7239;  // pulses / m
const int MAX_SPEED = 480;
const int _pin_M1FLT = 5;
const int _pin_M2FLT = 6;
const int _pin_M1PWM = 12;
const int _pin_M2PWM = 13;
const int _pin_M1EN = 22;
const int _pin_M2EN = 23;
const int _pin_M1DIR = 24;
const int _pin_M2DIR = 25;

// Globals 
clock_t next_t;
redisContext* redis_context;
redisReply* redis_reply;
BWLowPass* low_pass_filter;

struct Motor {
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
    redisContext* c;
    redisReply* reply;
    char* ip;
    int port;
    clock_t next_time;
};

Motor_t* Motor(int channel, char* ip)
{
    Motor_t* motor;
    motor = malloc(sizeof(Motor_t));

    motor->kp = 0;
    motor->kv = 0;
    motor->ki = 0;
    motor->curr_pos = 0;
    motor->curr_vel = 0;
    motor->des_pos = 0;
    motor->des_vel = 0;
    motor->c = redis_context;

    if (channel == 0) {
        motor->pin_flt = _pin_M1FLT;
        motor->pin_pwm = _pin_M1PWM;
        motor->pin_en = _pin_M1EN;
        motor->pin_dir = _pin_M1DIR;
    } else if (channel == 1) {
        motor->pin_flt = _pin_M2FLT;
        motor->pin_pwm = _pin_M2PWM;
        motor->pin_en = _pin_M2EN;
        motor->pin_dir = _pin_M2DIR;
    }
    gpioSetPullUpDown(motor->pin_flt, PI_PUD_UP);  // pull up fault pin 
    gpioWrite(motor->pin_en, 1);  // enable driver

    redis_context = malloc(sizeof(redisContext));
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    redis_context = redisConnectWithTimeout(motor->ip, motor->port, timeout);
    if (redis_context == NULL || redis_context->err) {
        if (redis_context) {
            printf("Connection error: %s\n", redis_context->errstr);
            redisFree(redis_context);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    redis_reply = malloc(sizeof(redisReply));
    freeReplyObject(redis_reply);

    motor->c = redis_context;
    motor->reply = redis_reply;

    motor->ip = ip;    

    next_t = clock();
    motor->t_prev = (double) next_t / CLOCKS_PER_SEC;
    motor->t_curr = (double) next_t / CLOCKS_PER_SEC;

    int order = 0;  // dimensionality of input - 1
    int sampling_frequency = 5000;  // drivers running at 5 kHz
    int half_power_frequency = 10;  // (aka cutoff frequency)
    low_pass_filter = create_bw_low_pass_filter(order, sampling_frequency, half_power_frequency);

    return motor;
}

void setGains(Motor_t* motor, double kp, double kv, double ki) 
{
    motor->kp = kp;
    motor->kv = kv;
    motor->ki = ki;
}

void setTarget(Motor_t* motor, double pos, double vel)
{
    motor->des_pos = pos;
    motor->des_vel = vel;
}

void readValues(Motor_t* motor)
{
    char* ptr;
    motor->reply = redisCommand(motor->c, "GET ");
    motor->des_pos = strtod(motor->reply->str, &ptr);
    freeReplyObject(motor->reply);
    motor->reply = redisCommand(motor->c, "GET ");
    motor->des_vel = strtod(motor->reply->str, &ptr);
    freeReplyObject(motor->reply);
}

void writeValues(Motor_t* motor)
{
    motor->reply = redisCommand(motor->c, "SET ");  // set position 
    freeReplyObject(motor->reply);
    motor->reply = redisCommand(motor->c, "SET ");  // set velocity 
    freeReplyObject(motor->reply); 
}

void updateVel(Motor_t* motor, double dt)
{
    double vel = (motor->curr_pos - motor->prev_pos) / dt;
    motor->curr_vel = bw_low_pass(low_pass_filter, vel);    
}

void updateControl(Motor_t* motor) 
{
    // Read targets
    readValues(motor);

    // Get time 
    next_t = clock();
    motor->t_curr = (double) next_t;
    double dt = motor->t_curr - motor->t_prev;

    // Update velocity 
    updateVel(motor, dt);

    // Compute PID for PWM duty cycle 
    motor->int_err += dt * (motor->curr_pos - motor->des_pos);
    double speed = motor->kp * (motor->curr_pos - motor->des_pos) 
                    + motor->kv * (motor->curr_vel - motor->des_vel) 
                    + motor->ki * motor->int_err;

    // set gpio 
    double dir_value = 0;
    if (speed < 0) {
        speed = -speed;
        dir_value = 1;
    } 

    if (speed > MAX_SPEED) {
        speed = MAX_SPEED;
    }

    gpioWrite(motor->pin_dir, dir_value);
    gpioHardwarePWM(motor->pin_pwm, 5000, (int) (speed * 6250 / 3));
    
    // Print information    
    writeValues(motor);

    // push forward
    motor->t_prev = motor->t_curr;
}