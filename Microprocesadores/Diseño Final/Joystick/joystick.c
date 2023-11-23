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
osStatus_t status;
uint8_t msg;
uint8_t flags;
uint32_t tiempo_init_boton = 0;
uint32_t tiempo_transcurrido = 0;

 
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
	uint8_t msg;
	
  while (1) {	
		flags = osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
		switch(flags)
		{
			case 1:
				osTimerStart(timsoft1, 50);
		}
  }
}

void Timer1_Callback(void *arg){	
	//----------Pasados los 50 ms leemos el valor del pin para eliminar los rebotes--------
	osTimerId_t timsoft2 = osTimerNew(Timer2_Callback, osTimerOnce, NULL, NULL);
	int t_final = 130; //aprox 1 segundos (130 ticks del sistema)
	tiempo_transcurrido = 0;
	tiempo_init_boton = HAL_GetTick();
	
	//Pulsaciones cortas
	//Arriba 1
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
		do{
			joystick_value = 1;
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido == t_final){
				osTimerStart(timsoft2, 1);
			}
		}while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET);
	}
	
	//Derecha 2
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET){
		do{
			joystick_value = 2;
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido == t_final){
				osTimerStart(timsoft2, 1);
			}
		}while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET);
	}
	
	//Abajo 3
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
		do{
			joystick_value = 3;
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido == t_final){
				osTimerStart(timsoft2, 1);
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET);
	}
	
	//Izquierda 4	
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){
		do{
			joystick_value = 4;
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido == t_final){
				osTimerStart(timsoft2, 1);
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET);
	}
		
	//Centro 5
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){
		do{
			joystick_value = 5;
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido == t_final){
				osTimerStart(timsoft2, 1);
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET);	
	}
	
	//Publicamos el valor si la pulsación fue corta
	if(!osTimerIsRunning(timsoft2)){
		status = osMessageQueuePut(queue_joystick, &joystick_value, 0U, 0U);
	}
}		

void Timer2_Callback(void *arg){	
	//Pulsaciones largas
	//Arriba 1
	if(joystick_value == 1){
		msg = 0b10001; //17
	}
	//Derecha 2
	else if(joystick_value == 2){
		msg = 0b10010; //18
	}
	//Abajo 3
	else if(joystick_value == 3){
		msg = 0b10011; //19
	}
	//Izquierda 4
	else if(joystick_value == 4){
		msg = 0b10100; //20
	}
	//Pulsacion en el centro 5
	else if(joystick_value == 5){
		msg = 0b10101; //21
	}
	status = osMessageQueuePut(queue_joystick, &msg, 0U, 0U);
}
