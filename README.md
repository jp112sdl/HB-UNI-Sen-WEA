# HB-UNI-Sen-WEA
## Selbstbau-Wetterstation (mit Netzteilbetrieb) für HomeMatic 
![complete](Images/4.png)

<br>

#### Code
- verwendete Bibliotheken:
  - _[BH1750](https://github.com/claws/BH1750) Helligkeitsensor (nach Möglichkeit den MAX44009 Sensor einsetzen, dieser hat einen wesentlich größeren Messbereich und benötigt keine Lib)_
  - [BME280](https://github.com/finitespace/BME280) Temperatur / Feuchte / Luftdruck

#### Hardware (Elektronik)
- benötigte Bauteile _(ohne Gewähr, dass es sich um die preiswertesten Angebote handelt!)_:
  - [CC1101 (868MHz)](https://de.aliexpress.com/item/CC1101-Wireless-Module-Long-Distance-Transmission-Antenna-868MHZ-M115-For-FSK-GFSK-ASK-OOK-MSK-64/32635393463.html) 
  - [Arduino Pro Mini 3.3V/8MHz](https://de.aliexpress.com/item/Free-Shipping-1pcs-pro-mini-atmega328-Pro-Mini-328-Mini-ATMEGA328-3-3V-8MHz-for-Arduino/32342672626.html)
  - [VEML6070](https://www.ebay.de/itm/183207531222)
  - [BME280](https://www.ebay.de/itm/253107395109)
  - [Reed-Kontakte](https://www.ebay.de/itm/263000077321)
  - _[BH1750](https://www.ebay.de/itm/162351871499) (nicht mehr empfohlen - der MAX44009 hat einen größeren Messbereich)_
  - [MAX44009](https://www.ebay.de/itm/162727018038)
  - [AS3935](https://de.aliexpress.com/item/New-AS3935-lightning-sensor-Lightning-lightning-lightning-detection-storm-distance/32830108558.html)
  - ein paar Widerstände (Werte bitte dem Schaltplan entnehmen) und einen Taster


#### Gehäuse-/Konstrukt-Teile
Rahmen + Anemometer + Windrichtungszeiger + Regenmengenmesser<br>
https://www.thingiverse.com/thing:2849562

Halterung für die Platine + Sensoren:<br>
https://www.thingiverse.com/thing:2821592

Neodym-Magenete: [eBay-Link](https://www.ebay.de/itm/180935986047) <br>
Kugellager: [eBay-Link](https://www.ebay.de/itm/251368539841)

#### Schaltplan

![wiring](Images/wiring.png)

_Hinweis: Die LED wird nicht mehr benutzt und braucht daher nicht verbaut zu werden._

#### Addon für die CCU / RaspberryMatic

**Um die Geräteunterstützung zu aktivieren, wird die aktuellste Version des [JP-HB-Devices Addon](https://github.com/jp112sdl/JP-HB-Devices-addon/releases/latest) benötigt!**

Einstellungen:<br>
![einstellungen](Images/CCU_Einstellungen.png)

- Sendeintervall
  - Sendeintervall :)
- Altitude
  - Höhe über dem Meeresspiegel (für die barometrische Luftdruckberechnung erforderlich)
- WEATHER|Anemometer Radius (cm)
  - Abstand zwischen Mitte Achse und Mitte Becher
- WEATHER|Anemometer Calibration Factor
  - Multiplikator, um den eigenen Windwiderstand auszugleichen

<br>Bedienung:<br>
![bedienung](Images/CCU_Bedienung.png)


