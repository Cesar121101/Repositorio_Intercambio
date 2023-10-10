#include "hardware_D1.h"
#include "FS.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char *ssid  = "MiFibra-C776";
const char *password = "auCpN7KN";
//const char *ssid  = "DIGIFIBRA-hSbN";
//const char *password = "StXtC4k6KPdK";
//const char *ssid  = "Cesarin";
//const char *password = "cesar121101";

const char *ntpServerName = "pool.ntp.org";

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, ntpServerName, 1);

int medicion_luz = 0;
int tiempo_actual = 0;
int tiempo_anterior = 0;
int tiempo_flash = 0;
int tiempo_flashA = 0;
int umbral; 
int contador_mediciones = 0;
int suma_mediciones = 0;
int media = 0;
String numero_cadena;
bool sobrescribir = true;
bool alertaUmbral = true;

ESP8266WebServer server(80);

String webPage = "";

void paginaNoEncontrada(){
  String message = "File not Found \n\n";
  message += "URI";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for(uint8_t i = 0; i < server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send (404, "text/plain", message);
}

void setup() {
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor_luz, INPUT);
  Serial.begin(9600);
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
  //Si estamos conectados a la red encendemos el led verde
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(led_g,1);
  }else{
    digitalWrite(led_g,0);
  }
  
  Serial.println("");
  Serial.print("Conectado a: ");
  Serial.println(ssid);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
  
  // Inicializa el cliente NTP
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  timeClient.update();

  if(!SPIFFS.begin()){
    Serial.println("Error");
    return;
  }
  cargarWeb();
  
  server.on("/", [](){
    server.send(200,"text/html", webPage); 
  });
  server.on("/On", [](){
    Serial.println("Encender alarma");
    medicion_luz = analogRead(sensor_luz);
    if(medicion_luz > umbral){
      digitalWrite(led_r, 1);
    }
    server.send(200,"text/html", webPage); 
  });
  server.on("/Off", [](){
    Serial.println("Apagar led");
    digitalWrite(led_r, 0);
    server.send(200,"text/html", webPage); 
  });
  server.on("/Num", [](){
    numero_cadena = server.arg("numero"); // Obtener el dato enviado desde la página web
    Serial.println("Dato recibido desde la página web: " + numero_cadena);
    umbral = numero_cadena.toInt();
    //Convertir el dato que recibimos a entero numero.toInt()
    server.send(200,"text/html", webPage); 
  });
  server.on("/Luz",[](){
    Serial.println("Enviar luminosidad");
    medicion_luz = analogRead(sensor_luz);
    server.send(200,"text/html", webPage + "<h3 style=\"text-align: center;\">" + String(medicion_luz) + "</h3>"); 
  }); 
  server.onNotFound(paginaNoEncontrada);
  server.begin();
  Serial.println("HTPP server started"); 

  tiempo_actual = millis();
  tiempo_flash = millis();
  tiempo_anterior = tiempo_actual;
  tiempo_flashA = tiempo_flash;
}

void cargarWeb(){
  File file = SPIFFS.open("/paginaWeb.html","r");
  if(!file){
    Serial.println("No se puede abrir");
    return;
  }
  while(file.available()){
    webPage += file.readString();
  }
  file.close();
}

void leerFichero(){
  File file = SPIFFS.open("/mediciones.txt", "r");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void escribirFichero(String media, String fecha, String hora, String alertaUmbral){
  if(sobrescribir){
    //Abrir archivo para escribir desde 0
    File file = SPIFFS.open("/mediciones.txt", "w");
    file.println("M:"+ media + " F:" + fecha + " H:" + hora + " U:" + alertaUmbral);
    file.close();
    sobrescribir = false;
  }else{
    //Abrir archivo para escribir desde la ultima linea
    File file = SPIFFS.open("/mediciones.txt", "a");
    file.println("M:"+ media + " F:" + fecha + " H:" + hora  + " U:" + alertaUmbral);
    file.close();
  }
}

void loop() {
  tiempo_actual = millis();
  tiempo_flash = millis();
  
  // Obtiene la hora actual del servidor NTP
  timeClient.update();

  // Obtiene fecha y hora
  String hora = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int dia = ptm->tm_mday;
  int mes = ptm->tm_mon+1;
  int year = ptm->tm_year+1900;
  String fecha = (String(dia)+"/"+String(mes)+"/"+String(year));
     
  // Medicion de luz  
  if(tiempo_actual - tiempo_anterior >= 1000){
    Serial.println("Medicion sensor");
    medicion_luz = analogRead(sensor_luz);
    suma_mediciones += medicion_luz;
    contador_mediciones ++;
    tiempo_anterior = tiempo_actual;
  }

  if(tiempo_flash - tiempo_flashA >= 3000){
    Serial.println("Guardar Mediciones");
    media = suma_mediciones/contador_mediciones;
    Serial.print("Media: ");
    Serial.println(media);
    if(media > umbral){
      escribirFichero(String(media), fecha, hora, "arriba");
    }else{
      escribirFichero(String(media), fecha, hora, "abajo");
    }
    tiempo_flashA = tiempo_flash;
    leerFichero();
    suma_mediciones = 0;
    contador_mediciones = 0;
  }
  server.handleClient();
}
