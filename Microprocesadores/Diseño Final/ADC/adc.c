#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include <stdlib.h> 
#include "stm32f4xx_hal.h"
#define RESOLUTION_12B 4096U
#define VREF 3.3f

osThreadId_t adc;                        // thread id
void adc_Func(void *argument);                   // thread function

osMessageQueueId_t queue_adc;

/**
  * @brief config the use of analog inputs ADC123_IN10 and ADC123_IN13 and enable ADC1 clock
  * @param None
  * @retval None
  */
void ADC1_pins_F429ZI_config(){
	  GPIO_InitTypeDef GPIO_InitStruct;
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/*PC0     ------> ADC1_IN10
    PC3     ------> ADC1_IN13
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  }
/**
  * @brief Initialize the ADC to work with single conversions. 12 bits resolution, software start, 1 conversion
  * @param ADC handle
	* @param ADC instance
  * @retval HAL_StatusTypeDef HAL_ADC_Init
  */
int ADC_Init_Single_Conversion(ADC_HandleTypeDef *hadc, ADC_TypeDef  *ADC_Instance){
   /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
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

/**
  * @brief Configure a specific channels ang gets the voltage in float type. This funtion calls to  HAL_ADC_PollForConversion that needs HAL_GetTick()
  * @param ADC_HandleTypeDef
	* @param channel number
	* @retval voltage in float (resolution 12 bits and VRFE 3.3
  */
float ADC_getVoltage(ADC_HandleTypeDef *hadc, uint32_t Channel){
		ADC_ChannelConfTypeDef sConfig = {0};
		HAL_StatusTypeDef status;

		uint32_t raw = 0;
		float voltage = 0;
		 /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
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
	queue_adc = osMessageQueueNew(4, sizeof(float), NULL);
  return(0);
}

void adc_Func (void *argument) {
  ADC_HandleTypeDef adchandle; //handler definition
	ADC1_pins_F429ZI_config(); //specific PINS configuration
	ADC_Init_Single_Conversion(&adchandle , ADC1); //ADC1 configuration
	float value1, value2;
	float final1, final2, ant1, ant2;
	int pot1 = 1, pot2 = 2;
	uint32_t status;
  while (1) {
    status=osThreadFlagsWait(0x1,osFlagsWaitAny,osWaitForever);
		switch (status){
			case 1:
				value1=ADC_getVoltage(&adchandle , 10 );
				value2=ADC_getVoltage(&adchandle , 13 );
				final1 = valor_Temperatura(value1);
				final2 = valor_Temperatura(value2);
				if(ant1 != final1){
					ant1 = final1;
					osMessageQueuePut(queue_adc, &pot1, 0U, 0U);
					osMessageQueuePut(queue_adc, &final1, 0U, 0U);
				}
				if(ant2 != final2){
					ant2 = final2;
					osMessageQueuePut(queue_adc, &pot2, 0U, 0U);
					osMessageQueuePut(queue_adc, &final2, 0U, 0U);
				}
			break;
		}
  }
}
