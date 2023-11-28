#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
#include "stm32f4xx_hal.h"
#define RESOLUTION_12B 4096U
#define VREF 3.3f

osThreadId_t adc;                        // thread id
void adc_Func(void *argument);                   // thread function

osMessageQueueId_t queue_adc;

typedef struct {                               
  float pot1;
	float pot2;
} msgQueue_ADC;

void ADC1_pins_F429ZI_config(){
	  GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }

int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADC_Instance){
  hadc->Instance = ADC_Instance;
  hadc->Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc->Init.Resolution = ADC_RESOLUTION_12B;
  hadc->Init.ScanConvMode = DISABLE;
  hadc->Init.ContinuousConvMode = DISABLE;
  hadc->Init.DiscontinuousConvMode = DISABLE;
  hadc->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc->Init.NbrOfConversion = 1;
  hadc->Init.DMAContinuousRequests = DISABLE;
	hadc->Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(hadc) != HAL_OK){
    return -1;
  }
	return 0;
}

float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel){
	ADC_ChannelConfTypeDef sConfig = {0};
	HAL_StatusTypeDef status;

	uint32_t raw = 0;
	float voltage = 0;
  sConfig.Channel = Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK){
    return -1;
  }
	HAL_ADC_Start(hadc);
	do {
		status = HAL_ADC_PollForConversion(hadc, 0); //This funtions uses the HAL_GetTick(), then it only can be executed wehn the OS is running
	}while(status != HAL_OK);
	
	raw = HAL_ADC_GetValue(hadc);
	voltage = raw*VREF/RESOLUTION_12B; 
	
	return voltage;
}

float valor_Temperatura(float voltaje){
	float valorN,valorF;
	valorN = (voltaje-0.02)/(3.2);
	valorF = valorN * (25) + 5;
	valorF = (valorF > 30) ? 30 : (valorF < 5) ? 5 : valorF;
	return valorF;
}

int init_ADC(void) {
	adc = osThreadNew(adc_Func, NULL, NULL); //Crear el Thread del timer
	if (adc == NULL) {
    return(-1);
  }
	queue_adc = osMessageQueueNew(4, sizeof(msgQueue_ADC), NULL);
  return(0);
}

void adc_Func (void *argument) {
  ADC_HandleTypeDef adchandle; //handler definition
	ADC1_pins_F429ZI_config(); //specific PINS configuration
	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
	msgQueue_ADC mensaje;
	float final1, final2, ant1, ant2;
	uint32_t status;
  while (1) {
    status=osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				final1 = valor_Temperatura(ADC_getVoltage(&adchandle , 10 ));
				final2 = valor_Temperatura(ADC_getVoltage(&adchandle , 3 ));
				mensaje.pot1 = final1;
				mensaje.pot2 = final2;
				osMessageQueuePut(queue_adc, &mensaje, 0U, 0U);
			break;
		}
  }
}

void adc_Func_test(void *argument);

int init_ADC_test(void) {
	osThreadId_t adc_test;
	adc_test = osThreadNew(adc_Func_test, NULL, NULL); //Crear el Thread del timer
	if (adc_test == NULL) {
    return(-1);
  }
  return(0);
}

float potenciometro1; //Variables globales para ver resultado del potenciometro 1
float potenciometro2; //Variables globales para ver resultado del potenciometro 2

void adc_Func_test(void *argument) {
	msgQueue_ADC mensaje;
  while (1) {
    osThreadFlagsSet(adc,0x1);
		osMessageQueueGet(queue_adc, &mensaje, NULL, 10U);   // wait for message
		potenciometro1 = mensaje.pot1;
		potenciometro2 = mensaje.pot2;
  }
}