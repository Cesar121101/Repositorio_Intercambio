#include "lcd.h"
#include "Driver_SPI.h"
#include "Arial12x12.h"

extern ARM_DRIVER_SPI Driver_SPI1;
ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

unsigned char buffer[512];
TIM_HandleTypeDef htim7; //Definimos el TIM7
int posicionL1 = 0, posicionL2 = 256, posicionLCD = 0;;
int flagL2 = 0, flagF2 = 0, full = 0;

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
	delay(1);
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
	LCD_Reset();
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

void LCD_clear(void)
{
	int i;

	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	for(i=0;i<128;i++){
		LCD_wr_data(0x00);
	}

	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	for(i=0;i<128;i++){
		LCD_wr_data(0x00);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB2); //Página 2
	for(i=0;i<128;i++){
		LCD_wr_data(0x00);
	}

	LCD_wr_cmd(0x00);
	LCD_wr_cmd(0x10);
	LCD_wr_cmd(0xB3); // Pagina 3
	for(i=0;i<128;i++){
		LCD_wr_data(0x00);
	}
	full = 0;
}

void LCD_update(void)
{
	int i;
	int pagina1, pagina2, pagina3, pagina4;
	posicionLCD = posicionL2;
	pagina1 = 128;
	pagina2 = 256;
	pagina3 = posicionLCD;
	pagina4 = posicionLCD+128;

	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB0); // Página 0
	for(i = 0; i < pagina1; i++){
		LCD_wr_data(buffer[i]);
	}
	
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB1); // Página 1
	for(i = 128; i < pagina2; i++){
		LCD_wr_data(buffer[i]);
	}
	
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB2); // Página 2
	for(i = pagina2; i < pagina3; i++){
		LCD_wr_data(buffer[i]);
	}
	
	LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
	LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
	LCD_wr_cmd(0xB3); // Página 3
	for(i = 384; i < pagina4; i++){
		LCD_wr_data(buffer[i]);
	}
}

void symbolToLocalBuffer_L1(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	offset = 25* (symbol - ' ');
	for (i = 0; i < Arial12x12[offset]; i++){
		if(i + posicionLCD > 127){
				posicionLCD += i;
				return;
		}
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i + posicionL1] = value1;
		buffer[i+128 + posicionL1] = value2;
	}
	posicionL1 += Arial12x12[offset];
	posicionLCD += Arial12x12[offset];
}

void symbolToLocalBuffer_L2(uint8_t symbol){
	uint8_t i, value1, value2;
	uint16_t offset = 0;
	
	if(flagF2 == 0){
		posicionLCD = 256;
		flagF2 = 1;
	}
	
	offset = 25* (symbol - ' ');
	for (i = 0; i < Arial12x12[offset]; i++){
		if(i + posicionLCD > 383){
				posicionLCD += i;
				return;
		}
		value1 = Arial12x12[offset+i*2+1];
		value2 = Arial12x12[offset+i*2+2];
		
		buffer[i + posicionL2] = value1;
		buffer[i+128 + posicionL2] = value2;
	}
	posicionL2 += Arial12x12[offset];
	posicionLCD += Arial12x12[offset];
}

void symbolToLocalBuffer(uint8_t line, uint8_t symbol){
	if(full == 0){
		uint8_t i, value1, value2;
		uint16_t offset = 0;
		
		offset = 25* (symbol - ' ');
		if(line == 1){ //Cambiar a linea 1
			posicionLCD = posicionL1;
			flagL2 = 0;
		}
		if(line == 2){ //Cambiar a linea 2
			posicionLCD = posicionL2;
			flagL2 = 1;
		}
		if(posicionLCD > 383){ //Si llegamos al final de la linea 2 ya no escribiremos 2
			full = 1;
			return;
		}
		if(symbol == ' ' && posicionLCD == 256){ //Primer caracter de la linea 2 vacio
			return;
		} 
		for (i = 0; i < Arial12x12[offset]; i++){
			if(i + posicionLCD > 127 && flagL2 == 0){
				posicionLCD += i;
				return;
			}
			if(i + posicionLCD > 383){
				posicionLCD += i;
				return;
			}
			value1 = Arial12x12[offset+i*2+1];
			value2 = Arial12x12[offset+i*2+2];
			
			buffer[i + posicionLCD] = value1;
			buffer[i+128 + posicionLCD] = value2;
		}
		posicionLCD += Arial12x12[offset];
		if(line == 1){
			posicionL1 += Arial12x12[offset];
		}
		if(line == 2){
			posicionL2 += Arial12x12[offset];
		}
	}
}
