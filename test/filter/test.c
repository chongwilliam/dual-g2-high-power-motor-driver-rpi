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

#include "../../include/filter.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void low_pass_example()
{
    BWLowPass* filter = create_bw_low_pass_filter(6, 1000, 10);
    
    for (int i = 0; i < 100; i++){
        printf("Output[%d]:%f\n", i, bw_low_pass(filter, i* 100));
    }

    free_bw_low_pass(filter);

}

int main() 
{   
    printf("========= Butterworth low pass filter example =========\n\n");
    low_pass_example();
    printf("========= Done. =========\n\n");
    return 0;
}