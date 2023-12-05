#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <esp_sleep.h>
#include <LiquidCrystal_I2C.h>
#include <Bounce2.h>
#include <DHT.h>
#include <SPIFFS.h>
#include "hardware.h"

RTC_DATA_ATTR int estatus = 0;
RTC_DATA_ATTR int estatusRele = 0;
RTC_DATA_ATTR float temperaturaDeseada = 20.0;
RTC_DATA_ATTR bool sobrescribir = true;

const char *ssid  = "Cesarin";
const char *password = "cesar121101";
const char *ntpServerName = "pool.ntp.org";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServerName, 1);

// Servidor asincronico en el puerto 80
AsyncWebServer server(80);

// Creación de eventos
AsyncEventSource events("/events");

Bounce botonA = Bounce(); //Objeto del boton arriba
Bounce botonD = Bounce(); //Objeto del boton abajo
DHT sensor_temperatura(sensor_t,DHT22); //Objeto del sensor de temperatura

//------------------------------------SIMBOLO °---------------------------------------------
LiquidCrystal_I2C lcd(0x27,16,2);
byte o[8] = {
B11100,
B10100,
B11100,
B00000,
B00000,
B00000,
B00000,
B00000,
};

//------------------------------------VARIABLES GLOBALES---------------------------------------------
float temperaturaActual = 0.0;
float humedadActual = 0.0;
int tiempoAnterior = 0; //Mediciones de tiempo
int tiempoServidor = 0; //Tiempo del servidor entre cada evento 
int tiempoFlash = 0;
char tempD[7]; //Arreglo de temeperatura deseada
char tempA[7]; //Arreglo de temperatura actual
char humedad[5]; //Arreglo de humedad
char index_html[5000]; //Arreglo de pagina web


//-------------------------------INICIAR WIFI------------------------------------------
void initWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
    }
    Serial.println("Wifi connected!");
    delay(500);
    Serial.println(WiFi.localIP());
     // Inicializa el cliente NTP
    timeClient.begin();
    timeClient.setTimeOffset(7200);
    timeClient.update();
    if(!SPIFFS.begin()){
      Serial.println("Error");
      return;
    }
}

//-------------------------------MANEJADOR DE EVENTOS DEL SERVIDOR------------------------------------------
String processor(const String& var){
  if(var == "TemperaturaA"){
    return String(temperaturaActual);
  }
  else if (var == "TemperaturaD"){
    return String(temperaturaDeseada);
  }
  else if(var == "Humedad"){
    return String(humedadActual);
  }
  return String();
}

//-------------------------------SERVIDOR FUNCIONANDO------------------------------------------
void initServidor(){
    if ((millis() - tiempoServidor) > 1000) {
      events.send("ping",NULL,millis());
      events.send(String(temperaturaActual).c_str(),"temperatura_actual",millis()); //valores del sensor de temperatura
      events.send(String(temperaturaDeseada).c_str(),"temperatura_deseada",millis()); //temperatura deseada por el usuario
      events.send(String(humedad).c_str(),"humedad",millis()); //temperatura deseada por el usuario
    }
}

