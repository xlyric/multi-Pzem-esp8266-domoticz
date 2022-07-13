# multi-Pzem-esp8266-domoticz
esp8266 code for mutil Pzem004 on domoticz and web interface

Version 3.0  13/07/2022 



this code was developped for make a hub of PZEM 004 t on an esp8266 and send informations on domoticz server
actualy only 2 pzem is possible but easyly add more 

 *    Powering Pzem : GND and VU on ESP8266
 
 *   Pzem  on D1-D2 pin on the wemos

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


http://IP/set?Publish= publish of MQTT


save to SPIFFS 
http://IP/set?save  >> save config on spiffs

Installation : 

use VS code and plateformIO
or use .bin in the firmware folder 


At the first start it will create a new wifi network ( Pzem ) pwd : Pzem
and you can configure your network.

you will push upload by OTA at the url /update 

For Domoticz 
create dummy's connection and connect with idx  by an MQTT server

for multi Pzem, you need to change the adress of the Pzem ( https://github.com/mandulaj/PZEM-004T-v30/tree/master/examples/PZEMChangeAddress )

Adress : 0x10 and ++

-----------------------------
changelog
-- 20200402 ; New full recreate version  with Web client and MQTT support, wifimanager, OTA, SPIFFS
-- 202220713 ; new version for Pzem V3. 




