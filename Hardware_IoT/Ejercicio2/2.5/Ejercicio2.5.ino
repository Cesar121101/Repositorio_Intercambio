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
  }else { 
    digitalWrite(led_r, HIGH);   // Apagar el LED
    estado_alarma = "ON";
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
  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected!");
    }
    client->send("hello!", NULL, millis(), 1000);
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
    
    timeClient.update();
    // Obtiene fecha y hora
    String hora = timeClient.getFormattedTime();
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime); 
    int dia = ptm->tm_mday;
    int mes = ptm->tm_mon+1;
    int year = ptm->tm_year+1900;
    String fecha = (String(dia)+"/"+String(mes)+"/"+String(year));
    
    actualizarMedicionLuz();

    if(tiempo_flash - tiempo_flashA >= 60000){
      Serial.println("Guardar Mediciones");
      media = suma_mediciones/contador_mediciones;
      Serial.print("Media: ");
      Serial.println(media);
      if(media > medicion_umbral){
        escribirFichero(String(media), fecha, hora, "arriba");
      }else{
        escribirFichero(String(media), fecha, hora, "abajo");
      }
      tiempo_flashA = tiempo_flash;
      leerFichero();
      suma_mediciones = 0;
      contador_mediciones = 0;
    }
    lastTime = millis();
  }
}
