
/*
 * Kompilacja szkicu stacji pogodowej Marek Urbaniak 
 * https://github.com/moondec/stacja-meteo-esp8266.git
 * Oryginalne nagłówki i odnośniki do źródeł:
 * 
 **********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
  Complete project details at https://randomnerdtutorials.com/esp8266-ds18b20-temperature-sensor-web-server-with-arduino-ide/
  Complete project details at https://randomnerdtutorials.com/esp8266-bme280-arduino-ide/
*********
Hardware connections:
NodeMCU     BMP280
3.3V         VIN
GND          GND
D1 (GPIO 5)  SCL          
D2 (GPIO 4)  SDA    

             DHT22/11/21  
D3 (GPIO 0)  (sygnal)

             onewire (DS18B20)
D4 (GPIO 2)  (sygnal)
(WARNING: do not connect + to 5V or the sensor will be damaged!)
 
*/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BME280.h>
// Zastąp danymi własnej sieci wi-fi
const char* ssid = "NAZWA_SIECI_WIFI";
const char* password = "HASŁO_SIECI_WIFI";

#define DHTPIN 0     // Pin cyfrowy do którego podłączony jest czujnik DHT: D3 (GPIO 0)
#define ONE_WIRE_BUS 2 // Termometr DS18B20 podłączony do GPIO 2 czyli pin D4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
// Jeśli masz inny sensor niż DHT22 odznacz właściwą linię usuwając znak komentarza // z początku linii:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

//odznacz komentarz jeśli BME280 łączysz przez SPI
/*#include <SPI.h>
#define BME_SCK 14  //D5
#define BME_MISO 12 //D6
#define BME_MOSI 13 //D7
#define BME_CS 15   //D8*/
#define SEALEVELPRESSURE_HPA (1013.25) // hPa = mb
Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

// Ustawienie wartości poczatkowych zmiennych pomiarowych aktualizowanych w sekcji loop()
float t = 0.0; //DHT - temperatura
float h = 0.0; //DHT - wilgotność
float tempC = -127.0; // DS18B20 - temperatura
char status;
float T = 0.0; //BME280 - temperatura
float P = 0.0; //BME280 - ciśnienie
float W = 0.0; //BME280 - wilgotność powietrza
float a = 0.0; //BME280 - wysokość
  
// Tworzenie serwera WWW AsyncWebServer object na porcie 80 - domyślny dla przeglądarek
AsyncWebServer server(80);

unsigned long previousMillis = 0;    // przechowuje czas ostatniego pomiaru

// Ustaw częstotliwość pomiarów 10 seconds
const long interval = 10000;  
// zawartość strony interntowej
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h1 { font-size: 1.5rem; }
    h2 { font-size: 0.9rem; }
    h3 { font-size: 1.3rem; }
    p { font-size: 2.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h1>ESP8266 Server</h1>
  <h2>Noc Naukowc&#243;w 2020</h2>
  <h3>DHT</h3>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperatura</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Wilgotno&#347;&#263;</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&#37;</sup>
  </p>
  <h3>DS18B20</h3>
  <p>
    <i class="fas fa-thermometer-three-quarters" style="color:#9e0519;"></i> 
    <span class="dht-labels">Temperatura przy gruncie</span> 
    <span id="dstemp">%DSTEMP%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <h3>BMP280</h3>
  <p>
    <i class="fas fa-thermometer-three-quarters" style="color:#ff8d18;"></i> 
    <span class="dht-labels">Temperatura</span> 
    <span id="tempBMP">%TEMPBMP%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-compress" style="color:#28a745;"></i> 
    <span class="dht-labels">Ci&#347;nienie</span> 
    <span id="barrP">%BARP%</span>
    <sup class="units">hPa</sup>
  </p>
  <p>
    <i class="fas fa-ruler-vertical" style="color:#b0e0e6;"></i> 
    <span class="dht-labels">Wysoko&#347;&#263; npm</span> 
    <span id="alti">%ALTI%</span>
    <sup class="units">m</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#b0e0e6;"></i> 
    <span class="dht-labels">Wilgotno&#347;&#263;</span> 
    <span id="alti">%HUMBMP%</span>
    <sup class="units">&#37;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("dstemp").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/dstemp", true);
  xhttp.send();
}, 10000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("tempBMP").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/tempBMP", true);
  xhttp.send();
}, 10000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("barrP").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/barrP", true);
  xhttp.send();
}, 10000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("alti").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/alti", true);
  xhttp.send();
}, 10000) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humBMP").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humBMP", true);
  xhttp.send();
}, 10000) ;
</script>
</html>)rawliteral";

