#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "temperatura.h"
#include "Driver_I2C.h"

#define LM75B_ADDRESS 0x48 // Dirección I2C del LM75B

osThreadId_t temperatura;
osMessageQueueId_t queue_temperatura; 

void THTemperatura (void *argument);                   // thread function

extern ARM_DRIVER_I2C Driver_I2C1;
ARM_DRIVER_I2C* I2Cdrv = &Driver_I2C1;

static volatile uint32_t I2C_Event;

void I2C_SignalEvent (uint32_t event) {
  /* Save received events */
  I2C_Event |= event;
 
  if (event & ARM_I2C_EVENT_TRANSFER_DONE) {
    /* Transfer or receive is finished */
		osThreadFlagsSet(temperatura, 0x1);
		osThreadFlagsSet(temperatura, 0x2);
  }
}

void init_I2C(void){
	/* Initialize the I2C driver */
	I2Cdrv->Initialize(I2C_SignalEvent);
	/* Power up the I2C peripheral */
	I2Cdrv->PowerControl(ARM_POWER_FULL);
	I2Cdrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
	I2Cdrv->Control(ARM_I2C_BUS_CLEAR, 0);
}

static void init_GPIO(void) {
	//Configuramos los pnes de SCL y SDA
  __HAL_RCC_GPIOB_CLK_ENABLE(); // Habilitar el reloj para el puerto GPIOB
	
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  // Configurar el pin PB8 como SCL
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; // Modo de salida en drenaje abierto
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1; // AF4 para I2C1
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Configurar el pin PB9 como SDA
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
} 

int init_Temperatura(void) {
  temperatura = osThreadNew(THTemperatura, NULL, NULL);
  if (temperatura == NULL) {
    return(-1);
  }
	queue_temperatura = osMessageQueueNew(8, sizeof(float), NULL);
  return (0);
}

void THTemperatura(void *argument) {
  // Configurar e inicializar el sensor LM75B
	init_I2C();
	float temperaturaSensor = 0.0f;
	uint16_t lecturaSensor;
	
  while (1) {
    // Leer la temperatura del sensor LM75B
		// Seleccionar el registro que contiene la temperatura
		uint8_t regAddress = 0;
		I2Cdrv->MasterTransmit(LM75B_ADDRESS, &regAddress, 1, true);
		osThreadFlagsWait(0x1, osFlagsWaitAny, osWaitForever);

		// Leer los datos desde el registro seleccionado
		uint8_t data[2];
		I2Cdrv->MasterReceive(LM75B_ADDRESS, data, 2, false);
		osThreadFlagsWait(0x2, osFlagsWaitAny, osWaitForever);
		// Combinar los dos bytes para formar el valor de temperatura completo (11 bits)
		lecturaSensor = ((data[0] << 8) | data[1]) >> 5;

		// Signo extendido para números negativos (si es necesario)
		if (lecturaSensor & (1 << 10)){
				lecturaSensor |= 0xFC00;
		}
		 
		temperaturaSensor = lecturaSensor * 0.125;
		
		osMessageQueuePut(queue_temperatura, &temperaturaSensor, 0U, 0U);
  }
}

void THTemperatura_test (void *argument);                   // thread function

float temperaturaSensor_test = 0.0f;

int init_Temperatura_test(void) {
	osThreadId_t temperatura_test = osThreadNew(THTemperatura_test, NULL, NULL);
  if (temperatura == NULL) {
    return(-1);
  }
  return (0);
}


void THTemperatura_test(void *argument) {
  while (1) {
    osMessageQueueGet(queue_temperatura, &temperaturaSensor_test, NULL, osWaitForever);
	}
}
