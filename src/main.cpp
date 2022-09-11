/**************
 *  MultiPzem 
 *  **************
 *  by Cyril Poissonnier 2022
 *  V3
 *    
 *    Powering Pzem : GND and VU on ESP8266
 *
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
 */


/***************************
 * Librairy
 **************************/

// Web services
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWiFiManager.h>    
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h> 
// File System
#include <LittleFS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
// ota mise à jour sans fil
#include <WiFiUdp.h>
#include <AsyncElegantOTA.h>
//mqtt
#include <PubSubClient.h>
#define mqtt_user "mosquitto"
#define mqtt_passwd "test-123"
// PZEM
#include <PZEM004T.h>

/***************************
 * Begin Settings
 **************************/

const String VERSION = "Version 2.0" ;

//***********************************
//************* Gestion du serveur WEB
//***********************************
// Create AsyncWebServer object on port 80
WiFiClient domotic_client;
// mqtt
void mqtt(String idx, String value);
PubSubClient client(domotic_client);

AsyncWebServer server(80);
DNSServer dns;
HTTPClient http;

//const char* HA_discovery_msg = "{\"name\": \"Pzem\",\"device_class\":\"power\",\"unit_of_measurement\":\"W\",\"icon\":\"mdi:transmission-tower\",\"value_template\":\"{{ value_json.power}}\"}" ;
//const char* topic_HA = "homeassistant/sensor" ; 


/***************************
 * Parameter 
 **************************/

#define USE_SERIAL  Serial
#define SLEEP_DELAY_IN_SECONDS 1
int envoie=0; 
/***************************
 * Pzem Parameter 
 **************************/
 /////   Pin parameter Pzem 1
const int rx1 = 4; //D1
const int tx1 = 5; //D2

const int rx2 = 2; //D3
const int tx2 = 0; //D4

PZEM004T Pzem_1(rx1,tx1);
PZEM004T Pzem_2(rx2,tx2);
IPAddress ip(192,168,1,1);

/***************************
 * End Settings
 **************************/


//***********************************
//************* Gestion de la configuration
//***********************************

struct Config {
  char hostname[15];
  int port;
  int IDX_U1;
  int IDX_I1;
  int IDX_W1;
  int IDX_PE1;
  
  int IDX_U2;
  int IDX_I2;
  int IDX_W2;
  int IDX_PE2;
  
  char Publish[100];
  
};

const char *filename_conf = "/config.json";
Config config; 

//***********************************
//************* Variables 
//***********************************
  float v1,a1,w1, wh1,   v2,a2,w2, wh2; 

//***********************************
//************* Gestion de la configuration - Lecture du fichier de configuration
//***********************************

// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File configFile = LittleFS.open(filename_conf, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  config.port = doc["port"] | 1883;
  strlcpy(config.hostname,                  // <- destination
          doc["hostname"] | "192.168.1.20", // <- source
          sizeof(config.hostname));         // <- destination's capacity
  config.IDX_U1 = doc["IDX_U1"] | 101; 
  config.IDX_I1 = doc["IDX_I1"] | 102; 
  config.IDX_W1 = doc["IDX_W1"] | 103; 
  config.IDX_PE1 = doc["IDX_PE1"] | 104; 
  config.IDX_U2 = doc["IDX_U2"] | 105; 
  config.IDX_I2 = doc["IDX_I2"] | 106; 
  config.IDX_W2 = doc["IDX_W2"] | 107; 
  config.IDX_PE2 = doc["IDX_PE2"] | 108;
  
  strlcpy(config.Publish,                  // <- destination
          doc["Publish"] | "domoticz/in", // <- source
          sizeof(config.Publish));         // <- destination's capacity
  configFile.close();
      
}

//***********************************
//************* Gestion de la configuration - sauvegarde du fichier de configuration
//***********************************

void saveConfiguration(const char *filename, const Config &config) {
  
  // Open file for writing
   File configFile = LittleFS.open(filename_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Set the values in the document
  doc["hostname"] = config.hostname;
  doc["port"] = config.port;
 
  doc["IDX_U1"] = config.IDX_U1; 
  doc["IDX_I1"] = config.IDX_I1; 
  doc["IDX_W1"] = config.IDX_W1; 
  doc["IDX_PE1"] = config.IDX_PE1; 

  doc["IDX_U2"] = config.IDX_U2; 
  doc["IDX_I2"] = config.IDX_I2; 
  doc["IDX_W2"] = config.IDX_W2; 
  doc["IDX_PE2"] = config.IDX_PE2; 

  doc["Publish"] = config.Publish;
  


  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  configFile.close();
}
 



    //***********************************
    //************* function web 
    //***********************************

String getState() {
  String state; 
  state = String(v1) + ";" + String(a1) + ";" + String(w1)+ ";" + String(wh1)+ ";" + String(v2) + ";" + String(a2) + ";" + String(w2)+ ";" + String(wh2)+ ";"; 
  return String(state);
}


String processor(const String& var){
   Serial.println(var);
  if (var == "STATE"){
    return getState();
  } 
    if (var == "VERSION"){
    return (VERSION);
  } 
 return (VERSION);
} 
 

String getconfig() {
  String configweb;  
  configweb = String(config.IDX_U1) + ";" + String(config.IDX_I1) + ";" + String(config.IDX_W1)+ ";" + String(config.IDX_PE1) + ";" + String(config.IDX_U2) + ";" + String(config.IDX_I2) + ";" + String(config.IDX_W2)+ ";" + String(config.IDX_PE2); 
  return String(configweb);
}

    //***********************************
    //************* Setup 
    //***********************************

void setup() {
  Serial.begin(115200);

  //démarrage file system
  LittleFS.begin();
  Serial.println("Demarrage file System");
  
  USE_SERIAL.println("Pzem Program is starting...");

      //***********************************
    //************* Setup -  récupération du fichier de configuration
    //***********************************
  
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadConfiguration(filename_conf, config);

  // Create configuration file
  Serial.println(F("Saving configuration..."));
  saveConfiguration(filename_conf, config);

  

    //***********************************
    //************* Setup - Connexion Wifi
    //***********************************

  AsyncWiFiManager wifiManager(&server,&dns);
  wifiManager.autoConnect("Pzem","Pzem");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //Si connexion affichage info dans console
  Serial.println("");
  Serial.print("Connection ok sur le reseau :  ");
 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println(ESP.getResetReason());

    //***********************************
    //************* Setup - OTA 
    //***********************************
    AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  
    //***********************************
    //************* Setup - Web pages
    //***********************************
   
   server.on("/",HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", String(), false, processor);
  }); 

  server.on("/state", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState().c_str());
  });

  server.on("/all.min.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/all.min.css", "text/css");
  });

    server.on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.ico", "image/png");
  });

  server.on("/fa-solid-900.woff2", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/fa-solid-900.woff2", "text/css");
  });
  
    server.on("/sb-admin-2.js", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/sb-admin-2.js", "text/javascript");
  });

  server.on("/sb-admin-2.min.css", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/sb-admin-2.min.css", "text/css");
  });

  server.on("/config.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.json", "application/json");
  });

