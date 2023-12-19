#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h>                        // CMSIS RTOS header file
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "clock.h"
#include "rgb.h"
#include "lcd.h"
#include "com-pc.h"
#include "pot1pot2.h"
#include "joystick.h"
#include "temperatura.h"
#include "pwm.h"

//Threads IDs
osThreadId_t principal; 
extern osThreadId_t clock;
extern osThreadId_t adc;
extern osThreadId_t temperatura;
extern osTimerId_t timer_clock;

//Queues IDs
extern osMessageQueueId_t queue_joystick;
extern osMessageQueueId_t queue_lcd;
extern osMessageQueueId_t queue_temperatura;
extern osMessageQueueId_t queue_rgb;
extern osMessageQueueId_t queue_adc;
extern osMessageQueueId_t queue_recibir;
extern osMessageQueueId_t queue_enviar;
extern osMessageQueueId_t queue_pwm;

//Variables
extern int horas,minutos,segundos;
float Tr = 25; //Temperatura de referencia
float Tm = 0; //Temperatura del sistema
float TrP; //Temperatura de referencia provisional
int num = 0; //Numero a subrayar
int numeroMediciones = 0; //Numero de mediciones en el buffer circular
int horasP, minutosP, segundosP; //Horas provisionales en caso de horas invalidas
char memoria[10][34]; //Buffer circular
msgQueue_LCD mensajeLCD; //mensajeLCD LCD

void modoReposo(){
	msgQueue_PWM mensajePWM;
	//Enviar datos al PWM
	mensajePWM.Tm = -1;
	mensajePWM.Tr = -1;
	osMessageQueuePut(queue_pwm, &mensajePWM, 0U, 0U);
	//Limpiar mensajeLCD LCD
	for(int i = 0; i < sizeof(mensajeLCD.cadena); i++){ 
		mensajeLCD.cadena[i] = ' ';
	}
	//Enviar mensajeLCD al LCD
	sprintf(mensajeLCD.cadena,"     SBM 2023");
	mensajeLCD.linea = 1;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U);
	sprintf(mensajeLCD.cadena,"      %02d:%02d:%02d", horas,minutos,segundos);
	mensajeLCD.linea = 2;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U); //Enviar datos al LCD
}

void modoActivo(){
	msgQueue_RGB mensajeRGB;
	msgQueue_PWM mensajePWM;
	//Limpiar mensajeLCD LCD
	for(int i = 0; i < sizeof(mensajeLCD.cadena); i++){ 
		mensajeLCD.cadena[i] = ' ';
	}
	//Leer datos de temperatura
	osMessageQueueGet(queue_temperatura, &Tm, NULL, 500); 
	//Redondear valores de temperatura
	Tr = round(Tr*2.0)/2.0; 
	Tm = round(Tm*2.0)/2.0;
	//Enviar datos al RGB
	mensajeRGB.Tm = Tm; 
	mensajeRGB.Tr = Tr;
	osMessageQueuePut(queue_rgb, &mensajeRGB, 0U, 0U);
	//Enviar datos al PWM
	mensajePWM.Tm = Tm;
	mensajePWM.Tr = Tr;
	float diferencia = Tr - Tm;
	int pwm;
	if(diferencia >= 5){
		pwm = 100;
	}else if(diferencia >= 0 && diferencia < 5){
		pwm = 80;
	}else if(diferencia >-5 && diferencia < 0){
		pwm = 40;
	}else if(diferencia <= -5){
		pwm = 0;
	}
	osMessageQueuePut(queue_pwm, &mensajePWM, 0U, 0U);
	//Enviar datos al LCD
	sprintf(mensajeLCD.cadena,"   ACT---%02d:%02d:%02d---", horas,minutos,segundos);
	mensajeLCD.linea = 1;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U);
	sprintf(mensajeLCD.cadena," Tm:%04.1f-Tr:%04.1f-D:%d%%",Tm,Tr,pwm);
	mensajeLCD.linea = 2;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U); 
	//Guardar mediciones en el buffer circular
	for(int i = 0; i < 9; i++){ 
		memcpy(memoria[i],memoria[i + 1], sizeof(memoria[i]));
	}
	sprintf(memoria[9],"%02d:%02d:%02d--Tm:%4.1f--Tr:%4.1f--%d%%",horas,minutos,segundos,Tm,Tr,pwm);
	if(numeroMediciones < 10){
		numeroMediciones++;
	}
}

