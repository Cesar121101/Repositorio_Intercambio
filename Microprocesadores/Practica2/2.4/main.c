/**
  ******************************************************************************
  * @file    Templates/Src/main.c 
  * @author  MCD Application Team
  * @brief   STM32F4xx HAL API Template project 
  *
  * @note    modified by ARM
  *          The modifications allow to use this file as User Code Template
  *          within the Device Family Pack.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#ifdef RTE_CMSIS_RTOS2_RTX5
/**
  * Override default HAL_GetTick function
  */
uint32_t HAL_GetTick (void) {
  static uint32_t ticks = 0U;
         uint32_t i;

  if (osKernelGetState () == osKernelRunning) {
    return ((uint32_t)osKernelGetTickCount ());
  }

  /* If Kernel is not running wait approximately 1 ms then increment 
     and return auxiliary tick counter value */
  for (i = (SystemCoreClock >> 14U); i > 0U; i--) {
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
  }
  return ++ticks;
}

#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
TIM_HandleTypeDef htim2; //Manejador del TIM2
TIM_HandleTypeDef htim5; //Manejador del TIM5
TIM_OC_InitTypeDef TIM_Channel_InitStruct; //Estructura en modo Output Compare
TIM_IC_InitTypeDef TIM_Channel_InitStruct2; //Estructura en modo Input Capture
uint32_t periodo = 4;
uint32_t contador = 0;
uint32_t captura[2];
uint32_t diffCapture = 0;
uint32_t TimerClock = 84000000;
uint32_t prescaler = 84;
float frecuencia;

static void init_GPIO(void){
	GPIO_InitTypeDef GPIO_InitStruct; //Definicion los GPIO
	
	__HAL_RCC_GPIOA_CLK_ENABLE();	 //Habilitar el reloj de los puertos GPIO A
	__HAL_RCC_GPIOB_CLK_ENABLE();	 //Habilitar el reloj de los puertos GPIO B
	__HAL_RCC_GPIOC_CLK_ENABLE();	 //Habilitar el reloj de los puertos GPIO C
	
	//Pin PC13 en modo interrupcion flanco de subida
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; //Habilitar el modo input
	GPIO_InitStruct.Pull = GPIO_NOPULL; //Habilitar resitencias pulldown del pin
	GPIO_InitStruct.Pin = GPIO_PIN_13; //Definir al pin 13
	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); //Inicializar el pin 13
	
	//Pin PB11 en modo funcion alternativa del timer 2
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; //Habilitar el modo funcion alternativa
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM2; //Definir el modo alternativo del pin
	GPIO_InitStruct.Pin = GPIO_PIN_11; //Definir al pin 11
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //Inicializar el pin 11
	
	//Pin PB0 en modo output
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //Habilitar el modo push pull de los GPIO
	GPIO_InitStruct.Pull = GPIO_PULLUP; //Habilitar resitencias pull up de los GPIO
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //Establecer velocidad de frecuencia en modo alto
	
	GPIO_InitStruct.Pin = GPIO_PIN_0; //Definir los pines
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //Inicializar los pines
	
	//Pin A' en modo funcion alternativa del timer 5
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; //Habilitar el modo funcion alternativa
	GPIO_InitStruct.Pull = GPIO_NOPULL; //Habilitar el modo funcion alternativa
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW; //Habilitar el modo funcion alternativa
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM5; //Definir el modo alternativo del pin
	GPIO_InitStruct.Pin = GPIO_PIN_0; //Definir al pin 0
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); //Inicializar el pin 0
}

//Timer 2 en modo Output Compare
static void init_Timer2(uint32_t periodo){
	
	htim2.Instance = TIM2; 
	htim2.Init.Prescaler = 8399; //Prescaler a 8400, El reloj de APB1 es de 84 MHz / Prescaler (8400) = 10KHz
	//Para obtener el tiempo dividimos periodo/frecuencia de conteo (en este caso 10KHz)
	htim2.Init.Period = periodo; //Frecuencia de conteo = 10KHz/Period = Frecuencia de interrupcion
	HAL_TIM_OC_Init(&htim2);
	
	//Configuraciones del Output
	TIM_Channel_InitStruct.OCMode = TIM_OCMODE_TOGGLE;
	TIM_Channel_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_Channel_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
	
	//Establecer configuraciones
	HAL_TIM_OC_ConfigChannel(&htim2, &TIM_Channel_InitStruct, TIM_CHANNEL_4); //Configurar el canal
	HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4); //Iniciar el Output
}

