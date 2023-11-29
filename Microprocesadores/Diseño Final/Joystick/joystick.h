#ifndef __JOYSTICK_H
#define __JOYSTICK_H
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h" 

typedef struct{
	uint8_t pulsacion;
	uint8_t joystick_value;
} MSGQUEUE_JOYSTICK;

	int init_Joystick (void);
#endif
