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

/*---------------------------------------------------------------------------
 *		Para visualizar el funcionamiento del thread, usar el watch 
 *		window con las variables GESTO y PULSACION, GESTO contiene:
 *		si es arriba = 1/abajo = 2/izquierda = 3/derecha = 4/centro = 5
 *		PULSACION contiene: si fue larga o corta, 0 si fue corta, 1 si fue larga
 *--------------------------------------------------------------------------*/

static osStatus_t status;
uint8_t gesto;
uint8_t pulsacion;
static uint8_t flags;
uint32_t tiempo_init_boton = 0;
uint32_t tiempo_transcurrido = 0;
int publicado = 0;

//Objeto donde publicaremos la pulsaci�n y si fue corta o larga
MSGQUEUE_JOYSTICK localObject;
 
//Timer para eliminar rebote del bot�n de joystick
void Timer1_Callback(void *arg);

//Timer para medir la duraci�n de la pulsaci�n por bot�n
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
	uint8_t t_final = 100; //(100 ticks del sistema) o 1 segundo
	tiempo_init_boton = HAL_GetTick();
	//osTimerId_t timsoft2 = osTimerNew(Timer2_Callback, osTimerOnce, NULL, NULL);
	
	//Pulsaciones cortas
	//Arriba 1
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET){
		localObject.joystick_value = 1;
		localObject.pulsacion = 0;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
		}while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET);
	}
	
	//Derecha 2
	else if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET){
		localObject.joystick_value = 2;
		localObject.pulsacion = 0;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
		}while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET);
	}
	
	//Abajo 3
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET){
		localObject.joystick_value = 3;
		localObject.pulsacion = 0;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET);
	}
	
	//Izquierda 4	
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET){
		localObject.joystick_value = 4;
		localObject.pulsacion = 0;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET);
	}
		
	//Centro 5
	else if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET){
		localObject.joystick_value = 5;
		localObject.pulsacion = 0;
		do{			
			tiempo_transcurrido = HAL_GetTick() - tiempo_init_boton;
			if(tiempo_transcurrido >= t_final){
				localObject.pulsacion = 1;
			}
		}while(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET);
	}
	else{
		localObject.joystick_value = 0;
		localObject.pulsacion = 0;
	}
	osDelay(50);
	gesto = localObject.joystick_value;
  pulsacion = localObject.pulsacion;
	status = osMessageQueuePut(queue_joystick, &localObject, 0U, osWaitForever);
}	