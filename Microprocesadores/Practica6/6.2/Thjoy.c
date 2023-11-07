#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t Timer;                        // thread id 
void Timer_Func (void *argument);                   // thread function
void Timer_Callback(void *arg);

//Callback del timer
void Timer_Callback(void *arg){
	int *contador = (int *)arg;
	//Lectura de pines
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		contador+=1;
	}
	else if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_11) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		contador+=2;
	}
	else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_12) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
		contador+=3;
	}
	else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
		contador+=4;
	}
	else if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
		contador+=5;
	}
}

void init_GPIO(){
	GPIO_InitTypeDef GPIO_InitStruct; //Estructura GPIO
	__HAL_RCC_GPIOB_CLK_ENABLE();	 //Habilitar el reloj de los puertos GPIO B
	//LED 1,2 y 3
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //Modo Output
	GPIO_InitStruct.Pull = GPIO_NOPULL; //Deshabilitar resitencias del pin
	
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_7 | GPIO_PIN_14; //Pines
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //Inicializar los pines
}

int Init_JoyThread (void) {
	Timer = osThreadNew(Timer_Func, NULL, NULL); //Crear el Thread del timer
	if (Timer == NULL) {
    return(-1);
  }
	init_GPIO	();
  return(0);
}
 
void Timer_Func (void *argument) {
	int contador = 0; 
	osTimerId_t timsoft1 = osTimerNew(Timer_Callback, osTimerOnce, &contador, NULL); //Crear el timer	
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