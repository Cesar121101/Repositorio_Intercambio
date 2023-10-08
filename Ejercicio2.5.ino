#include "hardware_D1.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//const char *ssid  = "MiFibra-C776";
//const char *password = "auCpN7KN";
const char *ssid  = "DIGIFIBRA-hSbN";
const char *password = "StXtC4k6KPdK";
//const char *ssid  = "Cesarin";
//const char *password = "cesar121101";

const char *ntpServerName = "pool.ntp.org";

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, ntpServerName, 2);

int medicion_luz = 0;
int tiempo_actual = 0;
int tiempo_anterior = 0;
int umbral; 
bool primer_buzzer = true;
bool enviarLuminosidad = false;
String numero_cadena;

ESP8266WebServer server(80);

String webPage;

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
    primer_buzzer = false;
    digitalWrite(led_g,0);
  }
  
  Serial.println("");
  Serial.print("Conectado a: ");
  Serial.println(ssid);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Inicializa el cliente NTP
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  timeClient.update();

  webPage = "<h1 style=\"text-align: center;\">&nbsp;<span style=\"color: #ff0000;\"><strong>Bienvenidos al Servidor</strong></span></h1>";
  webPage += "<h4 style=\"text-align: center;\"><a href=\"On\">Encender alarma</a></h4><p>&nbsp;</p>";
  webPage += "<h4 style=\"text-align: center;\"><a href=\"Off\">Apagar alarma</a></h4><br>";
  webPage += "<form action=\"Num\" method=\"post\"><h4 style=\"text-align: center;\"><input id=\"numero\" name=\"numero\" required=\"\" type=\"number\"";
  webPage += "placeholder=\"Umbral de luminosidad\" /> </h4>";
  webPage += "<h4 style=\"text-align: center;\"><input type=\"submit\" value=\"Enviar\" /></h4></form>";
  webPage += "<form action=\"Luz\" method=\"post\"><h4 style=\"text-align: center;\"><input type=\"submit\" value=\"Luz Actual\" /></h4></form>";
  webPage += "<h3 style=\"text-align: center;\">Luminosida Actual:</h3>";
  
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
}

void loop() {
  // Obtiene la hora actual del servidor NTP
  timeClient.update();

  // Obtiene la hora, minutos y segundos
  int hours = timeClient.getHours();
  int minutes = timeClient.getMinutes();
  int seconds = timeClient.getSeconds();

  // Muestra la hora en el puerto serie
  Serial.print("Hora actual: ");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(seconds);
  server.handleClient();
}
