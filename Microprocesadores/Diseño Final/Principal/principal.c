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
extern osThreadId_t clock;
extern osTimerId_t timer_clock;

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
int num = 0;
char memoria[10][35]; //Buffer circular
msgQueue_LCD mensaje; //Mensaje LCD

void modoReposo(){
	//Limpiar mensaje LCD
	for(int i = 0; i < sizeof(mensaje.cadena); i++){ 
		mensaje.cadena[i] = ' ';
	}
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
	Tm = round(Tm*2.0)/2.0;
	Tr = round(Tr*2.0)/2.0;
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
	//Limpiar mensaje LCD
	for(int i = 0; i < sizeof(mensaje.cadena); i++){ 
		mensaje.cadena[i] = ' ';
	}
	//Redondear valor de temperatura
	float Tr_r = round(Tr*2.0)/2.0; 
	//Enviar datos al LCD
	sprintf(mensaje.cadena,"      ---P&D---");
	mensaje.linea = 1;
	mensaje.subrayar = 0;
	mensaje.numero = 0;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
	sprintf(mensaje.cadena," H: %02d:%02d:%02d ---Tr:%04.1f", horas,minutos,segundos,Tr_r);
	mensaje.linea = 2;
	mensaje.subrayar = 1;
	mensaje.numero = num;
	osMessageQueuePut(queue_lcd, &mensaje, 0U, 0U);
}

void aumentarNumero(){
	uint32_t decenas = 0, unidades = 0;
	int entera;
	float decimal;
	switch(num){
		case 0:
			if(horas%10 < 4){
				horas += 10;
				if(horas/10 > 2){
					unidades = horas%10;
					horas = unidades;
				}
			}else{
				horas += 10;
				if(horas/10 > 1){
					unidades = horas%10;
					horas = unidades;
				}
			}
			break;
		case 1: 
			if(horas > 19){
				horas += 1;
				if(horas%10 > 3){
					decenas = horas/10;
					horas = decenas*10;
				}
			}else{
				horas += 1;
				if(horas%10 == 0){
					decenas = horas/10 - 1;
					horas = decenas*10;
				}
			}
			break;
		case 2:
			minutos += 10;
			if(minutos/10 > 5){
				unidades = minutos%10;
				minutos = unidades;
			}
			break;
		case 3: 
			minutos += 1;
			if(minutos%10 == 0){
				decenas = minutos/10;
				minutos = (decenas-1)* 10;
			}
			break;
		case 4:
			segundos += 10;
			if(segundos/10 > 5){
				unidades = segundos%10;
				segundos = unidades;
			}
			break;
		case 5: 
			segundos += 1;
			if(segundos%10 == 0){
				decenas = segundos/10;
				segundos = (decenas-1)* 10;
			}
			break;
		case 6:
			if(Tr < 26){
				Tr += 10;
				entera = (int)Tr;
				decimal = Tr - entera;
				if(entera%10 == 0 && decimal == 0.0){
					if(entera/10 > 3){
						unidades = entera%10;
						Tr = unidades + decimal;
					}
				}else{
					if(entera/10 > 2){
						unidades = entera%10;
						Tr = unidades + decimal;
					}
				}
			}
			break;
		case 7: 
			if(Tr < 30){
				Tr += 1;
				entera = (int)Tr;
				decimal = Tr - entera;
				if(entera%10 == 0){
					decenas = entera/10;
					Tr = (decenas-1)* 10 + decimal;
				}
			}
			break;
		case 8:
			if(Tr < 30){
				entera = (int)Tr;
				decimal = Tr - entera;
				if(decimal == 0.5){
					decimal = 0.0;
				}else if (decimal == 0.0){
					decimal = 0.5;
				}
				Tr = entera + decimal;
			}
			break;
	}
}

void disminuirNumero(){
	uint32_t decenas = 0, unidades = 0;
	int entera;
	float decimal;
	switch(num){
		case 0:
			if(horas > 9){
				horas -= 10;
			}
			break;
		case 1: 
			if(horas > 0){
				if(horas > 19){
					horas -= 1;
					if(horas%10 == 9){
						decenas = horas/10 + 1;
						horas = decenas*10 + 3;
					}
				}else{
					horas -= 1;
					if(horas%10 == 9){
						decenas = horas/10 + 1;
						horas = decenas*10 + 9;
					}
				}
			}
			break;
		case 2:
			if(minutos > 9){
				minutos -= 10;
			}
			break;
		case 3: 
			if(minutos > 0){
				minutos -= 1;
				if(minutos%10 == 9){
					decenas = minutos/10 + 1;
					minutos = decenas*10 + 9;
				}
			}
			break;
		case 4:
			if(segundos > 9){
				segundos -= 10;
			}
			break;
		case 5: 
			if(segundos > 0){
				segundos -= 1;
				if(segundos%10 == 9){
					decenas = segundos/10 + 1;
					segundos = decenas*10 + 9;
				}
			}
			break;
		case 6:
			if(Tr > 14){
				Tr -= 10;
			}
			break;
		case 7:
			if(Tr > 5 && Tr < 30){
				Tr -= 1;
				entera = (int)Tr;
				decimal = Tr - entera;
				if(entera%10 == 9){
					decenas = entera/10 + 1;
					Tr = decenas* 10 + 9 + decimal;
				}
			}
			break;
		case 8:
			if(Tr < 30){
				entera = (int)Tr;
				decimal = Tr - entera;
				if(decimal == 0.5){
					decimal = 0.0;
				}else if (decimal == 0.0){
					decimal = 0.5;
				}
				Tr = entera + decimal;
			}
			break;
	}
}

void principal_Func (void *argument);

int init_Principal(void) {
	principal = osThreadNew(principal_Func, NULL, NULL);
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
			estado++;
			if(estado >= 4){
				estado = 0;
			}
			if(estado == 3){
				osThreadSuspend(clock);
				osTimerStop(timer_clock);
			}else if(estado == 0){
				osThreadResume(clock);
				osTimerStart(timer_clock,1000);
			}
		}else{
			if(joystick_p.pulsacion == 0){
				switch (joystick_p.joystick_value){
					case 1:
						aumentarNumero();
						break;
					case 2:
						num++;
						if(num > 8){
							num = 0;
						}
						break;
					case 3:
						disminuirNumero();
						break;
					case 4:
						num--;
						if(num < 0){
							num = 8;
						}
						break;
				}
			}
		}
		//Reiniciar el valor del joystick
		joystick_p.joystick_value = 0;
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