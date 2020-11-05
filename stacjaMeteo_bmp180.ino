
/*
 * Kompilacja szkicu stacji pogodowej Marek Urbaniak 
 * https://github.com/moondec/stacja-meteo-esp8266.git
 * Oryginalne nagłówki i odnośniki do źródeł:
 * 
 **********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
  Complete project details at https://randomnerdtutorials.com/esp8266-ds18b20-temperature-sensor-web-server-with-arduino-ide/
  
*********/
/* 
 *  BMP180 Interface with ESP8266 Code Example
 *  https://circuits4you.com
 *  March 2019
 *  
Hardware connections:
NodeMCU     BMP180
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
#include <SFE_BMP180.h>
#include <Wire.h>
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

SFE_BMP180 pressure;
 
#define ALTITUDE 84.0 // Określ wysokość n.p.m. swojej lokalizacji
// https://www.wysokosciomierz.pl
#define SEALEVELPRESSURE_HPA (1013.25) // hPa = mb - tu można wstawić pierwszy odczyt p0 zaraz po uruchomieniu i ponownie wgrać szkic do ESP

// Ustawienie wartości poczatkowych zmiennych pomiarowych aktualizowanych w sekcji loop()
float t = 0.0; //DHT - temperatura
float h = 0.0; //DHT - wilgotność
float tempC = -127.0; // DS18B20 - temperatura
char status;
double T = 0.0; //BME180 - temperatura
double P = 0.0; //BME180 - ciśnienie
double p0 = 0.0; //BME180 - ciśnienie zredukowane do p.m.
double a = 0.0; //BME180 - wysokość
  
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
  <h3>BMP180</h3>
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
  return String();
}

// Ta część wykonuje się raz po włączeniu
void setup(){
  // Ustawia port szeregowy do komunikacji ESP8266 z komputerem
  Serial.begin(115200);
  dht.begin();
  // Start up the DS18B20 library
  sensors.begin();
  // Stert up the BMP180 library
  pressure.begin();
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
  // Najpierw odczytaj temperaturę:
  // Jeśli się udało zwraca czas w ms potrzebny do wykonania pomiaru
  // Jeśli się nie udało zwraca 0.
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Czekaj na zakończenie pomiaru:
    delay(status);
 
    // Uzyskuje pomiar:
    // Pomiar przechowywany jest w zmiennej T.
    // Funkcja zwraca 1 jeśli się udało, 0 jeśli nie.
 
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Wyświetl wyniki w Monitorze portu szeregowego:
      Serial.print("Temperatura BMP: ");
      Serial.print(T,2);
      Serial.print(" deg C, ");
      Serial.print((9.0/5.0)*T+32.0,2);
      Serial.println(" deg F");
      
      // Start pomiaru ciśnienia:
      // Parametr funkcji przyjmuje wartości, od 0 do 3 (najszybciej, najdłużej).
      // Jeśli się udało zwraca czas w ms potrzebny do wykonania pomiaru
      // Jeśli się nie udało zwraca 0.
 
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Czekaj na zakończenie pomiaru:
        delay(status);
 
        // Uzyskuje pomiar:
        // Pomiar przechowywany jest w zmiennej P.
        // UWAGA funkcja wymaga wcześniejszego pomiaru temperatury (T).
        // Function returns 1 if successful, 0 if failure.
 
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // Wyświetl wyniki w Monitorze portu szeregowego:
          Serial.print("Cisnienie bezwzgledne: ");
          Serial.print(P,2);
          Serial.print(" mb, "); // mb = hPa
          Serial.print(P*0.0295333727,2);
          Serial.println(" inHg");
 
          // Czujnik cisnienia mierzy cisnienie bezwzględne, które zależy od wysokości n.p.m. 
          // Można sprowadzić tę wartość do wartości ciśnienia na poziomie morza 
          // Co jest najczęściej praktykowane w raportach meteorologicznych
          // Parametry: P = ciśnienie bezwzględne w mb (hPa), ALTITUDE = bieżąca wysokość w m.
          // Wynik: p0 = ciśnienie na poziomie morza w mb (hPa).
 
          p0 = pressure.sealevel(P,ALTITUDE); // jesteśmy na 84 metrach (Poznan, PL)
          Serial.print("Ciśnienie na poziomie morza: ");
          Serial.print(p0,2);
          Serial.print(" mb, ");
          Serial.print(p0*0.0295333727,2);
          Serial.println(" inHg");
 
          // Z drugiej strony majac ciśnienie można wyznaczyć wysokość,
          // zastosuj funkcję altitude. Znając ciśnienie na danej wysokości i ciśnienie na poziomie morza (lub inne bazowe) oblicz różnicę wysokości.
          // Parametry: P = ciśnienie bezwzględne w mb (hPa), SEALEVELPRESSURE_HPA = ciśnienie bazowe np. n.p.m. w mb (hPa)
          // Wynik: a = wysokość w m.
 
          a = pressure.altitude(P,SEALEVELPRESSURE_HPA); // SEALEVELPRESSURE_HPA = 1013.25 - ta wartość powinna być ustalona jako stała dla krótkiego przedziału czasu. Można ją odczytać z p0 zaraz po uruchomieniu.
          Serial.print("Wysokosc: ");
          Serial.print(a,0);
          Serial.print(" metrow, ");
          Serial.print(a*3.28084,0);
          Serial.println(" stop");
        }
        else{ 
          Serial.println("Blad odczytu cisnienia z BMP\n");
          a = -99;
          P = -99;
          }
      }
      else{ 
        Serial.println("Blad uruchomienia pomiaru cisnienia z BMP\n");
        a = -99;
        P = -99;
        }
    }
    else{ 
      Serial.println("Blad odczytu temperatury z BMP\n");
      T = -99;
    }
  }
  else{ 
    Serial.println("Blad uruchomienia pomiaru temperatury z BMP\n");
    T = -99;
  }
 }
}
 
