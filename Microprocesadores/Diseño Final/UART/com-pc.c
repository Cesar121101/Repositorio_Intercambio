#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "Driver_USART.h"

osThreadId_t com_recibir;                        // thread id
osThreadId_t com_enviar;                        // thread id
void recibir_Func(void *argument);                   // thread function
void enviar_Func(void *argument);                   // thread function

typedef struct {                               
	char informacion[3];
	char cadenaRecibir[8];
	char final;
} msgQueue_Recibir;

typedef struct { 
	char cadenaEnviar[39];
} msgQueue_Enviar;

/* USART Driver */
extern ARM_DRIVER_USART Driver_USART3;
ARM_DRIVER_USART * USARTdrv = &Driver_USART3;

osMessageQueueId_t queue_recibir;
osMessageQueueId_t queue_enviar;


void myUSART_callback(uint32_t event)
{
  uint32_t mask;
  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
         ARM_USART_EVENT_TRANSFER_COMPLETE |
         ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
  if (event & mask) {
    /* Success: Wakeup Thread */
    if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
			osThreadFlagsSet(com_recibir,0x1);
		}else if(event & ARM_USART_EVENT_SEND_COMPLETE){
			osThreadFlagsSet(com_enviar,0x1);
		}
  }
  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
  }
  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
  }
}

void init_USART(){
	/*Initialize the USART driver */
	USARTdrv->Initialize(myUSART_callback);
	/*Power up the USART peripheral */
	USARTdrv->PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 115200 Bits/sec */
	USARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
										ARM_USART_DATA_BITS_8 |
										ARM_USART_PARITY_NONE |
										ARM_USART_STOP_BITS_1 |
										ARM_USART_FLOW_CONTROL_NONE, 115200);
	 
	/* Enable Receiver and Transmitter lines */
	USARTdrv->Control (ARM_USART_CONTROL_TX, 1);
	USARTdrv->Control (ARM_USART_CONTROL_RX, 1);
}

int init_COM(void) {
	com_recibir = osThreadNew(recibir_Func, NULL, NULL);
	com_enviar = osThreadNew(enviar_Func, NULL, NULL);
	if (com_recibir == NULL) {
    return(-1);
  }
	init_USART();
	queue_enviar = osMessageQueueNew(2, sizeof(msgQueue_Enviar), NULL);
	queue_recibir = osMessageQueueNew(2, sizeof(msgQueue_Recibir), NULL);
  return(0);
}

uint8_t size;
void recibir_Func (void *argument) {
	uint32_t status;
	msgQueue_Recibir mensaje;
  while (1) {
		USARTdrv->Receive(&mensaje.informacion, 3); //SOH, Comando, Longuitud
		status = osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		size = mensaje.informacion[2] - 0x4;
		USARTdrv->Receive(&mensaje.cadenaRecibir,size); //Informacion del mensaje
		status = osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		USARTdrv->Receive(&mensaje.final, 1); //Final de la trama
		status = osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				osMessageQueuePut(queue_recibir, &mensaje, 0U, 0U);
				break;
		}
  }
}

void enviar_Func (void *argument) {
	msgQueue_Enviar mensaje;
	osStatus_t status;
  while (1) {
		status = osMessageQueueGet(queue_enviar, &mensaje, NULL, osWaitForever);
		USARTdrv->Send(&mensaje.cadenaEnviar, 39);
		status=osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
	}
}

void test_Func(void *argument);

int init_COM_test(void) {
	osThreadId_t com_test;   
	com_test = osThreadNew(test_Func, NULL, NULL);
	if (com_test == NULL) {
    return(-1);
  }
  return(0);
}

void test_Func (void *argument) {
	msgQueue_Recibir mensajeRecibir;
	msgQueue_Enviar mensajeEnviar;
	int contador = 0;
	char brinco[2] = "\n\r";
	osStatus_t status;
  while (1) {
		status = osMessageQueueGet(queue_recibir, &mensajeRecibir, NULL, osWaitForever);
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
		for(int i = 0; i < 2; i++){
			mensajeEnviar.cadenaEnviar[contador] = brinco[i];
			contador++;
		}
		osMessageQueuePut(queue_enviar, &mensajeEnviar, 0U, 0U);
		contador = 0;
  }
}
