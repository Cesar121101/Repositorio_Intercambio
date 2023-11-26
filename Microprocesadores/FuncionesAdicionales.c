//Diagonales pagina 1
void diagonales(){
	//Fuera del while
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
	//Fuera del while 
	posicionL1 = 9; //Posicion inicial de la letra
	symbolToLocalBuffer(1,'@');
	LCD_update();
	if(contador < 97 && valor == 0){
		desplazarDerecha();
		contador++;
	}else{
		valor = 1;
		desplazarIzquierda();
		contador--;
		if(contador == 9){
			valor = 0;
		}
	}
	LCD_update();
}

void obtenerPosicion(int p){ //Cambia de posicion entera a la posicion del LCD pagina 0
	uint8_t real = posicion;
	uint8_t abajo = real & 0x0F;
	uint8_t arriba = real & 0xF0;
	arriba = arriba >> 4;
	arriba = arriba | 0x10;
	LCD_wr_cmd(abajo);
	LCD_wr_cmd(arriba);
	LCD_wr_cmd(0xB0); // Página 0
}