//TIM5 en modo Input Capture
static void init_Timer5(void){
	
	htim5.Instance = TIM5; 
	htim5.Init.Prescaler = 83; //Prescaler a 84, El reloj de APB1 es de 84 MHz / Prescaler (84) = 1MHz
	htim5.Init.Period = 4294967295; //2^32
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_IC_Init(&htim5); //Iniciar el timer en modo input
	
	//Configuracion del input
	TIM_Channel_InitStruct2.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING; //Detectar flancos de subida
	TIM_Channel_InitStruct2.ICSelection = TIM_ICSELECTION_DIRECTTI; //Configuraciones directa de canales
	TIM_Channel_InitStruct2.ICPrescaler = TIM_ICPSC_DIV1; //Sin prescaler
	TIM_Channel_InitStruct2.ICFilter = 0;
	
	HAL_TIM_IC_ConfigChannel(&htim5, &TIM_Channel_InitStruct2, TIM_CHANNEL_1); //Configurar el canal
	HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1); //Iniciar las capturas
}

//Manejador de interrupciones del timer 5
void TIM5_IRQHandler(void){
	//Manejador de interrupciones del timer por HAL
	HAL_TIM_IRQHandler(&htim5);
}

//Callback de la interrupcion del HAL
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	if(contador == 0){ //Primer captura del timer
		captura[0] = HAL_TIM_ReadCapturedValue(&htim5,TIM_CHANNEL_1); //Leer el valor en el PIN
		contador = 1;
	}else if(contador == 1){ //Segunda captura del timer
		captura[1] = HAL_TIM_ReadCapturedValue(&htim5,TIM_CHANNEL_1);
		if(captura[1] >= captura[0]){
				diffCapture = captura[1] - captura[0];
			}else{ 
				diffCapture = (0xffffff - captura[0]) + captura[1];
			}
			frecuencia = TimerClock/prescaler;
			frecuencia = frecuencia / diffCapture; //Calculo de Frecuencia
			__HAL_TIM_SET_COUNTER(&htim5, 0); //Establecemos el contador del timer a 0
			contador = 0;
		}
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
}

//Manejador de interrupciones de los pines 10-15
void EXTI15_10_IRQHandler(void){
	//Manejador de interrupcioes de los GPIO HAL
		HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}
	
//Callback de las interrupciones de GPIO por HAL
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN){
		if(periodo == 4){
			periodo = 1;
		}
		else if(periodo == 1){
			periodo = 4;
		}
		HAL_TIM_OC_Stop(&htim2,TIM_CHANNEL_4); //Detener el timer para cambiar periodo
		init_Timer2(periodo);
}	
	
int main(void)
{
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user 
             can eventually implement his proper time base source (a general purpose 
             timer for example or other time source), keeping in mind that Time base 
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* Add your application code here
     */
	init_GPIO(); //Llamada de la funcion de iniciar los GPIO
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //Habilitar la interrupcion del los puertos 10-15
	__HAL_RCC_TIM2_CLK_ENABLE(); //Habilitar reloj del timer 2
	__HAL_RCC_TIM5_CLK_ENABLE(); //Habilitar reloj del timer 5
	HAL_NVIC_EnableIRQ(TIM5_IRQn); //Habilitar las interrupciones del timer 5
	init_Timer2(periodo); //LLamar a la funcion de iniciar el Timer 2
	init_Timer5(); //Llamar a la funcion de inciar el Timer 5

#ifdef RTE_CMSIS_RTOS2
  /* Initialize CMSIS-RTOS2 */
  osKernelInitialize ();

  /* Create thread functions that start executing, 
  Example: osThreadNew(app_main, NULL, NULL); */

  /* Start thread execution */
  osKernelStart();
#endif

  /* Infinite loop */
  while (1)
  {
	}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
