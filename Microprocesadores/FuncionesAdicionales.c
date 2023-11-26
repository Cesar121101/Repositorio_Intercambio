//Diagonales pagina 1
void diagonales(){
	//Fuera del while
	uint8_t contador = 0;
	LCD_wr_cmd(abajo);
	LCD_wr_cmd(arriba);
	LCD_wr_cmd(0xB0); // Página 0
 	if(contador < 16){
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

void obtenerPosicion(int p){ //Obtiene la posicion del LCD deacuerdo a la posicion que pasemos
	uint8_t real = posicion;
	uint8_t abajo = real & 0x0F;
	uint8_t arriba = real & 0xF0;
	arriba = arriba >> 4;
	arriba = arriba | 0x10;
	LCD_wr_cmd(abajo);
	LCD_wr_cmd(arriba);
	LCD_wr_cmd(0xB0); // Página 0
}


void pintaPelota(int posicion){
	uint8_t real, arriba, abajo; //Declarar fuera de la funcion
	LCD_clear();
	uint8_t real = posicion;
	uint8_t abajo = real & 0x0F;
	uint8_t arriba = real & 0xF0;
	arriba = arriba >> 4;
	arriba = arriba | 0x10;
	LCD_wr_cmd(abajo);
	LCD_wr_cmd(arriba);
	LCD_wr_cmd(0xB0); // Página 0
	for(int i = 0; i < 8; i++){
		LCD_wr_data(0xFF);
	}
	LCD_wr_cmd(abajo);
	LCD_wr_cmd(arriba);
	LCD_wr_cmd(0xB1); // Página 1
	for(int i = 0; i < 8; i++){
		LCD_wr_data(0x0F);
	}
}

void dibujarRectangulo(){
	uint8_t arriba = 0x10,abajo = 0x00;
	uint32_t status;
	while (1) {
		status=osThreadFlagsWait(0x3,osFlagsWaitAny,osWaitForever); //Esperar Banderas
		LCD_clear();
		switch (status){
			case 1:
				contador++;
				derecha++;
				break;
			case 2:
				contador--;
				izquierda++;
				break;
		}
		LCD_wr_cmd(abajo);
		LCD_wr_cmd(arriba);
		LCD_wr_cmd(0xB0); // Página 0
		for(int i = 0; i < contador; i++){
			LCD_wr_data(0xFF);
		}
		LCD_wr_cmd(abajo);
		LCD_wr_cmd(arriba);
		LCD_wr_cmd(0xB1); // Página 0
		for(int i = 0; i < contador; i++){
			LCD_wr_data(0xFF);
		}
}

void cuadricula8x8(){ //Dibuja la cuadricula de 8x8
	uint8_t arriba = 0x10,abajo = 0x00;
	uint32_t status;
	uint8_t valor, contador = 0, contador2 = 0, bandera = 1;
	uint8_t pagina1, pagina2;
	while (1) {
	if(contador < 16){
		if(contador %2 == 0){
			pagina1 = 0xB0;
			pagina2 = 0xB2;
		}else{
			pagina1 = 0xB1;
			pagina2 = 0xB3;
		}
		LCD_wr_cmd(abajo);
		LCD_wr_cmd(arriba);
		LCD_wr_cmd(pagina1); // Página 0
		for(int i = 0; i < 8; i++){
			LCD_wr_data(0xFF);
		}
		LCD_wr_cmd(abajo);
		LCD_wr_cmd(arriba);
		LCD_wr_cmd(pagina2); // Página 0
		for(int i = 0; i < 8; i++){
			LCD_wr_data(0xFF);
		}
		if(bandera == 0){
			abajo = 0x00;
			bandera = 1;
		}else{
			abajo = 0x08;
			bandera = 0;
		}
		contador2++;
		if(contador2 == 2){
			contador2 = 0;
			arriba++;
		}
		contador++;
		osDelay(500);
	}
}

void lineahorizontal(){
	uint8_t arriba = 0x10,abajo = 0x00;
	uint32_t status;
	uint8_t valor, contador = 0, data = 0x01;
	while (1) {
		if(contador < 8){
			LCD_clear();
			LCD_wr_cmd(abajo);
			LCD_wr_cmd(arriba);
			LCD_wr_cmd(0xB0); // Página 0
			for(int i = 0; i < 128; i++){
				LCD_wr_data(data);
			}
			data = data << 1;
			contador++;
			if(contador == 8){
				data = 0x01;
			}
		}else if(contador < 16){
			LCD_clear();
			LCD_wr_cmd(abajo);
			LCD_wr_cmd(arriba);
			LCD_wr_cmd(0xB1); // Página 0
			for(int i = 0; i < 128; i++){
				LCD_wr_data(data);
			}
			data = data << 1;
			contador++;
			if(contador == 16){
				data = 0x01;
			}
		}else if(contador < 24){
			LCD_clear();
			LCD_wr_cmd(abajo);
			LCD_wr_cmd(arriba);
			LCD_wr_cmd(0xB2); // Página 0
			for(int i = 0; i < 128; i++){
				LCD_wr_data(data);
			}
			data = data << 1;
			contador++;
			if(contador == 24){
				data = 0x01;
			}
		}else if(contador < 32){
			LCD_clear();
			LCD_wr_cmd(abajo);
			LCD_wr_cmd(arriba);
			LCD_wr_cmd(0xB3); // Página 0
			for(int i = 0; i < 128; i++){
				LCD_wr_data(data);
			}
			data = data << 1;
			contador++;
			if(contador == 32){
				contador = 0;
				data = 0x01;
			}
		}
		osDelay(200);
	}
}
