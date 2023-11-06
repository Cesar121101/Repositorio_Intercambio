#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t THled1;                        // thread id
extern osThreadId_t THled2;

void Thread1(void *argument);                   // thread function
 
GPIO_InitTypeDef led_ld1 = {
			.Pin = GPIO_PIN_0,
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

mygpio_pin pinB0;
 
int Init_Thread1(void) {
  // Initialize LEDS
	__HAL_RCC_GPIOB_CLK_ENABLE();
	pinB0.pin= led_ld1;
	pinB0.port=GPIOB;
	pinB0.delayE=200;
	pinB0.delayA=800;
  THled1 = osThreadNew(Thread1, (void *)&pinB0, NULL);
  if (THled1 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread1(void *argument) {
	uint32_t status; //Flags de los Threads
	int contador = 0;
  mygpio_pin *gpio = (mygpio_pin *)argument;
	HAL_GPIO_Init(gpio->port, &(gpio->pin));	
  while (1) {
		HAL_GPIO_TogglePin(gpio->port, gpio->pin.Pin);
		osDelay(gpio->delayE);
		HAL_GPIO_TogglePin(gpio->port, gpio->pin.Pin);
		osDelay(gpio->delayA);
		contador++;
		if (contador >= 5){
			contador = 5;
			status= osThreadFlagsSet(THled2,0x0001);
		}
  }
}
