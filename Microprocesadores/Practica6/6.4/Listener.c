#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>   

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t Listener;   // thread id                
void Listener_Func (void *argument);                   // thread function

uint32_t flag = 0;

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

void Timer2_Callback(){
	flag = 1;
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
	osTimerId_t timsoft2;
	timsoft2 = osTimerNew(Timer2_Callback, osTimerOnce, NULL, NULL); //Crear el timer	
	uint32_t flag2 = 0;
	uint32_t status;
  while (1) {
    status=osThreadFlagsWait(0x3,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				flag = 0;
				osTimerStart(timsoft2,2000);
				while(flag == 0){
					HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);
					osDelay(125);
				}
				break;
			case 2:
				flag2 = 0;
				while(1){
					if(flag2 == 0){
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
						osDelay(250);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
						osDelay(250);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
						flag2 = 1;
					}else{
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
						flag2 = 0;
					}
					osDelay(250);
				}
				break;
		}
  }
}
