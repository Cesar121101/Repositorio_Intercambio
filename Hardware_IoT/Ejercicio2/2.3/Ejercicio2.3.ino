#include "hardware_D1.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

//const char *ssid  = "MiFibra-C776";
//const char *password = "auCpN7KN";
const char *ssid  = "Cesarin";
const char *password = "cesar121101";
int tiempo_actual = 0;
int tiempo_anterior = 0;
int rafaga = 0;
bool encender_buzzer = false;
String numero_cadena;
String status_buzzer = "encendido";


void setup() {
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Iniciando conexion WiFi");

  //Esperar a conectarse a la red
  while(WiFi.status() != WL_CONNECTED){
    digitalWrite(led_g,1);
    delay(250);
    digitalWrite(led_g,0);
    delay(250);
    Serial.print(".");
  }
  encender_buzzer = true;
  Serial.println("");
  Serial.print("Conectado a: ");
  Serial.println(ssid);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Serial.print("Tiempo actual: ");
  Serial.println(tiempo_actual);
  Serial.print("Tiempo anterior: ");
  Serial.println(tiempo_anterior);
  tiempo_actual = millis();
  //Encender buzzer
  if(encender_buzzer){
    tone(buzzer,1000);
    encender_buzzer = false;
    status_buzzer = "encendido"; 
    tiempo_anterior = tiempo_actual;
  }
  if(tiempo_actual-tiempo_anterior >= 1000 and status_buzzer == "encendido"){
    noTone(buzzer);
    status_buzzer = "apagado";
    tiempo_anterior = tiempo_actual;
    rafaga += 1;
  }
  if(tiempo_actual-tiempo_anterior >= 2000 and status_buzzer == "apagado" and rafaga < 3){
    tone(buzzer,1000);
    status_buzzer = "encendido";
    tiempo_anterior = tiempo_actual;
  }
  //Si estamos conectados a la red encendemos el led verde
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(led_g,1);
  }else{
    digitalWrite(led_g,0);
  }
}
