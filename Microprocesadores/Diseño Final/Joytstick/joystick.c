#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "joystick.h"
#include <stdlib.h> 
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

//Declaro el hilo 2 como externo
osThreadId_t joystick;                        // thread id
osMessageQueueId_t queue_joystick;  

int init_Joystick (void); 
void THJoystick (void *argument);                   // thread function

uint8_t joystick_value = 0;
uint32_t status;
 
//Timer para eliminar rebote del botón de joystick
void Timer1_Callback(void *arg);

//Timer para medir la duración de la pulsación por botón
void Timer2_Callback(void *arg);

int init_Joystick (void) {
	queue_joystick = osMessageQueueNew(16, sizeof(uint8_t), NULL);
  joystick = osThreadNew(THJoystick, NULL, NULL);
  if (joystick == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void THJoystick (void *argument) {
	osTimerId_t timsoft1 = osTimerNew(Timer1_Callback, osTimerOnce, NULL, NULL);
	osTimerStart(timsoft1, 50);
	
  while (1) {	
		status = osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		switch(status)
		{
			case 1:
				osTimerStart(timsoft1, 50);
		}
  }
}

void Timer1_Callback(void *arg){	
	//----------Pasados los 50 ms leemos el valor del pin para eliminar los rebotes--------
	osTimerId_t timsoft2 = osTimerNew(Timer2_Callback, osTimerOnce, NULL, NULL);
	
	osTimerStart(timsoft2, 1000);
	
	//Arriba 1 p15 mbed
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
		joystick_value = 1;
	}
	//Derecha 2 p16 mbed
	else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET){
		joystick_value = 2;
	}
	//Abajo 3 p12 mbed
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
		joystick_value = 3;
	}
	//Izquierda 4 p13 mbed
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){
		joystick_value = 4;
	}
	//Pulsacion en el centro 5 p14 mbed
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){
		joystick_value = 5;
	}
}		

void Timer2_Callback(void *arg){	
	uint8_t msg;
	//Arriba 1 p15 mbed
	if(joystick_value == 1){
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
			msg = 0b10001; //17
		}
		else{
			msg = 0b00001;
		}
	}
	//Derecha 2 p16 mbed
	else if(joystick_value == 2){
		if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET){
			msg = 0b10010; //18
		}
		else{
			msg = 0b00010;
		}
	}
	//Abajo 3 p12 mbed
	else if(joystick_value == 3){
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
			msg = 0b10011; //19
		}
		else{
			msg = 0b00011;
		}
	}
	//Izquierda 4 p13 mbed
	else if(joystick_value == 4){
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){
			msg = 0b10100; //20
		}
		else{
			msg = 0b00100;
		}
	}
	//Pulsacion en el centro 5 p14 mbed
	else if(joystick_value == 5){
		if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){
			msg = 0b10101; //21
		}
		else{
			msg = 0b00101;
		}
	}
	status = osMessageQueuePut(queue_joystick, &msg, 0U, 0U);
}