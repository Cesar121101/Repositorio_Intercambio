#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"
#include "lcd.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t clock;                        // thread id 
osMessageQueueId_t id_MsgQueue;
extern osThreadId_t Listener;

osTimerId_t timsoft1;
char str[50];
int i;
uint32_t segundos =0;
uint32_t minutos = 0;
uint32_t horas = 0;


void clock_Func (void *argument);                   // thread function
void Timer_Callback(void *arg);

//Callback del timer
void Timer_Callback(void *arg){
	uint32_t status;
	LCD_clear();
	segundos++;
	if(segundos == 60){
		minutos++;
		segundos = 0;
	}
	if(minutos == 60){
		horas++;
		minutos = 0;
	}
	sprintf(str,"H:%02d M:%02d S:%02d", horas, minutos, segundos);
	for(i = 0; i<sizeof(str); i++){
		if(str[i] != NULL){
			symbolToLocalBuffer(1,str[i]);
		}
	}
	LCD_update();
}

int Init_ClockThread (void) {
	clock = osThreadNew(clock_Func, NULL, NULL); //Crear el Thread del timer
	if (clock == NULL) {
    return(-1);
  }
	LCD_Init();
	LCD_clear();
  return(0);
}
 
void clock_Func (void *argument) {
	timsoft1 = osTimerNew(Timer_Callback, osTimerPeriodic, NULL, NULL); //Crear el timer	
	osTimerStart(timsoft1,1000); //Reiniciar el timer
  while (1) {
  }
}