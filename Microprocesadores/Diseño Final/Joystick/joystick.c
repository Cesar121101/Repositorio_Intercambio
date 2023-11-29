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

static osStatus_t status;
uint8_t aux;
uint8_t aux2;
static uint8_t msg;
static uint8_t flags;
uint32_t tiempo_init_boton = 0;
uint32_t tiempo_transcurrido = 0;
uint8_t activo = 0;

//Objeto donde publicaremos la pulsación y si fue corta o larga
MSGQUEUE_JOYSTICK localObject;
 
//Timer para eliminar rebote del botón de joystick
void Timer1_Callback(void *arg);

//Timer para medir la duración de la pulsación por botón
void Timer2_Callback(void *arg);

int init_Joystick (void) {
	queue_joystick = osMessageQueueNew(16, sizeof(MSGQUEUE_JOYSTICK), NULL);
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
	uint8_t t_final = 100; //(100 ticks del sistema)
	tiempo_transcurrido = 0;
	tiempo_init_boton = HAL_GetTick();
	
	//Pulsaciones cortas
	//Arriba 1
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
		localObject.joystick_value = 1;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
			else{
				localObject.pulsacion = 0;
			}
		}while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET);
	}
	
	//Derecha 2
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET){
		localObject.joystick_value = 2;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
			else{
				localObject.pulsacion = 0;
			}
		}while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET);
	}
	
	//Abajo 3
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
		localObject.joystick_value = 3;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
			else{
				localObject.pulsacion = 0;
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET);
	}
	
	//Izquierda 4	
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){
		localObject.joystick_value = 4;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
			else{
				localObject.pulsacion = 0;
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET);
	}
		
	//Centro 5
	if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){
		localObject.joystick_value = 5;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
			else{
				localObject.pulsacion = 0;
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET);
	}
	
	//Publicamos el valor
	aux = localObject.joystick_value;
	aux2 = localObject.pulsacion;
	
	status = osMessageQueuePut(queue_joystick, &localObject, 0U, osWaitForever);
	
}		