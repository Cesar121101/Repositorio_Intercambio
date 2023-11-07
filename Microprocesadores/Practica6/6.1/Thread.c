#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t Timer;                        // thread id 
int flag = 0;

void Timer_Func (void *argument);                   // thread function

GPIO_InitTypeDef led_ld1 = {
			.Pin = GPIO_PIN_0,
			.Mode = GPIO_MODE_OUTPUT_PP,
			.Pull = GPIO_NOPULL,
			.Speed = GPIO_SPEED_FREQ_LOW
	};

GPIO_InitTypeDef led_ld2 = {
		.Pin = GPIO_PIN_14,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW
};

typedef struct  {
	GPIO_InitTypeDef pin;
	GPIO_TypeDef *port;
} mygpio_pin;

mygpio_pin pinB0;
mygpio_pin pinB14;

void Timer_Callback(void *arg);

//Callback del timer
void Timer_Callback(void *arg){
	if(flag == 0){
		HAL_GPIO_TogglePin(GPIOB,led_ld1.Pin);
		HAL_GPIO_TogglePin(GPIOB,led_ld2.Pin);
	}
	flag = 1;
}	

int Init_Thread (void) {	
	Timer = osThreadNew(Timer_Func, NULL, NULL); //Crear el Thread del timer
	if (Timer == NULL) {
    return(-1);
  }
  return(0);
}
 
void Timer_Func (void *argument) {
	// Initialize LEDS
	__HAL_RCC_GPIOB_CLK_ENABLE();
  
	HAL_GPIO_Init(GPIOB, &led_ld1);
	HAL_GPIO_Init(GPIOB, &led_ld2);
	
	HAL_GPIO_TogglePin(GPIOB,led_ld1.Pin);
	
	osTimerId_t timsoft1 = osTimerNew(Timer_Callback, osTimerOnce, NULL, NULL); //Crear el timer
	
	osTimerStart(timsoft1,3000); //Iniciar el timer
	uint32_t status; //Flags de los Threads
  while (1) {
		status=osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				osTimerStart(timsoft1,3000); //Reiniciar el timer
			break;
		}
  }
}