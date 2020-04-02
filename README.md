# multi-Pzem-esp8266-domoticz
esp8266 code for mutil Pzem004 on domoticz and web interface

Version 2.0  02/04/2020 


this code was developped for make a hub of PZEM 004 t on an esp8266 and send informations on domoticz server
actualy only 2 pzem is possible but easyly add more 

 *    Powering Pzem : GND and VU on ESP8266
 
 *   Pzem 1 on D1-D2 pin
 *   Pzem 2 on D3-D4 pin 
 * 
 *  Pzem Connexion
 *  ------------------------
 *  |VDD              AC-IN| LOAD (80-260V -50/60Hz)
 *  |RX               AC-IN| LOAD (80-260V -50/60Hz) 
 *  |TX               CT   | 0-100A 
 *  |GND              CT   | 0-100A
 *  ------------------------

----- USE -------

show start config : http://IP/config.json ( SPIFFS ) 
actual config    http://IP/config

change configuration 
http://IP/set?hostname= IP of MQTT 
http://IP/set?port= port of MQTT
http://IP/set?IDX_U1= your IDX
etc ... 
IDX_I1
IDX_W1
IDX_PE1

IDX_U2
IDX_I2
IDX_W2
IDX_PE2

http://IP/set?Publish= publish of MQTT


save to SPIFFS 
http://IP/set?save  >> save config on spiffs

Installation : 

use .bin files, it's more simple than librairy

The librairy : 
https://codeload.github.com/olehs/PZEM004T/zip/master

At the first start it will create a new wifi network ( Pzem ) pwd : Pzem
and you can configure your network.

For Domoticz 
create dummy's connection and connect with idx  by an MQTT server




-----------------------------
changelog
-- 20200402 ; New full recreate version  with Web client and MQTT support, wifimanager, OTA, SPIFFS





