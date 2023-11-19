#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "rgb.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t rgb;                        // thread id
//extern osMessageQueueId_t queue_rgb; 
extern osMessageQueueId_t queue_joystick; 

int init_RGB(void);

void ThreadRGB (void *argument);                   // thread function
 
void initLeds(void){
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  __HAL_RCC_GPIOD_CLK_ENABLE();

  
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_RESET);
	
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}  
 
int init_RGB (void) {
 
	initLeds();
  rgb = osThreadNew(ThreadRGB, NULL, NULL);
  if (rgb == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThreadRGB (void *argument) {
	osStatus_t status;
	uint8_t msg;
	
  while (1) {
		status = osMessageQueueGet(queue_joystick, &msg, NULL, 10U);
		switch(msg){
			case 1:
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
				break;
			case 2:
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
				break;
			case 3:
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
				break;
			case 4:
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
				break;
			case 5:
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
				break;
		}		
	}
}

//Código para testear el módulo
//void Th_Leds_Test(void *argument); 
// 
//osThreadId_t tid_Th_Leds_Test;

//int init_Th_Leds_Test(void) {
// 
//  tid_Th_Leds_Test = osThreadNew(Th_Leds_Test, NULL, NULL);
//  if (tid_Th_Leds_Test == NULL) {
//    return(-1);
//  }	
//  return(0);
//}
// 
//void Th_Leds_Test (void *argument) {

//	MSGQUEUE_LEDS_t localObject;
//	uint8_t contador = 0;
//	
//	
//	
//  while (1) {
//		
//		osDelay(500);
//		
//		contador++;
//		
//		localObject.LD1 = contador & 0x01; //LSB
//		localObject.LD2 = contador & 0x02; 
//		localObject.LD3 = contador & 0x04;
//		
//		
//		osMessageQueuePut(mid_MsgQueueLeds, &localObject, 0U, 0U);
//    		
//  }
//}