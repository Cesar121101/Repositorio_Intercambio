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

void actualizarMedicionLuz() {
  medicion_luz = analogRead(sensor_luz);
  delay(500);
  suma_mediciones += medicion_luz;
  contador_mediciones ++;
  if (medicion_luz > medicion_umbral) {
    digitalWrite(led_r, HIGH);  // Encender el LED
    estado_alarma = "ON";
    
} else {
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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p { font-size: 1.2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #FF9A84; color: black; font-size: 1rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 800px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); }
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>Servidor ESP8266 Hardware para IoT </h1>
  </div>
  <div class="content">    
    <div class="cards">
      <div class="card">
        <p><i class="fas light" style="color:#059e8a;"></i>Luz</p><p><span class="reading"><span id="luz">%Luz%</span> </span></p>
      </div>

      <div class="card">
        <p><i class="fas light" style="color:#059e8a;"></i>Alarma</p><p><span class="reading"><span id="alarma">%Alarma%</span> </span></p>
      </div>

     <div class="card">
      <p><i class="fas light" style="color:#00add6;"></i>Umbral</p><p><span class="reading"><span id="umbral">%Umbral%</span> </span></p>
      <form action="Num\" method="post">
      <h4 style="text-align: center;"><input id="numero" name="numero" required="" type="number" placeholder="Umbral de luminosidad" /></h4>
      <h4 style="text-align: center;"><input type="submit" value="Enviar" /></h4>
      </form>
    </div>   
    </div>
  </div>
<script>
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('luz_actual', function(e) {
  console.log("luz_actual", e.data);
  document.getElementById("luz").innerHTML = e.data;
 }, false);

  source.addEventListener('estado_alarma', function(e) {
    console.log("estado_alarma", e.data);
    document.getElementById("alarma").innerHTML = e.data;
 }, false);

  source.addEventListener('umbral_actual', function(e) {
    console.log("umbral_actual", e.data);
    document.getElementById("umbral").innerHTML = e.data;
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup() {
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sensor_luz, INPUT);
  Serial.begin(9600);
  initWiFi();

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
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/Alarma", HTTP_POST, [](AsyncWebServerRequest *request){
    String string_cadena;
    if(request->hasParam("alarm", true)){
      string_cadena = request->getParam("alarm", true)->value();
      encender_alarma = string_cadena;
      Serial.print(encender_alarma);
    }
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
    //events.send(String(medicion_umbral).c_str(),"umbral_actual",millis()); //umbral de input en la pagina
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
