 cv//Incluir definiciones de sensores, leds y botones
#include "hardware.h"

//Umbral de luminosidad
int umbral_min = 5;
int umbral_max = 980;

//Variables globales
int medicion_luz = 0;
int tiempo_anterior = 0;
int tiempo_actual = 0;
int tiempo_exposicion_actual = 0;
int tiempo_exposicion_anterior = 0;
int tiempo_verde = 0; 
int tiempo_verde_anterior = 0;
int estado = 0;
int estado_anterior = 0;
int umbral_luminosidad = 800;
bool primer_verde = true;
bool status_buzzer = false;  
bool primera_exposicion = true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(r, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(x, INPUT);
  pinMode(led_rojo, OUTPUT);
  pinMode(y, INPUT);
  pinMode(z, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor_luz, INPUT);
  pinMode(boton, INPUT_PULLUP);
  pinMode(switch_, INPUT);
}

void loop() {
  // Valor minimo del sensor 5, maximo 980
  //Medicion del tiempo
  tiempo_actual = millis();
  //Si han transcurrido 400 ms
  if (tiempo_actual - tiempo_anterior >= 400){
    //Medicion del sensor de luz
    medicion_luz = analogRead(sensor_luz);
    Serial.print("Medicion del sensor: ");
    Serial.println(medicion_luz);
    //Condicion para encender leds
    if (medicion_luz <= umbral_min) {
      digitalWrite(led0, LOW);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
      digitalWrite(led5, LOW);
    }
    else if(medicion_luz <= (umbral_max-umbral_min)/6){
      digitalWrite(led0, HIGH);
      digitalWrite(led1, LOW);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
      digitalWrite(led5, LOW);
    }
    else if(medicion_luz <= ((umbral_max-umbral_min)/6 * 2)){
      digitalWrite(led0, HIGH);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, LOW);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
      digitalWrite(led5, LOW);
    }
    else if(medicion_luz <= ((umbral_max-umbral_min)/6) * 3){
      digitalWrite(led0, HIGH);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, LOW);
      digitalWrite(led4, LOW);
      digitalWrite(led5, LOW);
    }
    else if(medicion_luz <= ((umbral_max-umbral_min)/6) * 4){
      digitalWrite(led0, HIGH);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
      digitalWrite(led4, LOW);
      digitalWrite(led5, LOW);
    }
    else if(medicion_luz <= ((umbral_max-umbral_min)/6) * 5){
      digitalWrite(led0, HIGH);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
      digitalWrite(led4, HIGH);
      digitalWrite(led5, LOW);
    }
    else {
      digitalWrite(led0, HIGH);
      digitalWrite(led1, HIGH);
      digitalWrite(led2, HIGH);
      digitalWrite(led3, HIGH);
      digitalWrite(led4, HIGH);
      digitalWrite(led5, HIGH);
    }
    //Condicion de umbral de luminosidad excesiva
    if (medicion_luz >= umbral_luminosidad) {
      if(primera_exposicion == true){
        tiempo_exposicion_actual = millis();
        tiempo_exposicion_anterior = tiempo_exposicion_actual;
        primera_exposicion = false;
      }else{
        tiempo_exposicion_actual = millis();
      }
      //Condicion para encender luz verde
      if(tiempo_exposicion_actual - tiempo_exposicion_anterior >= 10000){
        //Verificar que es la primera vez que hemos encendido el led verde
        if(primer_verde == true){
          tiempo_verde = millis();
          tiempo_verde_anterior = tiempo_verde;
          digitalWrite(g, HIGH);
          primer_verde = false;
        }else{
          tiempo_verde = millis();
        }
        //Apagar el led si han pasado 500 ms
        if(tiempo_verde - tiempo_verde_anterior >= 500 and (digitalRead(g) == HIGH)){
          digitalWrite(g, LOW);
          tiempo_verde_anterior = tiempo_verde;
        }
        //Encender de nuevo el led si pasaron 500 mss
        else if(tiempo_verde - tiempo_verde_anterior >= 500  and (digitalRead(g) == LOW)){
          digitalWrite(g, HIGH);
          tiempo_verde_anterior = tiempo_verde;
        }
      }
      //Condicion para hacer sonar la alarma 
      if(tiempo_exposicion_actual - tiempo_exposicion_anterior >= 20000){
        tone(buzzer, 1000);
        status_buzzer = true; 
      }
    }else{
      primer_verde = true;
      primera_exposicion = true;
      digitalWrite(g, LOW);
    }
    //Tiempo global para llevar control de los 400 ms
    tiempo_anterior = tiempo_actual;
    Serial.print("Tiempo Exposicion anterior: ");
    Serial.println(tiempo_exposicion_anterior);
    Serial.print("Tiempo Exposicion sactual: ");
    Serial.println(tiempo_exposicion_actual);
  }
  //Si pulsamos el boton, apagamos el buzzer, apagamos el led verde y detenemos el sistema 30 segundos
  if(digitalRead(boton) == 0 and status_buzzer == true){
    noTone(buzzer);
    digitalWrite(g, LOW);
    digitalWrite(led0, LOW);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(led5, LOW);
    primer_verde = true;
    primera_exposicion = true;
    status_buzzer = false;
    //Detener el tiempo 30 segundos
    delay(30000); 
  }
}