//------------------------------------CARGAR SERVIDOR---------------------------------------------
void cargarServidor(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/Num", HTTP_POST, [](AsyncWebServerRequest *request){
    //Variable auxiliar temporal
    String TemperaturaDServer;
    if(request->hasParam("numero", true)){
      TemperaturaDServer = request->getParam("numero", true)->value();
      temperaturaDeseada = TemperaturaDServer.toFloat();
      tiempoAnterior = millis();
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

//-------------------------------CARGAR PAGINA WEB------------------------------------------
void cargarWeb(){
  File file = SPIFFS.open("/paginaWeb.html","r");
  if(!file){
    Serial.println("No se puede abrir");
    return;
  }
  while(file.available()){
    String contenidoArchivo = file.readString(); 
    char contenidoChar[contenidoArchivo.length() + 1];
    contenidoArchivo.toCharArray(contenidoChar, contenidoArchivo.length() + 1);
    strcat(index_html, contenidoChar);
  }
  file.close();
}

//------------------------------------INICIAR LCD---------------------------------------------
void initLCD(){
  lcd.init();
  lcd.backlight();
  lcd.createChar (0,o);
  lcd.clear();
  lcd.setCursor(0, 0);
  sprintf(tempA,"TA:%04.1f", temperaturaActual);
  lcd.print(tempA);
  lcd.write (byte (0));
  lcd.print("C");
  lcd.setCursor(11, 0);
  sprintf(humedad,"%04.1f%%", humedadActual);
  lcd.print(humedad);
  lcd.setCursor(0, 1);
  sprintf(tempD,"TD:%04.1f", temperaturaDeseada);
  lcd.print(tempD);
  lcd.write (byte (0));
  lcd.print("C");
}

//------------------------------------ACTUALIZAR LCD---------------------------------------------
void actualizarLCD(){
  lcd.setCursor(0, 0);
  sprintf(tempA,"TA:%04.1f", temperaturaActual);
  lcd.print(tempA);
  lcd.write (byte (0));
  lcd.print("C");
  lcd.setCursor(11, 0);
  sprintf(humedad,"%04.1f%%", humedadActual);
  lcd.print(humedad);
  lcd.setCursor(0, 1);
  sprintf(tempD,"TD:%04.1f", temperaturaDeseada);
  lcd.print(tempD);
  lcd.write (byte (0));
  lcd.print("C");
}

//------------------------------------PULSACIONES DE UN BOTON---------------------------------------------
void pulsaciones(){ 
  botonA.update();
  botonD.update();
  if(botonA.fell()){
    tiempoAnterior = millis();
    if(temperaturaDeseada < 99){
      temperaturaDeseada += 0.5;
    }
  }
  if(botonD.fell()){
    tiempoAnterior = millis();
    if(temperaturaDeseada > 0){
      temperaturaDeseada -= 0.5;
    }
  }
}

//------------------------------------ENCENDER/APAGAR RELE---------------------------------------------
void comprobarRele(){
  if(temperaturaActual < temperaturaDeseada and estatusRele == 0){
    estatusRele = 1;
    digitalWrite(rele_on, HIGH);
    digitalWrite(rele_off, LOW);
    delay(10);
    digitalWrite(rele_on, LOW);
    Serial.println("Rele encendido");
  }
  if(temperaturaActual >= temperaturaDeseada and estatusRele == 1){
    estatusRele = 0;
    digitalWrite(rele_off, HIGH);
    digitalWrite(rele_off, HIGH);
    digitalWrite(rele_on, LOW);
    delay(10);
    digitalWrite(rele_off, LOW);
    Serial.println("Rele Apagado");
  }
}

//------------------------------------FUNCION PARA APAGAR EL SISTEMA---------------------------------------------
void estadoSistema(){ 
  if(millis() - tiempoAnterior >= 10000){
    Serial.println("Apagar sistema");
    estatus = 1;
    lcd.noDisplay();
    lcd.noBacklight();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    esp_sleep_enable_ext1_wakeup(BIT(GPIO_NUM_2) | BIT(GPIO_NUM_15), ESP_EXT1_WAKEUP_ANY_HIGH); //Despertar por boton
    esp_sleep_enable_timer_wakeup(5 * 1000000ULL); //Despertar por tiempo
    esp_deep_sleep_start();
  }else{
    estatus = 0;
  }
}

//------------------------------------FICHEROS---------------------------------------------
void escribirFichero(String temperatura, String fecha, String hora, String alertaUmbral){
  if(sobrescribir){
    //Abrir archivo para escribir desde 0
    File file = SPIFFS.open("/mediciones.txt", "w");
    file.println("T:"+ temperatura + " F:" + fecha + " H:" + hora + " U:" + alertaUmbral);
    file.close();
    sobrescribir = false;
  }else{
    //Abrir archivo para escribir desde la ultima linea
    File file = SPIFFS.open("/mediciones.txt", "a");
    file.println("T:"+ temperatura + " F:" + fecha + " H:" + hora  + " U:" + alertaUmbral);
    file.close();
  }
}

void guardarMedicion(){
  timeClient.update();
  // Obtiene fecha y hora
  String hora = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  int dia = ptm->tm_mday;
  int mes = ptm->tm_mon+1;
  int year = ptm->tm_year+1900;
  String fecha = (String(dia)+"/"+String(mes)+"/"+String(year));
  mediciones();
  if(millis() - tiempoFlash >= 5000){ //Cada 5 segundos se guarda una muestra
    Serial.println("Guardar Mediciones");
    if(temperaturaActual >= temperaturaDeseada){
      escribirFichero(String(temperaturaActual), fecha, hora, "Temperatura aceptable");
    }else{
      escribirFichero(String(temperaturaActual), fecha, hora, "Aumentando temperatura");
    }
    tiempoFlash = millis();
  }
}

//------------------------------------OBTENER MEDICIONES DEL SENSOR---------------------------------------------
void mediciones(){
  temperaturaActual = sensor_temperatura.readTemperature(); //Leer temperatura actual
  humedadActual = sensor_temperatura.readHumidity(); //Leer humeada actual
}

void setup() {
  Serial.begin(115200);
  pinMode(sensor_t, INPUT);
  pinMode(rele_on, OUTPUT);
  pinMode(rele_off, OUTPUT);
  pinMode(boton_arriba, INPUT);
  pinMode(boton_abajo, INPUT);
  sensor_temperatura.begin();
  botonA.attach(boton_arriba);
  botonA.interval(10);
  botonD.attach(boton_abajo);
  botonD.interval(10);
  esp_sleep_wakeup_cause_t causaDespertar = esp_sleep_get_wakeup_cause();
  switch (causaDespertar) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Despertado por el temporizador");
      // Realizar las acciones necesarias cuando se despierta por el temporizador
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Despertado por un pulsador");
        estatus = 0;
      break;
  }
  tiempoAnterior = millis(); //Empezar a contar el tiempo
  mediciones(); //Actualizar mediciones del sensor
  if(estatus == 0){ //Primer encendido del sistema
    Serial.println("El sistema esta activo por primera vez");
    initWiFi();
    cargarWeb();
    cargarServidor();
    initLCD();
  }else if(estatus == 1){ //El sistema ya se ha apagado
    Serial.println(temperaturaActual);
    if((temperaturaActual < temperaturaDeseada and estatusRele == 1) or (temperaturaActual >= temperaturaDeseada and estatusRele == 0)){ //No tenemos que modificar el estado del rele
      Serial.println("El sistema se desperto pero no hay ninguna accion pendiente");
      esp_sleep_enable_ext1_wakeup(BIT(GPIO_NUM_2) | BIT(GPIO_NUM_15), ESP_EXT1_WAKEUP_ANY_HIGH);
      esp_sleep_enable_timer_wakeup(5 * 1000000ULL);
      esp_deep_sleep_start();
    }
    else if((temperaturaActual >= temperaturaDeseada and estatusRele == 1) or (temperaturaActual < temperaturaDeseada and estatusRele == 0)){ //Tenemos que modificar el estado del rele
      Serial.println("El sistema se ha despertado de nuevo");
      estatus = 0;
      tiempoAnterior = millis();
      initWiFi();
      cargarWeb();
      cargarServidor();
      initLCD();
    }
  }
} 

void loop() {
  pulsaciones(); //Pulsaciones de Botones
  mediciones(); //Actualizar las mediciones del sensor
  guardarMedicion(); //Funcion del log del sistema
  comprobarRele(); //Funcion para encender rele
  estadoSistema(); //Funcion para apagar el sistema
  initServidor(); //Funcion para mantener servidor
  actualizarLCD(); //Actualizar LCD
}
