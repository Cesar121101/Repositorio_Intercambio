#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
 
/*----------------------------------------------------------------------------
 *      Thread 3 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t THled3;                        // thread id
extern osThreadId_t THled2;

void Thread3(void *argument);                   // thread function
 
GPIO_InitTypeDef led_ld3 = {
			.Pin = GPIO_PIN_14,
			.Mode = GPIO_MODE_OUTPUT_PP,
			.Pull = GPIO_NOPULL,
			.Speed = GPIO_SPEED_FREQ_LOW
	};

typedef struct  {
	GPIO_InitTypeDef pin;
	GPIO_TypeDef *port;
	int delayE;
	int delayA;
} mygpio_pin;

mygpio_pin pinB14;
 
int Init_Thread3(void) {
 
  // Initialize LEDS
	__HAL_RCC_GPIOB_CLK_ENABLE();
	pinB14.pin= led_ld3;
	pinB14.port=GPIOB;
	pinB14.delayE=287;
	pinB14.delayA=287;
  THled3 = osThreadNew(Thread3, (void *)&pinB14, NULL);
  if (THled3 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread3(void *argument) {
 
  mygpio_pin *gpio = (mygpio_pin *)argument;
	HAL_GPIO_Init(gpio->port, &(gpio->pin));
	uint32_t status2; //Flags del Thread 2
	uint32_t status3; //Flags del Thread 3
	int contador = 0;
	int errors=0;	
  while (1) {
		status2=osThreadFlagsWait(0xA,osFlagsWaitAny,osWaitForever);
		switch (status2){
			case 2:
				if(contador < 30){
					HAL_GPIO_TogglePin(gpio->port, gpio->pin.Pin);
					osDelay(gpio->delayE);
					HAL_GPIO_TogglePin(gpio->port, gpio->pin.Pin);
					osDelay(gpio->delayA);
					contador ++;
					if(contador == 25){
						status3 = osThreadFlagsSet(THled2,0x0004);
					}
				}
				break; 
			case 8:
				contador=0;
				break;
			default:errors++;
				break;			
		}
  }
}
