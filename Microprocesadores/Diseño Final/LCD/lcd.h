#ifndef __LCD_H
#define __LCD_H
#include "stm32f4xx_hal.h"

typedef struct {                               
  uint8_t linea;
	char cadena[25];
	uint8_t subrayar;
	uint8_t numero;
} msgQueue_LCD;

int init_LCD(void);
int init_LCD_test(void);
#endif