#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"
#include "lcd.h"

/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t Timer;                        // thread id 
osMessageQueueId_t id_MsgQueue;
extern osThreadId_t Listener;

osTimerId_t timsoft1;
char str[50];
int i;
uint32_t segundos =60;
uint32_t minutos = 2;
uint32_t multiplo = 160;

void Timer_Func (void *argument);                   // thread function
void Timer_Callback(void *arg);

//Callback del timer
void Timer_Callback(void *arg){
	uint32_t status;
	LCD_clear();
	if(segundos == 60){
		segundos = 0;
		sprintf(str,"Crono-> %02d:%02d", minutos, segundos);
		for(i = 0; i<sizeof(str); i++){
			if(str[i] != NULL){
				symbolToLocalBuffer(1,str[i]);
			}
		}
		segundos = 60;
		minutos--;
	}
	else{
		sprintf(str,"Crono-> %02d:%02d", minutos, segundos);
		for(i = 0; i<sizeof(str); i++){
			if(str[i] != NULL){
				symbolToLocalBuffer(1,str[i]);
			}
		}
	}
	if(multiplo >= 100 && multiplo%4 == 0){
		status = osThreadFlagsSet(Listener,0x0001);
	}
	segundos --;
	multiplo--;
	if(minutos > 0 && segundos == 0){
		segundos = 60;
	}
	if(minutos == 0 && segundos == -1){
		osTimerStop(timsoft1);
		status = osThreadFlagsSet(Listener,0x0002);
	}
	LCD_update();
}

int Init_JoyThread (void) {
	Timer = osThreadNew(Timer_Func, NULL, NULL); //Crear el Thread del timer
	if (Timer == NULL) {
    return(-1);
  }
	LCD_Init();
	LCD_clear();
  return(0);
}
 
void Timer_Func (void *argument) {
	uint32_t status;
	timsoft1 = osTimerNew(Timer_Callback, osTimerPeriodic, NULL, NULL); //Crear el timer	
	osTimerStart(timsoft1,1000); //Reiniciar el timer
	status = osThreadFlagsSet(Listener,0x0001);
  while (1) {
  }
}