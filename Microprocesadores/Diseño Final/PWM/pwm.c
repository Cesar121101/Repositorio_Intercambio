#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
#include "pwm.h"
 
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t pwm;                        // thread id
osMessageQueueId_t queue_pwm;

TIM_HandleTypeDef htim1;
TIM_OC_InitTypeDef TIM_Channel_InitStruct;

uint16_t dutyCycle;

void ThPWM (void *argument);                   // thread function

static void init_GPIO(void){
	GPIO_InitTypeDef GPIO_InitStruct; //Definicion los GPIO
		__HAL_RCC_GPIOE_CLK_ENABLE();	 //Habilitar el reloj de los puertos GPIO E
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; //Habilitar el modo funcion alternativa
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM1; //Definir el modo alternativo del pin
	GPIO_InitStruct.Pin = GPIO_PIN_9; //Definir al pin 9
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct); //Inicializar el pin 9
}

static void init_Timer1(void){
	
	__HAL_RCC_TIM1_CLK_ENABLE(); //Habilitar reloj del timer 1
	
	htim1.Instance = TIM1; 
	htim1.Init.Prescaler = 839; //Prescaler a 840, El reloj de APB1 es de 84 MHz / Prescaler (840) = 100 kHz
	htim1.Init.Period = 199; //Frecuencia de conteo = 100 kHz/Frec deseada/2 = 500Hz up y 500Hz down = 1kHz = Frecuencia de interrupcion = ARR = 200 - 1
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&htim1);
	
	TIM_Channel_InitStruct.OCMode = TIM_OCMODE_PWM1; //Definimos el modo PWM1
	TIM_Channel_InitStruct.Pulse = dutyCycle;
	TIM_Channel_InitStruct.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_Channel_InitStruct.OCFastMode = TIM_OCFAST_DISABLE;
	
	HAL_TIM_PWM_ConfigChannel(&htim1, &TIM_Channel_InitStruct, TIM_CHANNEL_1); //Configurar el canal

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); //Iniciar el timer
}

int init_PWM (void) {
  pwm = osThreadNew(ThPWM, NULL, NULL);
  if (pwm == NULL) {
    return(-1);
  }
	queue_pwm = osMessageQueueNew(8, sizeof(msgQueue_PWM), NULL);
	init_GPIO();
  return(0);
}

void ThPWM (void *argument) {
	msgQueue_PWM msg;
	float temperaturaAnterior = 0;
	float y;
  while (1) {
		osMessageQueueGet(queue_pwm, &msg, NULL, osWaitForever);
		if(msg.Tm > 0){	
			y = msg.Tr - msg.Tm;
			if(y != temperaturaAnterior){
				if (y > 5.0) {
					// Rojo
					dutyCycle = 200; //100%
				} 
				else if (y >= 0.0 && y <= 5.0) {
					dutyCycle = 160; //80%
				} 
				else if (y >= -5.0 && y < 0.0) {
					dutyCycle = 80; // 40%
				} 
				else if (y < -5.0) {
					dutyCycle = 0; 
				}
				init_Timer1();
			}
			temperaturaAnterior = y;
		}else{
			dutyCycle = 0;
			init_Timer1();
		}
	}
}