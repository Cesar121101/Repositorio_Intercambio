#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "rgb.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t rgb;                        // thread id
extern osMessageQueueId_t queue_temperatura; 

int init_RGB(void);
void ThreadRGB (void *argument);                   // thread function
float x = 0.0f;
void initLeds(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  __HAL_RCC_GPIOD_CLK_ENABLE();

  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_RESET);
	
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
		//led de prueba
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}  
 
int init_RGB (void) {
  rgb = osThreadNew(ThreadRGB, NULL, NULL);
  if (rgb == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadRGB (void *argument) {
	initLeds();
	osStatus_t status;
	float msg;
	float Tr = 25.0f;
  while (1) {
		status = osMessageQueueGet(queue_temperatura, &msg, NULL, 10U);
		x = Tr - msg;
		
		if (x > 5.0) {
			// Rojo
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
		} 
		else if (x >= 0.0 && x <= 5.0) {
			// Verde + Rojo
			//HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
		} 
		else if (x >= -5.0 && x < 0.0) {
			// Verde + Azul
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
		} 
		else if (x < -5.0) {
			// Azul
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
		} 
		else {
			// Apagar todos los LEDs
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
		}
	}
}