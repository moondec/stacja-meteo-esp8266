1. [ Potrzebne materiały ](#Potrzebne)
1. [ Schemat połączeń ](#Schemat)
1. [ Instalacja środowiska Arduino IDE ](#Arduino)
1. [ Instalujemy dodatek do obsługi ESP8266 ](#ESP8266)
1. [ Instalujemy potrzebne biblioteki ](#biblioteki)
1. [ Wgrywanie szkicu do ESP8266 ](#Wgrywanie)

> Barcode do tego repozytorium:

![barcode](/README.md.fld/barcode.gif)

# Projekt prostej stacji meteorologicznej na Noc Naukowców 2020.
> ***Projekt ten jest kompilacją dostępnych w Internecie poradników. Oto linki:***
> * https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/ 
> * https://randomnerdtutorials.com/esp8266-ds18b20-temperature-sensor-web-server-with-arduino-ide/ 
> * https://circuits4you.com/2019/03/23/esp8266-bmp180-pressure-sensor-interface/ 
> * https://randomnerdtutorials.com/esp8266-bme280-arduino-ide/

<a name="Potrzebne"></a>
## Potrzebne materiały

* Płytka oparta na ESP8266, np. ___NodeMCU___, ___WemosD1 mini___, 
<img src="/README.md.fld/image001.png" width="600"><img src="/README.md.fld/image002.jpg" width="600"> 

* Termohigrometr ___DHT11___ lub ___DHT22___, 
<img src="/README.md.fld/image003.jpg" width="600"> 

* Termometr Dallas ___DS18B20___, 

<img src="/README.md.fld/image004.jpg"><img src="/README.md.fld/image005.jpg"> 

* Barometr ___BMP180___ lub ___BMP280___, 

<img src="/README.md.fld/image006.jpg"><img src="/README.md.fld/image007.jpg"> 

* Płytka stykowa, 
<img src="/README.md.fld/image008.jpg"> 

* Kabelki, 
<img src="/README.md.fld/image010.jpg"> 

* Komputer i kabel USB z wtyczką mikro-B (stare do ładowania smartfona) 

<a name="Schemat"></a>
## Schemat połączeń

![Schemat](/README.md.fld/image011.png)

<a name="Arduino"></a>
## Instalacja środowiska Arduino IDE

[Pobieramy Arduino IDE z tej lokalizacji](https://www.arduino.cc/en/software)

<a name="ESP8266"></a>
## Instalujemy dodatek do obsługi ESP8266 w/g tej instrukcji:

https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/

Lub po prostu dodajemy obsługę ESP8266 w Arduino IDE:

Dla MacOS lub Linux **Arduino > Preferencje > Dodatkowe adresy URL do menadżera płytek** 

Dla Windows **Plik > Preferencje > Dodatkowe adresy URL do menadżera płytek** 
i wklejamy: 
http://arduino.esp8266.com/stable/package_esp8266com_index.json

Następnie wybieramy **Narządzia > Płytka > Menadżer płytek...**
W otwartym oknie poszukujemy *__esp8266__* i klikamy **Instaluj**. Zajmie to chwilę. 

![install esp8266 lib](/README.md.fld/image025.png)

<a name="biblioteki"></a>
## Instalujemy potrzebne biblioteki:

* #### DHT:
W programie Arduino IDE klikamy kolejno **Szkic > Dołącz bibliotekę &gt; Zarządzaj bibliotekami…**. W oknie wyszukiwania wpisujemy ___DHT___

![install DHT lib](/README.md.fld/image012.png)

* #### Podobnie instalujemy bibliotekę: "Adafruit Unified Sensor”

![install "Adafruit Unified Sensor" lib](/README.md.fld/image021.png)

* #### Podobnie instalujemy bibliotekę: ___onewire___ zwracając uwagę na właściwe pochodzenie, gdyż w zasobach jest dużo bibliotek o podobnych nazwach i przeznaczeniu

![install onewire lib](/README.md.fld/image013.png)

* #### Oraz DS18B20:

![install DS18B20 lib](/README.md.fld/image014.png)

* #### Kolejna biblioteka konieczna do utworzenia miniserwera www na ESP8266: "ESPAsyncWebServer" do zainstalowania. 

[Pobieramy ją z tej lokalizacji.](https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip)

Pobrany plik należy rozpakować i zmienić nazwę na "ESPAsyncWebServer". Folder ten przenieść do **Moje dokumenty &gt; Arduino &gt; libraries**

* #### Powyższe kroki powtarzamy z biblioteką [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip)

* #### Kolejna biblioteka dla [barometru](https://circuits4you.com/wp-content/uploads/2019/03/BMP180_Breakout_Arduino_Library-master.zip) BMP180 do pobrania.

Instalujemy podobnie jak poprzednie. Można też wybrać: **Szkic &gt; Dołącz bibliotekę &gt; Dodaj Bibliotekę .ZIP…**

> Jeśli czujnikiem ciśnienia jest ___BMP280___ również należy zainstalować bibliotekę ("adafruit bme280") do obsługi tego czujnika.

![install DS18B20 lib](/README.md.fld/image020.png)

<a name="Wgrywanie"></a>
## Wgrywanie szkicu do ESP8266

### Sterowniki do konwerterów _USB <-> serial (port szeregowy)_
Układ ESP8266 (inne mikrokontrolery podobnie np. Arduino) Posiadają jedynie port szeregowy (RS232). Wgrywanie oprogramowania wymaga przykładowo takiego [specjalnego programatora](https://botland.com.pl/pl/programatory/4481-programatordebugger-stm8stm32-zgodny-z-st-linkv2-mini-waveshare-10053.html?search_query=programator&results=30). Płytki takie jak _NodeMCU_, czy _WemosD1 mini_ posiadają wbudowany konwerter portu szeregowego. Przważnie jest to układ CH340. 

![openfile](/README.md.fld/image026.png) 

Przed podłączeniem płytki do komputera należy zainstalować sterowniki do tego układu. [Sterowniki i instrukcje jak je zainstalować można znaleźć tutaj](https://sparks.gogo.co.nz/ch340.html) 

>Uwaga *Tylko MacOS* po wgraniu sterownika należy go włączyć w **Preferencje systemowe > Ochrona i prywatność**. [Opis zamieszczono tutaj](https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver.git)

### Przed wgraniem szkicu do mikrokontrolera (NodeMCU, WemosD1, i in.) należy ustawić w Arduino IDE *dwie ważne rzeczy*:
1. Model płytki z mikrokontrolerem
![openfile](/README.md.fld/image024.png) 
1. Port szeregowy do komunikacji ___komputer <-> płytka___
Przed podłączeniem płytki do komputera wybieramy *__Narzędzia > Port__*. W ten sposób możemy wykryć, na którym porcie pojawi się nasza płytka. 
![openfile](/README.md.fld/image022.png) 
Teraz podpinamy płytkę i ponownie wybieramy *__Narzędzia > Port__*
![openfile](/README.md.fld/image023.png) 

> UWAGA: W systemie Windows porty te będą widoczne inaczej jako **COM2, COM3** itd.
### Teraz możemy przystąpić do wgrywania szkicu, czyli piku _.ino_ do płytki 

1. Gotowy szkic należy pobrać z ___githuba___ **stacjaMeteo_bmp180.ino** lub **stacjaMeteo_bmp280.ino** 

1. Pobrany plik wgrywamy do katalogu **Dokumenty &gt; Arduino &gt; stacjaMeteo_bmp180** lub **Dokumenty &gt; Arduino &gt; stacjaMeteo_bmp280**
Otwieramy wybrany plik w Arduino IDE: ![openfile](/README.md.fld/image016.png) 

1. klikamy ![openfile2](/README.md.fld/image015.png) 

1. Zmieniamy ustawienia sieci Wi-Fi na własne
![openfile](/README.md.fld/image017.png)
```C++
// Zastąp danymi własnej sieci wi-fi
const char* ssid = "NAZWA_SIECI_WIFI";
const char* password = "HASŁO_SIECI_WIFI";
``` 
5. Sprawdzamy czy nie ma błędów w składni: ![openfile](/README.md.fld/image018.png) 

1. Jeśli wszystko poszło dobrze można wgrać program: ![openfile](/README.md.fld/image019.png) 

> Jeśli pojawiły się błędy kompilacji (z kroku 5), to najprawdopodobniej nie wszystkie wymagane biblioteki zostały zainstalowane. Należy się upewnić czy jakaś inna bibliotekanie powoduje konfliktów. Jeśli samodzielnie edytowałeś szkic to może pojawił się błąd składni. Najczęściej zapominamy o *__„;”__* na końcu linii bądź polecenia.

<font color:red >POWODZENIA!!!</font>
