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
#include "Driver_SPI.h"

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

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;
TIM_HandleTypeDef htim7; //Definimos el TIM7
unsigned char buffer[512];

void mySPI_callback(uint32_t event)
{
    switch (event)
    {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
        /* Success: Wakeup Thread */
        break;
    case ARM_SPI_EVENT_DATA_LOST:
        /*  Occurs in slave mode when data is requested/sent by master
            but send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when driver cannot transfer data fast enough. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    case ARM_SPI_EVENT_MODE_FAULT:
        /*  Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault. */
        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
        break;
    }
}

void init_SPI(void){
	/* Initialize the SPI driver */
	SPIdrv->Initialize(NULL);
	/* Power up the SPI peripheral */
	SPIdrv->PowerControl(ARM_POWER_FULL);
	/* Configure the SPI to Master, 8-bit mode @10000 kBits/sec */
	SPIdrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL1_CPHA1 | ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8), 20000000);
}

static void init_GPIO(void){
	GPIO_InitTypeDef GPIO_InitStruct; //Definicion los GPIO
	
	__HAL_RCC_GPIOD_CLK_ENABLE();	 //Habilitar el reloj de los puertos GPIO D
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; //Habilitar el modo push pull de los GPIO
	GPIO_InitStruct.Pull = GPIO_PULLUP; //Habilitar resitencias pull up de los GPIO
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //Establecer velocidad de frecuencia en modo alto
	
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_7; //Definir los pines
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct); //Inicializar los pines
}

void delay(uint32_t microsegundos)
{	
	uint32_t periodo = microsegundos - 1;
	
	// Configurar y arrancar el timer para generar un evento pasados n_microsegundos
	htim7.Instance = TIM7; 
	htim7.Init.Prescaler = 83; //Prescaler a 83, El reloj de APB1 es de 84 MHz / Prescaler (83) = 1MHz
	htim7.Init.Period = periodo; //Para obtener el tiempo dividimos periodo/frecuencia de conteo (en este caso 10KHz)
	__HAL_RCC_TIM7_CLK_ENABLE(); //Habilitar reloj del timer 7
	
	HAL_TIM_Base_Init(&htim7); //Configurar el timer
	HAL_TIM_Base_Start(&htim7); //Iniciar el timer
	
	// Esperar a que se active el flag del registro de desbordamiento (overflow)
	while ((htim7.Instance->SR & TIM_SR_UIF) == 0) {
	}
	// Borrar el flag de desbordamiento
	htim7.Instance->SR &= ~TIM_SR_UIF;

	// Parar el Timer y ponerlo a 0 para la siguiente llamada a la función
	HAL_TIM_Base_Stop(&htim7);
	__HAL_TIM_SET_COUNTER(&htim7, 0);
}

void LCD_Reset(void){
	init_SPI(); //Inicializar SPI
	init_GPIO(); //Iniciar GPIO
	//Iniciar los pines en valor alto
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET); //Reset
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,	GPIO_PIN_SET); //CS
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); //A0
	
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	delay(10);
	//HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
}
void LCD_wr_data(unsigned char data)
{
 // Seleccionar CS = 0;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,	GPIO_PIN_RESET); //CS

 // Seleccionar A0 = 1;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET); //A0
	
 // Escribir un dato (data) usando la función SPIDrv->Send(…);
	SPIdrv ->Send(&data, sizeof(data));
 // Esperar a que se libere el bus SPI;
	while(SPIdrv->GetStatus().busy == 1){
		
	}
 // Seleccionar CS = 1;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,	GPIO_PIN_SET); //CS
}

void LCD_wr_cmd(unsigned char cmd)
{
 // Seleccionar CS = 0;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,	GPIO_PIN_RESET); //CS
 // Seleccionar A0 = 0;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET); //A0

 // Escribir un comando (cmd) usando la función SPIDrv->Send(…);
	SPIdrv ->Send(&cmd, sizeof(cmd));
 // Esperar a que se libere el bus SPI;
	while(SPIdrv->GetStatus().busy == 1){
		
	}
 // Seleccionar CS = 1;
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11,	GPIO_PIN_SET); //CS
}

void LCD_Init(void){
	LCD_wr_cmd(0xAE);
	LCD_wr_cmd(0xA2);
	LCD_wr_cmd(0xA0);
	LCD_wr_cmd(0xC8);
	LCD_wr_cmd(0x22);
	LCD_wr_cmd(0x2F);
	LCD_wr_cmd(0x40);
	LCD_wr_cmd(0xAF);
	LCD_wr_cmd(0x81);
	LCD_wr_cmd(0x0B);
	LCD_wr_cmd(0xA4);
	LCD_wr_cmd(0xA6);
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
	
	LCD_Reset();
	LCD_Init();

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