void modoTest(){
	msgQueue_RGB mensajeRGB;
	msgQueue_PWM mensajePWM;
	msgQueue_ADC mensajeADC;
	//Limpiar mensajeLCD LCD
	for(int i = 0; i < sizeof(mensajeLCD.cadena); i++){ 
		mensajeLCD.cadena[i] = ' ';
	}
	//Leer valores de temperatura
	osThreadFlagsSet(adc,0x1);
	osMessageQueueGet(queue_adc, &mensajeADC, NULL, 500); 
	Tm = mensajeADC.pot1;
	Tr = mensajeADC.pot2;
	//Redondear valores de temperatura
	Tr = round(Tr*2.0)/2.0; 
	Tm = round(Tm*2.0)/2.0;
	//Enviar datos al RGB
	mensajeRGB.Tm = Tm;
	mensajeRGB.Tr = Tr;
	osMessageQueuePut(queue_rgb, &mensajeRGB, 0U, 0U);
	//Enviar datos al PWM
	mensajePWM.Tm = Tm;
	mensajePWM.Tr = Tr;
	float diferencia = Tr - Tm;
	int pwm;
	if(diferencia >= 5){
		pwm = 100;
	}else if(diferencia >= 0 && diferencia < 5){
		pwm = 80;
	}else if(diferencia >-5 && diferencia < 0){
		pwm = 40;
	}else if(diferencia <= -5){
		pwm = 0;
	}
	osMessageQueuePut(queue_pwm, &mensajePWM, 0U, 0U);
	//Enviar datos al LCD
	sprintf(mensajeLCD.cadena,"  TEST---%02d:%02d:%02d---", horas,minutos,segundos);
	mensajeLCD.linea = 1;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U);
	sprintf(mensajeLCD.cadena," Tm:%04.1f-Tr:%04.1f-D:%d%%",Tm,Tr,pwm);
	mensajeLCD.linea = 2;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U); 
}


void mensajeUART(){
	uint32_t horaP, minP, segP;
	float trP;
	uint32_t contador = 0;
	char brinco[2] = "\n\r";
	uint8_t enviar = 0;
	msgQueue_Enviar mensajeEnviar;
	msgQueue_Recibir mensajeRecibir;
	osMessageQueueGet(queue_recibir, &mensajeRecibir, NULL, 0);
	//Filtrar mensajes
	if(mensajeRecibir.informacion[0] == 0x01 && mensajeRecibir.final == 0xFE){
		switch(mensajeRecibir.informacion[1]){
			case 32: //0x20 Poner horas
				horaP = atoi(&mensajeRecibir.cadenaRecibir[0]);
				minP = atoi(&mensajeRecibir.cadenaRecibir[3]);
				segP = atoi(&mensajeRecibir.cadenaRecibir[6]);
				if(horaP < 24 && minP < 60 && segP < 60){
					horas = horaP;
					minutos = minP;
					segundos = segP;
					horasP = horaP;
					minutosP = minP;
					segundosP = segP;
					enviar = 1;
				}
				//Complemento a 1 del comando
				mensajeRecibir.informacion[1] = ~mensajeRecibir.informacion[1];
				break;
			case 37: //0x25 Poner temperatura de referencia
				trP = atof(&mensajeRecibir.cadenaRecibir[0]);
				if(trP >= 5 && trP <= 30){
					Tr = trP;
					TrP = Tr;
					enviar = 1;
				}
				//Complemento a 1 del comando
				mensajeRecibir.informacion[1] = ~mensajeRecibir.informacion[1];
				break;
			case 85: //0x55  Enviar medidas del buffer
				for(int i = 9; i >= 10-numeroMediciones; i--){
					contador = 0;
					mensajeEnviar.cadenaEnviar[0] = 0x01;
					mensajeEnviar.cadenaEnviar[1] = 0xAF;
					mensajeEnviar.cadenaEnviar[2] = 0x25;
					for(int j = 3; j < 36; j++){
						mensajeEnviar.cadenaEnviar[j] = memoria[i][contador];
						contador++;
					}
					mensajeEnviar.cadenaEnviar[36] = 0xFE;
					contador = 0;
					for(int i = 37; i < 39; i++){
						mensajeEnviar.cadenaEnviar[i] = brinco[contador];
						contador++;
					}
					osMessageQueuePut(queue_enviar, &mensajeEnviar, 0U, 0U);
				}
				break;
			case 96: //0x60 //Borrar medidas del buffer
				for(int i = 0; i < 10; i++){
					sprintf(memoria[i]," ");
				}
				//Complemento a 1 del comando
				mensajeRecibir.informacion[1] = ~mensajeRecibir.informacion[1];	
				//Borrar numero de mediciones
				numeroMediciones = 0;
				enviar = 1;
				break;
		}
		if(mensajeRecibir.informacion[1] != 85 && enviar == 1){
			//Enviar datos de regreso a la PC
			for(int i=0; i < 3; i++){
				mensajeEnviar.cadenaEnviar[contador] = mensajeRecibir.informacion[i];
				contador++;
			}
			for(int i=0; i < mensajeRecibir.informacion[2]-0x4; i++){
				mensajeEnviar.cadenaEnviar[contador] = mensajeRecibir.cadenaRecibir[i];
				contador++;
			}
			mensajeEnviar.cadenaEnviar[contador] = mensajeRecibir.final;
			contador++;
			for(int i = contador; i < 37; i++){
				mensajeEnviar.cadenaEnviar[i] = ' ';
				contador++;
			}
			contador = 0;
			for(int i = 37; i < 39; i++){
				mensajeEnviar.cadenaEnviar[i] = brinco[contador];
				contador++;
			}
			contador = 0;
			osMessageQueuePut(queue_enviar, &mensajeEnviar, 0U, 0U);
		}
	}
}

