/**
 * @file encoder.h
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-11-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _PI_ENCODER_H
#define _PI_ENCODER_H

#include <stdio.h>
#include <stdlib.h>

#include <pigpio.h>

typedef void (*Pi_Renc_CB_t)(int);

struct _Pi_Renc_s;

typedef struct _Pi_Renc_s Pi_Renc_t;

Pi_Renc_t * Pi_Renc(int gpioA, int gpioB, Pi_Renc_CB_t callback);
/*
   This function establishes a rotary encoder on gpioA and gpioB.

   When the encoder is turned the callback function is called.

   A pointer to a private data type is returned.  This should be passed
   to Pi_Renc_cancel if the rotary encoder is to be cancelled.
*/

void Pi_Renc_cancel(Pi_Renc_t *renc);
/*
   This function releases the resources used by the decoder.
*/

#endif