// Podmienia na stronie wartości pomiarów na aktualne
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  else if(var == "DSTEMP"){
    return String(tempC);
  }
  else if(var == "TEMPBMP"){
    return String(T);
  }
  else if(var == "BARP"){
    return String(P);
  }
  else if(var == "ALTI"){
    return String(a);
  }
  else if(var == "HUMBMP"){
    return String(W);
  }
  return String();
}

// Ta część wykonuje się raz po włączeniu
void setup(){
  // Ustawia port szeregowy do komunikacji ESP8266 z komputerem
  Serial.begin(115200);
  dht.begin();
  // Start up the DS18B20 library
  sensors.begin();
  // Start up the BMP280 library  
  bme.begin();  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Po włączeniu Monitora portu szeregowego pozwala wyświetlić IP adres ESP8266 jakie dostało od routera
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/dstemp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(tempC).c_str());
  });
  server.on("/tempBMP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(T).c_str());
  });
  server.on("/barrP", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(P).c_str());
  });
  server.on("/alti", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(a).c_str());
  });
  // Start server
  server.begin();
}

// Ta część wykonuje się co 10 sekund
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Blad odczytu z DHT!");
      t=-99;
    }
    else {
      t = newT;
      Serial.print("Temperatura DHT: ");
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Blad odczytu z DHT!");
      h=-99;
    }
    else {
      h = newH;
      Serial.print("Wilgotnosc DHT: ");
      Serial.println(h);
    }
  // DS18B20
  sensors.requestTemperatures(); 
  float new_tempC = sensors.getTempCByIndex(0);
  if(new_tempC == -127.00) {
    Serial.println("Blad odczytu z DS8B20!");
    tempC = -99;
  } else {
    Serial.print("Temperatura DS18B20 C: ");
    tempC=new_tempC;
    Serial.println(tempC); 
  }
  // BME280 
  float new_Tbme = bme.readTemperature();
  if(isnan(new_Tbme)) {
    Serial.println("Blad odczytu z BME280!");
    T = -99;
  } else {
    Serial.print("Temperatura BME280 C: ");
    T=new_Tbme;
    Serial.println(T); 
  }
  // Cisnienie
  float new_Pbme = bme.readPressure();
  if(isnan(new_Pbme)) {
    Serial.println("Blad odczytu z BME280!");
    P = -99;
  } else {
    Serial.print("Cisnienie BME280 hPa: ");
    P=new_Pbme/100; //Pa -> hPa
    Serial.println(P); 
  }
  // Wysokość
  float new_Abme = bme.readAltitude(SEALEVELPRESSURE_HPA);
  if(isnan(new_Abme)) {
    Serial.println("Blad odczytu z BME280!");
    a = -99;
  } else {
    Serial.print("Wysokosc BME280 m: ");
    a=new_Abme;
    Serial.println(a); 
  }
  // Wilgotność
  float new_Wbme = bme.readHumidity();
  if(isnan(new_Wbme)) {
    Serial.println("Blad odczytu z BME280!");
    W = -99;
  } else {
    Serial.print("Wilgotnosc BME280 %: ");
    W=new_Wbme;
    Serial.println(W); 
  }
 } 
}
 
