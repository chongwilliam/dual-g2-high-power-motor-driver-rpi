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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <hiredis/hiredis.h>

// Globals
redisContext* redis_context;
redisReply* redis_reply;
clock_t next_t;
char output[10];

struct Test {
    redisContext* c;
    redisReply* reply;
    char* ip;
    int port;
    clock_t next_time;
    double t_prev;
    double t_curr;
    double des_pos;
    double des_vel;
    double curr_pos;
    double curr_vel;
    char* buffer;
    char* pos_str;
    char* vel_str;
};

typedef struct Test Test_t;

Test_t* Test(int channel, char* ip, int port, int motor_id)
{
    Test_t* test;
    test = malloc(sizeof(Test_t));

    test->ip = ip;
    test->port = port;

    redis_context = malloc(sizeof(redisContext));
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    redis_context = redisConnectWithTimeout(test->ip, test->port, timeout);
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

    test->c = redis_context;
    test->reply = redis_reply;

    test->ip = ip;    

    test->next_time = next_t;
    test->next_time = clock();
    test->t_prev = (double) test->next_time / CLOCKS_PER_SEC;
    test->t_curr = (double) test->next_time / CLOCKS_PER_SEC;

    test->buffer = output;

    char set_pos_str[] = "SET POS";
    char set_vel_str[] = "SET VEL";
    char format_str[] = "%s";
    char motor_id_str[2];
    snprintf(motor_id_str, 2, "%u", motor_id);    

    snprintf(set_pos_str, sizeof(motor_id_str), "%s", motor_id_str);
    snprintf(set_pos_str, sizeof("%s"), "%s", format_str);
    test->pos_str = set_pos_str;

    snprintf(set_vel_str, sizeof(motor_id_str), "%s", motor_id_str);
    snprintf(set_vel_str, sizeof("%s"), "%s", format_str);
    test->vel_str = set_vel_str;
    // test->pos_str = strcat("SET POS", motor_id_str);
    // test->vel_str = strcat("SET VEL", motor_id_str);

    return test;
}

void updateTime(Test_t* test) 
{
    test->next_time = clock();
    test->t_curr = (double) test->next_time / CLOCKS_PER_SEC;
}

static void readValues(Test_t* test)
{
    test->reply = redisCommand(test->c, "GET test0");
    test->des_pos = strtod(test->reply->str, NULL);
    freeReplyObject(test->reply);
    test->reply = redisCommand(test->c, "GET test1");
    test->des_vel = strtod(test->reply->str, NULL);
    freeReplyObject(test->reply);
}

static void writeValues(Test_t* test, double val_a, double val_b)
{
    snprintf(test->buffer, 10, "%f", val_a);
    test->reply = redisCommand(test->c, "SET test0 %s", test->buffer);  // set position 
    freeReplyObject(test->reply);
    snprintf(test->buffer, 10, "%f", val_b);
    test->reply = redisCommand(test->c, "SET test1 %s", test->buffer);  // set velocity 
    freeReplyObject(test->reply); 
}

int main()
{
    Test_t* test = Test(0, "127.0.0.1", 6379, 0);
    writeValues(test, 0, 0);

    int n_samples = 10;  
    int i = 0;
    while (i < n_samples) {
        updateTime(test);        
        printf("%f us\n", (test->t_curr - test->t_prev) * 1e6);
        test->t_prev = test->t_curr;

        // read
        readValues(test);
        // printf("%f \n", test->des_pos);
        // printf("%f \n", test->des_vel);

        // write
        writeValues(test, i-1, i+1);

        i++;
    }
}