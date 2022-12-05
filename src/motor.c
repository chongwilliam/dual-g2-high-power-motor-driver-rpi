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

#include "../include/motor.h"

// Constants 
// const static double IN_TO_M = 0.0254;  
const static int MAX_SPEED = 480;
const static int _pin_M1FLT = 5;
const static int _pin_M2FLT = 6;
const static int _pin_M1PWM = 12;
const static int _pin_M2PWM = 13;
const static int _pin_M1EN = 22;
const static int _pin_M2EN = 23;
const static int _pin_M1DIR = 24;
const static int _pin_M2DIR = 25;

Motor_t* Motor(int channel, int motor_id, char* ip, int port)
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
    motor->motor_id = motor_id;

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

    motor->ip = ip;    
    motor->port = port;
    // redisContext* redis_context;
    motor->redis_context = malloc(sizeof(redisContext));
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    motor->redis_context = redisConnectWithTimeout(motor->ip, motor->port, timeout);
    if (motor->redis_context == NULL || motor->redis_context->err) {
        if (motor->redis_context) {
            printf("Connection error: %s\n", motor->redis_context->errstr);
            redisFree(motor->redis_context);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    // redisReply* redis_reply;
    motor->redis_reply = malloc(sizeof(redisReply));
    freeReplyObject(motor->redis_reply);

    // strings 
    char motor_id_str[2];
    snprintf(motor_id_str, 2, "%u", motor_id);
    char set_pos_str[] = "SET POS";
    char set_vel_str[] = "SET VEL";
    char get_des_pos_str[] = "GET POS";
    char get_des_vel_str[] = "GET VEL";
    char format_str[] = "%s";
    
    snprintf(set_pos_str, sizeof(motor_id_str), "%s", motor_id_str);
    snprintf(set_pos_str, sizeof("%s"), "%s", format_str);
    motor->set_pos_str = set_pos_str;

    snprintf(set_vel_str, sizeof(motor_id_str), "%s", motor_id_str);
    snprintf(set_vel_str, sizeof("%s"), "%s", format_str);
    motor->set_vel_str = set_vel_str;

    snprintf(get_des_pos_str, sizeof(motor_id_str), "%s", motor_id_str);
    motor->get_des_pos_str = get_des_pos_str;

    snprintf(get_des_vel_str, sizeof(motor_id_str), "%s", motor_id_str);
    motor->get_des_vel_str = get_des_vel_str;

    motor->loop_timer = malloc(sizeof(LoopTimer_t));
    motor->loop_timer = LoopTimer(2000);  // 2 kHz control by default 

    int order = 6;  
    int sampling_frequency = 2000;  // drivers running at 2 kHz
    int half_power_frequency = 1;  // cutoff frequency (Hz)
    motor->low_pass_filter = malloc(sizeof(BWLowPass));
    motor->low_pass_filter = create_bw_low_pass_filter(order, sampling_frequency, half_power_frequency);

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
    motor->redis_reply = redisCommand(motor->redis_context, motor->get_des_pos_str);
    motor->des_pos = strtod(motor->redis_reply->str, NULL);
    freeReplyObject(motor->redis_reply);
    motor->redis_reply = redisCommand(motor->redis_context, motor->get_des_vel_str);
    motor->des_vel = strtod(motor->redis_reply->str, NULL);
    freeReplyObject(motor->redis_reply);
}

void writeValues(Motor_t* motor)
{
    motor->redis_reply = redisCommand(motor->redis_context, motor->set_pos_str, motor->curr_pos);  // set position 
    freeReplyObject(motor->redis_reply);
    motor->redis_reply = redisCommand(motor->redis_context, motor->set_vel_str, motor->curr_vel);  // set velocity 
    freeReplyObject(motor->redis_reply); 
}

void updateVel(Motor_t* motor, double dt)
{
    double vel = (motor->curr_pos - motor->prev_pos) / dt;
    motor->curr_vel = bw_low_pass(motor->low_pass_filter, vel);    
}

void updateControl(Motor_t* motor) 
{
    // Read targets
    readValues(motor);

    // Get elapsed time and set start timer
    double dt = getElapsedTime(motor->loop_timer);
    setStartTime(motor->loop_timer);

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

    // push forward
    motor->prev_pos = motor->curr_pos;
    
    // Print information    
    writeValues(motor);

    // Wait until next loop
    waitUntilNextLoop(motor->loop_timer);  // this continuously updates end timer 
}

void stopMotor(Motor_t* motor)
{
    setGains(motor, 0, 0, 0);
    gpioHardwarePWM(motor->pin_pwm, 5000, 0);  // no speed sent to pwm pin 
    gpioWrite(motor->pin_en, 0);  // disable driver
}