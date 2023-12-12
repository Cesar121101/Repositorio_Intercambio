//---------------------------------------------------
// ÚNICAS VARIABLES DISPONIBLES PARA UTILIZAR:
int y0; //Salida del controlador en el instante k
int y1=0; //Salida del controlador en el instante k-1
int y2=0; //Salida del controlador en el instante k-2
int y3=0; //Salida del controlador en el instante k-3
int y4=0; //Salida del controlador en el instante k-4

int x0; //Entrada al controlador en el instante k
int x1=0; //Entrada al controlador en el instante k-1
int x2=0; //Entrada al controlador en el instante k-2
int x3=0; //Entrada al controlador en el instante k-3

int e;
//---------------------------------------------------


//---------------------------------------------------
//CÓDIGO EJECUTABLE: 
while (1==1) {
	//Error del sistema: 
	e=entrada-salida; //entrada y salida son variables globales

	/////////////////////////
	//INICIO CÓDIGO DE ALUMNO
	x0 = e;
	y0 = (8.3474*x0) - (7.7542*x1) + (0.998156*y1);
	x1 = x0;
	y1 = y0;
	//FIN CÓDIGO DE ALUMNO
	/////////////////////////
	
	//Retardo de ejecución del bucle:
	delay(T); //T es una variable global
}
//---------------------------------------------------
