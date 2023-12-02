#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "lcd.h"
#include "joystick.h"
#include "rgb.h"
#include "pot1pot2.h"

//Threads IDs
osThreadId_t principal;                         

//Queues IDs
extern osMessageQueueId_t queue_joystick;
extern osMessageQueueId_t queue_lcd;
extern osMessageQueueId_t queue_temperatura;
extern osMessageQueueId_t queue_rgb;
extern osMessageQueueId_t queue_adc;

//Variables
extern int horas,minutos,segundos;
float Tr = 25; //Temperatura de referencia
float Tm = 0; //Temperatura del sistema
char memoria[10][35]; //Buffer circular
msgQueue_LCD mensaje; //Mensaje LCD

void modoReposo(){
	//Enviar mensaje al LCD
	sprintf(mensaje.cadena,"     SBM 2023");
	mensaje.linea = 1;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	sprintf(mensaje.cadena,"      %02d:%02d:%02d", horas,minutos,segundos);
	mensaje.linea = 2;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U); //Enviar datos al LCD
}


void modoActivo(){
	msgQueue_RGB mensajeRGB;
	//Leer datos de temperatura
	osMessageQueueGet(queue_temperatura, &Tm, NULL, 0); 
	//Limpiar mensaje LCD
	for(int i = 0; i < sizeof(mensaje.cadena); i++){ 
		mensaje.cadena[i] = ' ';
	}
	//Redondear valores de temperatura
	float Tr_r = round(Tr*2.0)/2.0; 
	float Tm_r = round(Tm*2.0)/2.0;
	//Enviar datos al RGB
	mensajeRGB.Tm = Tm_r; 
	mensajeRGB.Tr = Tr_r;
	osMessageQueuePut(queue_rgb, &mensajeRGB, 0U, 0U);
	//Enviar datos al PWM
	
	//Enviar datos al LCD
	sprintf(mensaje.cadena,"   ACT---%02d:%02d:%02d---", horas,minutos,segundos);
	mensaje.linea = 1;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	sprintf(mensaje.cadena," Tm:%04.1f-Tr:%04.1f-D:80%%",Tm_r,Tr_r);
	mensaje.linea = 2;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U); 
	//Guardar mediciones en el buffer circular
	for(int i = 0; i < 9; i++){ 
		memcpy(memoria[i],memoria[i + 1], sizeof(memoria[i]));
	}
	sprintf(memoria[9],"%02d:%02d:%02d--Tm:XX.X--Tr:YY.Y--D:ZZ%%",horas,minutos,segundos);
}

void modoTest(){
	msgQueue_RGB mensajeRGB;
	msgQueue_ADC mensajeADC;
	//Limpiar mensaje LCD
	for(int i = 0; i < sizeof(mensaje.cadena); i++){ 
		mensaje.cadena[i] = ' ';
	}
	//Leer valores de temperatura
	osMessageQueueGet(queue_adc, &mensajeADC, NULL, 0); 
	Tr = mensajeADC.pot1;
	Tm = mensajeADC.pot2;
	//Enviar datos al RGB
	mensajeRGB.Tm = Tm; 
	mensajeRGB.Tr = Tr;
	osMessageQueuePut(queue_rgb, &mensajeRGB, 0U, 0U);
	//Enviar datos al PWM
	
	//Enviar datos al LCD
	sprintf(mensaje.cadena,"  TEST---%02d:%02d:%02d---", horas,minutos,segundos);
	mensaje.linea = 1;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	sprintf(mensaje.cadena," Tm:%04.1f-Tr:%04.1f-D:80%%",Tm,Tr);
	mensaje.linea = 2;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U); 
}

void modoPD(){
	
}

void principal_Func (void *argument);                   // thread function
int init_Principal(void) {
	principal = osThreadNew(principal_Func, NULL, NULL); //Crear el Thread del timer
	if (principal == NULL) {
    return(-1);
  }
  return(0);
}

uint32_t estado = 2;
void principal_Func (void *argument) {
	MSGQUEUE_JOYSTICK joystick_p;
	while (1) {
		osMessageQueueGet(queue_joystick, &joystick_p, NULL, 1000);
		if(joystick_p.joystick_value == 5 && joystick_p.pulsacion == 1){
			joystick_p.joystick_value = 0;
			joystick_p.pulsacion = 0;
			estado++;
			if(estado >= 4){
				estado = 0;
			}
		}
		if(estado == 0){
			modoReposo();
		}else if(estado == 1){
			modoActivo();
		}else if(estado == 2){
			modoTest();
		}else if(estado == 3){
			modoPD();
		}
  }
}