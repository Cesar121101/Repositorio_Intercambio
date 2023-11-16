#ifndef __LCD_H
#define __LCD_H

#include "stm32f4xx_hal.h"

void LCD_Init(void);
void LCD_clear(void);
void LCD_update(void);
void symbolToLocalBuffer_L1(uint8_t symbol);
void symbolToLocalBuffer_L2(uint8_t symbol);
void symbolToLocalBuffer(uint8_t line,uint8_t symbol);
void desplazarAbajo(void);
void desplazarArriba(void);
void desplazarDerecha(void);
void desplazarIzquierda(void);
#endif
