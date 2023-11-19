#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t principal;                        // thread id 
osMessageQueueId_t queue_lcd;

char str[50];
int i;

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
	int linea1 = 1;
	int linea2 = 2;
	sprintf(str,"hola como estas");
	
	for(i = 0; i<sizeof(str); i++){
		if(str[i] != NULL){
			osMessageQueuePut(queue_lcd, &linea1, 0U, 0U);
			osMessageQueuePut(queue_lcd, &str[i], 0U, 0U);
		}
	}
	

  while (1) {
  }
}