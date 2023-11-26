//Diagonales pagina 1

void diagonales(){
	uint8_t contador = 0;
	LCD_wr_cmd(abajo);
	LCD_wr_cmd(arriba);
	LCD_wr_cmd(0xB0); // Página 0
 	while(contador < 16){
		LCD_wr_data(0x01);
		osDelay(200);
		LCD_wr_data(0x02);
		osDelay(200);
		LCD_wr_data(0x04);
		osDelay(200);
		LCD_wr_data(0x08);
		osDelay(200);
		LCD_wr_data(0x10);
		osDelay(200);
		LCD_wr_data(0x20);
		osDelay(200);
		LCD_wr_data(0x40);
		osDelay(200);
		LCD_wr_data(0x80);
		osDelay(200);
		contador++;
	}
}
