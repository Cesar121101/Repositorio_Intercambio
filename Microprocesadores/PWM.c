TIM_HandleTypeDef htim1;
TIM_OC_InitTypeDef TIM_Channel_InitStruct;

void init_Timer1(void){
	
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
