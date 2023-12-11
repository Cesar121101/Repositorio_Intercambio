#ifndef __PWM_H
#define __PWM_H
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef struct {
	float Tm;
	float Tr;
}msgQueue_PWM;

int init_PWM(void);
#endif
