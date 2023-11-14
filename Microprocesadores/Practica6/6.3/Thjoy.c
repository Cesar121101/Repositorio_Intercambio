#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t Timer;                        // thread id 
osMessageQueueId_t id_MsgQueue;

void Timer_Func (void *argument);                   // thread function
void Timer_Callback(void *arg);

//Callback del timer
void Timer_Callback(void *arg){
	osStatus_t status;
	//Lectura de pines
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == GPIO_PIN_SET){
		uint8_t contadorArriba = 0b00001;
		status=osMessageQueuePut(id_MsgQueue, &contadorArriba, 0U, 0U);
	}
	else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11) == GPIO_PIN_SET){
		uint8_t contadorDerecha = 0b00010;
		status=osMessageQueuePut(id_MsgQueue, &contadorDerecha, 0U, 0U);
	}
	else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_12) == GPIO_PIN_SET){
		uint8_t contadorAbajo = 0b00011;
		status=osMessageQueuePut(id_MsgQueue, &contadorAbajo, 0U, 0U);
	}
	else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == GPIO_PIN_SET){
		uint8_t contadorIzquierda = 0b00100;
		status=osMessageQueuePut(id_MsgQueue, &contadorIzquierda, 0U, 0U);
	}
	else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == GPIO_PIN_SET){
		uint8_t contadorCentro = 0b00101;
		status=osMessageQueuePut(id_MsgQueue, &contadorCentro, 0U, 0U);
	}
}

int Init_JoyThread (void) {
	id_MsgQueue = osMessageQueueNew(16, sizeof(uint8_t), NULL);
	Timer = osThreadNew(Timer_Func, NULL, NULL); //Crear el Thread del timer
	if (Timer == NULL) {
    return(-1);
  }
  return(0);
}
 
void Timer_Func (void *argument) {
	osTimerId_t timsoft1 = osTimerNew(Timer_Callback, osTimerOnce, NULL, NULL); //Crear el timer	
	uint32_t status; //Flags de los Threads
  while (1) {
		status=osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				osTimerStart(timsoft1,50); //Reiniciar el timer
			break;
		}
  }
}