/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

server.on("/set", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///   /set?paramettre=xxxx
    if (request->hasParam("save")) { Serial.println(F("Saving configuration..."));
                          saveConfiguration(filename_conf, config);   
                            }
                          
   if (request->hasParam("hostname")) { request->getParam("hostname")->value().toCharArray(config.hostname,15);  }
   if (request->hasParam("port")) { config.port = request->getParam("port")->value().toInt();}
   if (request->hasParam("IDX_U1")) { config.IDX_U1 = request->getParam("IDX_U1")->value().toInt();}
   if (request->hasParam("IDX_I1")) { config.IDX_I1 = request->getParam("IDX_I1")->value().toInt();}
   if (request->hasParam("IDX_W1")) { config.IDX_W1 = request->getParam("IDX_W1")->value().toInt();}
   if (request->hasParam("IDX_PE1")) { config.IDX_PE1 = request->getParam("IDX_PE1")->value().toInt();}

   if (request->hasParam("IDX_U2")) { config.IDX_U2 = request->getParam("IDX_U2")->value().toInt();}
   if (request->hasParam("IDX_I2")) { config.IDX_I2 = request->getParam("IDX_I2")->value().toInt();}
   if (request->hasParam("IDX_W2")) { config.IDX_W2 = request->getParam("IDX_W2")->value().toInt();}
   if (request->hasParam("IDX_PE2")) { config.IDX_PE2 = request->getParam("IDX_PE2")->value().toInt();}


   if (request->hasParam("Publish")) { request->getParam("Publish")->value().toCharArray(config.Publish,100);}
   request->send(200, "text/html", getconfig().c_str());

  });





    //***********************************
    //************* Setup - web
    //***********************************
  Serial.println("start server");
  server.begin(); 

  // MQTT
  client.setServer(config.hostname, 1883);
  client.connect("Pzem",mqtt_user,mqtt_passwd);
  client.publish(topic_HA, HA_discovery_msg, true);

  //// pzem

  Pzem_1.setAddress(ip);
  Pzem_2.setAddress(ip);


}

    //***********************************
    //************* LOOP
    //***********************************

void loop() {
/// récupération des valeurs
   v1 = Pzem_1.voltage(ip);                                     // Associe la variable float "v" au Voltage
  
   a1 = Pzem_1.current(ip);                                     // Associe la variable float "a" au Courant
  
   w1 = Pzem_1.power(ip);                                       // Associe la variable float "w" a la Puissance
  
   wh1 = Pzem_1.energy(ip);                                     // Associe la variable float "wh" a la Consommation


   v2 = Pzem_2.voltage(ip);                                     // Associe la variable float "v" au Voltage
  
   a2 = Pzem_2.current(ip);                                     // Associe la variable float "a" au Courant
  
   w2 = Pzem_2.power(ip);                                       // Associe la variable float "w" a la Puissance
  
   wh2 = Pzem_2.energy(ip);                                     // Associe la variable float "wh" a la Consommation

/// envoie des valeurs 
if ( envoie >= 25 ) { 
mqtt(String(config.IDX_U1),String(v1));
mqtt(String(config.IDX_I1),String(a1));
mqtt(String(config.IDX_W1),String(w1));
mqtt(String(config.IDX_PE1),String(wh1));

mqtt(String(config.IDX_U2),String(v2));
mqtt(String(config.IDX_I2),String(a2));
mqtt(String(config.IDX_W2),String(w2));
mqtt(String(config.IDX_PE2),String(wh2));
envoie = 0;
delay(5000);
}
else {envoie ++; }



delay(SLEEP_DELAY_IN_SECONDS * 1000);


}







void reconnect() {
  // Loop until we're reconnected
  int timeout = 0; 
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Pzem";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user, mqtt_passwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      timeout++; // after 10s break for apply command 
      if (timeout > 2) {
          Serial.println(" try again next time ") ; 
          break;
          }

    }
  }
}



void mqtt(String idx, String value)
{
  reconnect();
  String nvalue = "0" ; 
  if ( value != "0" ) { nvalue = "2" ; }
  String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";


  client.loop();
  client.publish(config.Publish, String(message).c_str(), true);
  
  String jdompub = String(config.Publish) + "/"+idx ;
  client.publish(jdompub.c_str() , value.c_str(), true);

  Serial.println("MQTT SENT");
  
}
