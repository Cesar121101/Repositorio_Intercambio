#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "hardware_D1.h"

// Replace with your network credentials
//const char *ssid  = "DIGIFIBRA-hSbN";
//const char *password = "StXtC4k6KPdK";
const char *ssid  = "Cesarin";
const char *password = "cesar121101";

const char *ntpServerName = "pool.ntp.org";

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, ntpServerName, 1);

int medicion_luz = 0;
int tiempo_actual = 0;
int tiempo_anterior = 0;
int tiempo_flash = 0;
int tiempo_flashA = 0;
int medicion_umbral = 0; 
int contador_mediciones = 0;
int suma_mediciones = 0;
int media = 0;
String numero_cadena;
bool sobrescribir = true;
bool alertaUmbral = true;
String estado_alarma = "OFF";
String encender_alarma = "No";
char index_html[5000];

void actualizarMedicionLuz() {
  medicion_luz = analogRead(sensor_luz);
  delay(500);
  suma_mediciones += medicion_luz;
  contador_mediciones ++;
  if (medicion_luz < medicion_umbral) {
    digitalWrite(led_r, LOW);   // Apagar el LED
    estado_alarma = "OFF";
  }
  Serial.println(medicion_luz);
}

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;

// Initialize WiFi
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        digitalWrite(led_g,1);
        delay(250);
        digitalWrite(led_g,0);
        delay(250);
    }
    
    Serial.println("Wifi connected!");
    delay(1000);
    Serial.println(WiFi.localIP());
    if (WiFi.status() == WL_CONNECTED){
      digitalWrite(led_g,1);
       // Inicializa el cliente NTP
      timeClient.begin();
      timeClient.setTimeOffset(7200);
      timeClient.update();
    
      if(!SPIFFS.begin()){
        Serial.println("Error");
        return;
      }
      tiempo_actual = millis();
      tiempo_flash = millis();
      tiempo_anterior = tiempo_actual;
      tiempo_flashA = tiempo_flash;
    }
    else{
      digitalWrite(led_g,0);
    }
}

String processor(const String& var){
  if(var == "Luz"){
    return String(medicion_luz);
  }
  else if (var == "Alarma"){
    return String(estado_alarma);
  }
  else if(var == "Umbral"){
    return String(medicion_umbral);
  }
  return String();
}

void cargarWeb(){
  File file = SPIFFS.open("/paginaWeb.html","r");
  if(!file){
    Serial.println("No se puede abrir");
    return;
  }
  while(file.available()){
    String contenidoArchivo = file.readString(); // Lee el contenido del archivo en un objeto String
    char contenidoChar[contenidoArchivo.length() + 1]; // Crea un buffer para almacenar el contenido como un array de caracteres
    contenidoArchivo.toCharArray(contenidoChar, contenidoArchivo.length() + 1); // Convierte el String a un array de caracteres
    strcat(index_html, contenidoChar);
    //index_html += file.readString();
  }
  file.close();
}

void setup() {
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor_luz, INPUT);
  Serial.begin(9600);
  initWiFi();
  cargarWeb();

  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/Num", HTTP_POST, [](AsyncWebServerRequest *request){
    String numero_cadena;
    if(request->hasParam("numero", true)){
      numero_cadena = request->getParam("numero", true)->value();
      medicion_umbral = numero_cadena.toInt();
      Serial.println(medicion_umbral);
    }
    request->redirect("/");
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/Alarma", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.print(encender_alarma);
    if(medicion_luz > medicion_umbral){
      estado_alarma = "ON";
      digitalWrite(led_r, 1);
    }
    request->redirect("/");
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected!");
    }
    client->send("hello!", NULL, millis(), 1);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    tiempo_flash = millis();
    events.send("ping",NULL,millis());
    events.send(String(medicion_luz).c_str(),"luz_actual",millis()); //valores del sensor de luz
    events.send(String(estado_alarma).c_str(),"estado_alarma",millis()); // ON o OFF
    actualizarMedicionLuz();
    lastTime = millis();
  }
}
