#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"

//Threads IDs
osThreadId_t principal;                         
extern osThreadId_t adc;

//Queues IDs
osMessageQueueId_t queue_lcd;
extern osMessageQueueId_t queue_adc;

//Variables
extern int horas,minutos,segundos;
int i;
char str[50];
float pot1, pot2;

void adcFunc(){
	uint8_t potenciometro;
	osThreadFlagsSet(adc,0x1);
	osMessageQueueGet(queue_adc, &potenciometro, NULL, 10U);
	if(potenciometro == 1){
		osMessageQueueGet(queue_adc, &pot1, NULL, 10U);
	}else if(potenciometro == 2){
		osMessageQueueGet(queue_adc, &pot2, NULL, 10U);
	}
}

void horaLCD(){
	int linea1 = 1;
	int linea2 = 2;
	int limpiar = 3;
	osMessageQueuePut(queue_lcd, &limpiar, 0U, 0U);
	sprintf(str,"H: %02d M: %02d S:%02d", horas,minutos,segundos);
	for(i = 0; i<sizeof(str); i++){
		if(str[i] != NULL){
			osMessageQueuePut(queue_lcd, &linea1, 0U, 0U);
			osMessageQueuePut(queue_lcd, &str[i], 0U, 0U);
		}
	}
}

void potLCD(){
	int linea1 = 1;
	int linea2 = 2;
	int limpiar = 3;
	osMessageQueuePut(queue_lcd, &limpiar, 0U, 0U);
	sprintf(str,"Tm: %04.1f Tr: %04.1f",pot1,pot2);
	for(i = 0; i<sizeof(str); i++){
		if(str[i] != NULL){
			osMessageQueuePut(queue_lcd, &linea1, 0U, 0U);
			osMessageQueuePut(queue_lcd, &str[i], 0U, 0U);
		}
	}
}

void principal_Func (void *argument);                   // thread function
int init_Principal(void) {
	principal = osThreadNew(principal_Func, NULL, NULL); //Crear el Thread del timer
	queue_lcd = osMessageQueueNew(64, sizeof(uint8_t), NULL);
	if (principal == NULL) {
    return(-1);
  }
  return(0);
}

void principal_Func (void *argument) {
  
	while (1) {
		adcFunc();
		potLCD();
		osDelay(500);
  }
}