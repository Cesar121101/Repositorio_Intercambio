#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"
#include "string.h"
#include "lcd.h"
#include "joystick.h"

//Threads IDs
osThreadId_t principal;                         

//Queues IDs
extern osMessageQueueId_t queue_joystick;
extern osMessageQueueId_t queue_lcd;
//extern osMessageQueueId_t queue_adc;

//Variables
extern int horas,minutos,segundos;
char memoria[10][35];
msgQueue_LCD mensaje;

void modoReposo(){
	sprintf(mensaje.cadena,"     SBM 2023");
	mensaje.linea = 1;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	sprintf(mensaje.cadena,"      %02d:%02d:%02d", horas,minutos,segundos);
	mensaje.linea = 2;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
}

void modoActivo(){
	for(int i = 0; i < sizeof(mensaje.cadena); i++){
		mensaje.cadena[i] = ' ';
	}
	sprintf(mensaje.cadena,"   ACT---%02d:%02d:%02d---", horas,minutos,segundos);
	mensaje.linea = 1;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	sprintf(mensaje.cadena," Tm:21.5-Tr:22.5-D:80%%");
	mensaje.linea = 2;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	for(int i = 0; i < 9; i++){
		memcpy(memoria[i],memoria[i + 1], sizeof(memoria[i]));
	}
	sprintf(memoria[9],"%02d:%02d:%02d--Tm:XX.X--Tr:YY.Y--D:ZZ%%",horas,minutos,segundos);
}

void principal_Func (void *argument);                   // thread function
int init_Principal(void) {
	principal = osThreadNew(principal_Func, NULL, NULL); //Crear el Thread del timer
	if (principal == NULL) {
    return(-1);
  }
  return(0);
}

uint32_t estado = 0;
void principal_Func (void *argument) {
	MSGQUEUE_JOYSTICK joystick_p;
	while (1) {
		osMessageQueueGet(queue_joystick, &joystick_p, NULL, 1000);
		if(joystick_p.joystick_value == 5 && joystick_p.pulsacion == 1){
			joystick_p.joystick_value = 0;
			joystick_p.pulsacion = 0;
			estado = 1;
		}
		if(estado == 0){
			modoReposo();
		}else if(estado == 1){
			modoActivo();
		}
  }
}