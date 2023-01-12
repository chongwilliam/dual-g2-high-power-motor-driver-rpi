/**
 * @file encoder.c
 * @author William Chong (wmchong@stanford.edu)
 * @brief 
 * @version 0.1
 * @date 2022-11-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* 
    Copied from pigpio examples 
*/

#include "../include/encoder.h"

struct _Pi_Renc_s
{
   int gpioA;
   int gpioB;
   Pi_Renc_CB_t callback;
   int levA;
   int levB;
   int lastGpio;
};

/*

             +---------+         +---------+      0
             |         |         |         |
   A         |         |         |         |
             |         |         |         |
   +---------+         +---------+         +----- 1

       +---------+         +---------+            0
       |         |         |         |
   B   i|         |         |         |
       |         |         |         |
   ----+         +---------+         +---------+  1

*/
static void _cb0(int gpio, int level, uint32_t tick, void *user)
{
    Pi_Renc_t *renc;
    renc = user;
    if (gpio == renc->gpioA) renc->levA = level; else renc->levB = level;
    if (gpio != renc->lastGpio)
    {
		renc->lastGpio = gpio;
		if ((gpio == renc->gpioA) && (level == 1)) 
		{
			if (renc->levB) (renc->callback)(1);
		}
		else if ((gpio == renc->gpioB) && (level == 1)) 
		{
			if (renc->levA) (renc->callback)(-1);
		}
	}
}

/*
    Signal 2 leads when extending
    Signal 1 leads when retracting
*/
static void _cb(int gpio, int level, uint32_t tick, void *user)
{
    Pi_Renc_t *renc;

    renc = user;

    if (gpio == renc->gpioA) renc->levA = level; else renc->levB = level;

    if (gpio != renc->lastGpio) { /* debounce */
        renc->lastGpio = gpio;

		if (gpio == renc->gpioA) {
		    if (level == 1) {
				if (renc->levB) {
			    	(renc->callback)(1);
				} else {
					(renc->callback)(-1);
				}
			} else {
				if (renc->levB == 0) {
					(renc->callback)(1);
				} else {
					(renc->callback)(-1);
				}
			}
		} else {
			if (level == 1) {
				if (renc->levA) {
					(renc->callback)(-1);
				} else {
					(renc->callback)(1);
				}
			} else {
				if (renc->levA == 0) {
					(renc->callback)(-1);
				} else {
					(renc->callback)(1);
				}
			}
		}
	}
		
	    

        // if igpioA turns from 0 to 1
        //if ((gpio == renc->gpioA) && (level == 1))
        //{
            // if gpioB is 1
           // if (renc->levB) (renc->callback)(1);
        //}
        // if gpioB turns from 0 to 1
        //else if ((gpio == renc->gpioB) && (level == 1))
        //{
            // if gpioA is 1 
           // if (renc->levA) (renc->callback)(-1);
        //}
    //}
}

Pi_Renc_t * Pi_Renc(int gpioA, int gpioB, Pi_Renc_CB_t callback)
{
   Pi_Renc_t *renc;

   renc = malloc(sizeof(Pi_Renc_t));

   renc->gpioA = gpioA;
   renc->gpioB = gpioB;
   renc->callback = callback;
   renc->levA=0;
   renc->levB=0;
   renc->lastGpio = -1;

   gpioSetMode(gpioA, PI_INPUT);
   gpioSetMode(gpioB, PI_INPUT);

   /* pull up is needed as encoder common is grounded */

   gpioSetPullUpDown(gpioA, PI_PUD_UP);
   gpioSetPullUpDown(gpioB, PI_PUD_UP);

   /* monitor encoder level changes */

   gpioSetAlertFuncEx(gpioA, _cb, renc);
   gpioSetAlertFuncEx(gpioB, _cb, renc);

   return renc;
}

void Pi_Renc_cancel(Pi_Renc_t *renc)
{
   if (renc)
   {
      gpioSetAlertFunc(renc->gpioA, 0);
      gpioSetAlertFunc(renc->gpioB, 0);

      free(renc);
   }
}
