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

//Desplazar izquierda y derecha un numero finito de veces (contador = numero de veces a desplazar y valor es una flag)
void desplazar(){
	if(contador < 97 && valor == 0){
		for(int i = 0; i < 512; i++){
			buffer2[i] = buffer[i];
		}
		for(int i = 0; i < 128; i++){
			if(i == 0){
				buffer[i] = buffer2[i+127];
				buffer[i+128] = buffer2[i+255];
			}
			else{
				buffer[i] = buffer2[i-1];
				buffer[i+128] = buffer2[i+127];
			}
		}
		contador++;
	}else{
		valor = 1;
		for(int i = 0; i < 512; i++){
				buffer2[i] = buffer[i];
		}
		for(int i = 0; i < 128; i++){
			if(i == 127){
				buffer[i] = buffer2[i-127];
				buffer[i+128] = buffer2[i+1];
			}
			else{
				buffer[i] = buffer2[i+1];
				buffer[i+128] = buffer2[i+129];
			}
		}
		contador--;
		if(contador == 9){
			valor = 0;
		}
	}
	LCD_update();
}

