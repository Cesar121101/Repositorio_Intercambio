#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t clock;                        // thread id 

uint32_t segundos = 0;
uint32_t minutos = 0;
uint32_t horas = 0;

void clock_Func (void *argument);                   // thread function
void Timer_Callback(void *arg);

//Callback del timer
void Timer_Callback(void *arg){
	segundos++;
}

void verificar_Tiempo(){
	if(segundos >= 60){
		minutos++;
		segundos = 0;
	}
	if(minutos >= 60){
		horas++;
		minutos = 0;
	}
	if(horas >= 24){
		horas = 0;
	}
}

int init_Clock(void) {
	clock = osThreadNew(clock_Func, NULL, NULL); //Crear el Thread del timer
	if (clock == NULL) {
    return(-1);
  }
  return(0);
}

void clock_Func (void *argument) {
	osTimerId_t timsoft1;
	timsoft1 = osTimerNew(Timer_Callback, osTimerPeriodic, NULL, NULL); //Crear el timer	
	osTimerStart(timsoft1,1000); //Iniciar el timer

  while (1) {
		verificar_Tiempo();
	}
}

void clock_Func_Test(void *argument);
	
int init_Clock_Test(void) {
	osThreadId_t clock_test;
	clock_test = osThreadNew(clock_Func_Test, NULL, NULL); //Crear el Thread del timer
	if (clock == NULL) {
    return(-1);
  }
  return(0);
}

void clock_Func_Test(void *argument) {
	uint8_t flag = 0;
  while (1) {
		osDelay(3000);
		if(flag == 1){
			segundos = 55;
			minutos = 59;
			horas = 23;
			flag = 2;
		}
		if(flag == 0){
			segundos = 70;
			minutos = 90;
			horas = 25;
			flag = 1;
		}
  }
}