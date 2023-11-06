#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
 
/*----------------------------------------------------------------------------
 *      Thread 2 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t THled2;                        // thread id
extern osThreadId_t THled3;

void Thread2(void *argument);                   // thread function
 
GPIO_InitTypeDef led_ld2 = {
			.Pin = GPIO_PIN_7,
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

mygpio_pin pinB7;
 
int Init_Thread2(void) {
 
  // Initialize LEDS
	__HAL_RCC_GPIOB_CLK_ENABLE();
	pinB7.pin= led_ld2;
	pinB7.port=GPIOB;
	pinB7.delayE=137;
	pinB7.delayA=137;
  THled2 = osThreadNew(Thread2, (void *)&pinB7, NULL);
  if (THled2 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread2(void *argument) {
 
  mygpio_pin *gpio = (mygpio_pin *)argument;
	HAL_GPIO_Init(gpio->port, &(gpio->pin));	
  uint32_t status; //Flags del Thread 1
	uint32_t status2; //Flags del Thread 2
	int contador = 0, contador2 = 0;
	int errors=0;
	
	while (1) {
		status=osThreadFlagsWait(0x5,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				if(contador < 20){
					HAL_GPIO_TogglePin(gpio->port, gpio->pin.Pin);
					osDelay(gpio->delayE);
					HAL_GPIO_TogglePin(gpio->port, gpio->pin.Pin);
					osDelay(gpio->delayA);
					contador++;
				}
				if(contador >= 15 || contador2 >= 25){
					contador2 ++;
					if(contador2 == 40){
						contador2 = 0;
						status2= osThreadFlagsSet(THled3,0x0008);
					}else{
						status2= osThreadFlagsSet(THled3,0x0002);
					}
				}
				break; 
			case 4:
				contador = 0;
				break; 
			default:errors++;
				break;			
		}
  }
}