void modoPD(){
	msgQueue_RGB mensajeRGB;
	msgQueue_RGB mensajePWM;
	//Enviar datos al RGB
	mensajeRGB.Tm = -1;
	mensajeRGB.Tr = -1;
	osMessageQueuePut(queue_rgb, &mensajeRGB, 0U, 0U);
	//Enviar datos al PWM
	mensajePWM.Tm = -1;
	mensajePWM.Tr = -1;
	osMessageQueuePut(queue_pwm, &mensajePWM, 0U, 0U);
	//Limpiar mensajeLCD LCD
	for(int i = 0; i < sizeof(mensajeLCD.cadena); i++){ 
		mensajeLCD.cadena[i] = ' ';
	}
	//Recibir mensajes de la UART
	mensajeUART();
	//Redondear valor de temperatura
	Tr = round(Tr*2.0)/2.0; 
	//Enviar datos al LCD
	sprintf(mensajeLCD.cadena,"      ---P&D---");
	mensajeLCD.linea = 1;
	mensajeLCD.subrayar = 0;
	mensajeLCD.numero = 0;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U);
	sprintf(mensajeLCD.cadena," H: %02d:%02d:%02d ---Tr:%04.1f  ", horas,minutos,segundos,Tr);
	mensajeLCD.linea = 2;
	mensajeLCD.subrayar = 1;
	mensajeLCD.numero = num;
	osMessageQueuePut(queue_lcd, &mensajeLCD, 0U, 0U);
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
			Tr += 10;
			entera = (int)Tr;
			decimal = Tr - entera;
			unidades = entera%10;
			if(entera%10 == 0 && decimal == 0.0){
				if(entera/10 > 3){ //Si las unidades y los decimales son 0, podemos llegar a 3 en las decenas
					if(unidades > 4.5){ //Si las unidades son mayores a 4.5 podemos regresar a 0
						Tr = unidades + decimal;
					}else{ //Si no regresamos a 1 en las decenas
						Tr = 10 + unidades + decimal;
					}
				}
			}else{
				if(entera/10 > 2){ //Si las unidades o los decimales no son 0, las decenas solo pueden llegar a 2
					if(unidades > 4){ //Si las unidades son mayores a 4 podemos regresar al 0 en las decenas
						Tr = unidades + decimal;
					}else{ //Si no regresamos al 1
						Tr = 10 + unidades + decimal;
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
					if(decenas > 1){
						Tr = (decenas-1)* 10 + decimal;
					}
					else{
						Tr = (decenas-1)* 10 + 5 + decimal;
					}
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
			if(Tr > 14.5){
				Tr -= 10;
			}
			break;
		case 7:
			if(Tr > 5.5 && Tr < 30){
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
	init_Clock();
	init_Joystick();
	init_LCD();
	init_Temperatura();
	init_RGB();
	init_ADC();
	init_COM();
	init_PWM();
  return(0);
}

uint32_t estado = 0;
void principal_Func (void *argument) {
	MSGQUEUE_JOYSTICK joystick_p;
	uint8_t guardarHoras = 0;
	float T;
	while (1) {
		osMessageQueueGet(queue_temperatura, &T, NULL, 0);
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
				horasP = horas;
				minutosP = minutos;
				segundosP = segundos;
				TrP = Tr;
			}else if(estado == 0){
				if(guardarHoras == 0){
					horas = horasP;
					minutos = minutosP;
					segundos = segundosP;
					Tr = TrP;
				}else{
					guardarHoras = 0;
				}
				osThreadResume(clock);
				osTimerStart(timer_clock,1000);
			}
		}else{
			if(estado == 3){
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
						case 5:
							if(horasP != horas || minutosP != minutos || segundosP != segundos || TrP != Tr){
								guardarHoras = 1;
							}
					}
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