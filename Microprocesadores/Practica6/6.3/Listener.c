#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>   

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t Listener;                        // thread id
extern osMessageQueueId_t id_MsgQueue; 
void Listener_Func (void *argument);                   // thread function

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

int Init_Listener (void) {
  Listener = osThreadNew(Listener_Func, NULL, NULL);
  if (Listener == NULL) {
    return(-1);
  }
	init_GPIO();
  return(0);
}
 
void Listener_Func (void *argument) {
	osStatus_t status;
	uint8_t val=0;
  while (1) {
    status = osMessageQueueGet(id_MsgQueue, &val, NULL, 10U);   // wait for message
		switch (val){
			case 1:
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
				break;
			case 2:
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
				break;
			case 3:
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
				break;
			case 4:
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
				break;
			case 5:
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);				
				break;
		}
  }
}
