/**************
 *  MultiPzem 
 *  **************
 *  by Cyril Poissonnier 2022
 *  V4 for Pzem V3 
 *    
 *    Powering Pzem : GND and VU on ESP8266
 *
 *   Pzem ar ont Pin 4(RX) and Pin 5 (TX) ( soft serial )
 
 * 
 *  Pzem Connexion
 *  ------------------------
 *  |VDD              AC-IN| LOAD (80-260V -50/60Hz)
 *  |RX               AC-IN| LOAD (80-260V -50/60Hz) 
 *  |TX               CT   | 0-100A 
 *  |GND              CT   | 0-100A
 *  ------------------------
 * 
 * Warning 230V AC is deadly, please beware 
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
#define mqtt_user "Pzem"
// PZEM
//#include <PZEM004T.h>
#include <PZEM004Tv30.h>

/***************************
 * Begin Settings
 **************************/

const String VERSION = "Version 3.0" ;

/// number of Pzems, please prepare with adresse 0x10 and up 
#define NUM_PZEMS 3

PZEM004Tv30 pzems[NUM_PZEMS];

//***********************************
//************* declare soft serial 
//***********************************

#if defined(USE_SOFTWARE_SERIAL) && defined(ESP32)
    #error "Can not use SoftwareSerial with ESP32"
#elif defined(USE_SOFTWARE_SERIAL)

#include <SoftwareSerial.h>

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
#endif


//***********************************
//************* Gestion of web server 
//***********************************
// Create AsyncWebServer object on port 80
WiFiClient domotic_client;
// mqtt
void mqtt(String idx, String value);
PubSubClient client(domotic_client);

AsyncWebServer server(80);
DNSServer dns;
HTTPClient http;



/***************************
 * Parameter 
 **************************/

#define USE_SERIAL  Serial
#define SLEEP_DELAY_IN_SECONDS 5
int envoie=0; 

/***************************
 * End Settings
 **************************/


//***********************************
//************* configuration value 
//***********************************

struct Config {
  char hostname[15];
  int port;
  char Publish[100];
}; 

struct IDX {
  int voltage; 
  int current ; 
  int power;
  int energy;
  int frequency;
  int pf; 
}; 



const char *filename_conf = "/config.json";
String idx_conf = "/idx";
Config config; 
IDX idx[NUM_PZEMS]; 
IDX pzem[NUM_PZEMS]; 

//***********************************
//************* Variables 
//***********************************
 // float voltage[NUM_PZEMS], current[NUM_PZEMS],  power[NUM_PZEMS] , energy[NUM_PZEMS] , frequency[NUM_PZEMS] , pf[NUM_PZEMS] ; 

//***********************************
//************* read file configuration
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
  strlcpy(config.Publish,                  // <- destination
          doc["Publish"] | "domoticz/in", // <- source
          sizeof(config.Publish));         // <- destination's capacity
  configFile.close();
   
}

// Loads the configuration from a file
void loadIDX(const char *filename, IDX &idx) {
  // Open file for reading
  File configFile = LittleFS.open(filename, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config

  idx.voltage = doc["IDX_U"] | 999; 
  idx.current = doc["IDX_C"] | 999; 
  idx.power = doc["IDX_P"] | 999; 
  idx.energy = doc["IDX_E"] | 999; 
  idx.frequency = doc["IDX_F"] | 999; 
  idx.pf = doc["IDX_PF"] | 999; 

    configFile.close();
  
}

void saveConfiguration(const char *filename, const Config &config) {
  
  // Open file for writing
   File configFile = LittleFS.open(filename, "w");
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
  doc["Publish"] = config.Publish;
  


  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  configFile.close();
}

void saveIDX(const char *filename, const IDX &idx) {
  
  // Open file for writing
   File configFile = LittleFS.open(filename, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Set the values in the document
  doc["IDX_U"] = idx.voltage;
  doc["IDX_C"] = idx.current;
  doc["IDX_P"] = idx.power;
  doc["IDX_E"]= idx.energy;
  doc["IDX_F"] = idx.frequency;
  doc["IDX_PF"]= idx.pf;

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  configFile.close();
}
//***********************************
//************* Gestion de la configuration - sauvegarde du fichier de configuration
//***********************************




    //***********************************
    //************* function web 
    //***********************************

String getState() {
  String state; 
      for(int i = 0; i < NUM_PZEMS; i++)
    {
      state += String(pzem[i].voltage) + ";" + String(pzem[i].current) + ";" + String(pzem[i].power) + ";" + String(pzem[i].pf) + ";" ;
    }
//  state = String(v1) + ";" + String(a1) + ";" + String(w1)+ ";" + String(wh1)+ ";" + String(v2) + ";" + String(a2) + ";" + String(w2)+ ";" + String(wh2)+ ";"; 
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
//  configweb = String(config.IDX_U1) + ";" + String(config.IDX_I1) + ";" + String(config.IDX_W1)+ ";" + String(config.IDX_PE1) + ";" + String(config.IDX_U2) + ";" + String(config.IDX_I2) + ";" + String(config.IDX_W2)+ ";" + String(config.IDX_PE2); 
  return String(configweb);
}

    //***********************************
    //************* Setup 
    //***********************************

void setup() {
  Serial.begin(115200);

  Serial.println("start");

  //démarrage file system
  LittleFS.begin();
  Serial.println("Demarrage file System");
  
  USE_SERIAL.println("Pzem Program is starting...");

  //***********************************
  //************* Setup -  Initialize Pzem 
  //***********************************
    for(int i = 0; i < NUM_PZEMS; i++)
    {

      #if defined(USE_SOFTWARE_SERIAL)
              // Initialize the PZEMs with Software Serial
              pzems[i] = PZEM004Tv30(pzemSWSerial, 0x10 + i);
      #elif defined(ESP32)
              // Initialize the PZEMs with Hardware Serial2 on RX/TX pins 16 and 17
              pzems[i] = PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x10 + i);
      #else
              // Initialize the PZEMs with Hardware Serial2 on the default pins

              /* Hardware Serial2 is only available on certain boards.
              *  For example the Arduino MEGA 2560
              */
              pzems[i] = PZEM004Tv30(PZEM_SERIAL, 0x10 + i);
      #endif
      String idx_concat = idx_conf + String(i) + ".json"  ;
    
      const char *idx_file= idx_concat.c_str();
     loadIDX(idx_file, idx[i]);
     Serial.println(idx_file);
     saveIDX(idx_file, idx[i]);
    }

    //***********************************
    //************* Setup -  load file configuration
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

  server.on("/idx1.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/idx1.json", "application/json");
  });

  server.on("/idx2.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/idx2.json", "application/json");
  });

  server.on("/idx0.json", HTTP_ANY, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/idx0.json", "application/json");
  });


/////////////////////////
////// posible to change configuration with web command 
/////////////////////////

server.on("/set", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///   /set?paramettre=xxxx
    if (request->hasParam("save")) { Serial.println(F("Saving configuration..."));
                          saveConfiguration(filename_conf, config);   
                            }
                        
   if (request->hasParam("hostname")) { request->getParam("hostname")->value().toCharArray(config.hostname,15);  }
   if (request->hasParam("port")) { config.port = request->getParam("port")->value().toInt();}
   /*
   if (request->hasParam("IDX_U1")) { config.IDX_U1 = request->getParam("IDX_U1")->value().toInt();}
   if (request->hasParam("IDX_I1")) { config.IDX_I1 = request->getParam("IDX_I1")->value().toInt();}
   if (request->hasParam("IDX_W1")) { config.IDX_W1 = request->getParam("IDX_W1")->value().toInt();}
   if (request->hasParam("IDX_PE1")) { config.IDX_PE1 = request->getParam("IDX_PE1")->value().toInt();}

   if (request->hasParam("IDX_U2")) { config.IDX_U2 = request->getParam("IDX_U2")->value().toInt();}
   if (request->hasParam("IDX_I2")) { config.IDX_I2 = request->getParam("IDX_I2")->value().toInt();}
   if (request->hasParam("IDX_W2")) { config.IDX_W2 = request->getParam("IDX_W2")->value().toInt();}
   if (request->hasParam("IDX_PE2")) { config.IDX_PE2 = request->getParam("IDX_PE2")->value().toInt();}
*/

   if (request->hasParam("Publish")) { request->getParam("Publish")->value().toCharArray(config.Publish,100);}
   request->send(200, "text/html", getconfig().c_str());

  });





    //***********************************
    //************* Setup - web
    //***********************************
  Serial.println("start server");
  server.begin(); 

  // MQTT
  client.connect("Pzem");
  client.setServer(config.hostname, 1883);


  Serial.println(idx[0].current);
  Serial.println(idx[1].current);
  Serial.println(idx[2].current);

}
//---- end Setup ---- 


    //***********************************
    //************* LOOP
    //***********************************

void loop() {

    for(int i = 0; i < NUM_PZEMS; i++)
    {
      Serial.println(pzems[i].getAddress(), HEX);
      // Read the data from the sensor
         pzem[i].voltage = pzems[i].voltage();
         pzem[i].current = pzems[i].current();
         pzem[i].power = pzems[i].power();
         pzem[i].energy = pzems[i].energy();
         pzem[i].frequency = pzems[i].frequency();
         pzem[i].pf = pzems[i].pf();

        Serial.println(pzems[i].voltage());
                // Check if the data is valid
        if(isnan(pzem[i].voltage)){
            Serial.println("Error reading voltage");
        } else if (isnan(pzem[i].current)) {
            Serial.println("Error reading current");
        } else if (isnan(pzem[i].power)) {
            Serial.println("Error reading power");
        } else if (isnan(pzem[i].energy)) {
            Serial.println("Error reading energy");
        } else if (isnan(pzem[i].frequency)) {
            Serial.println("Error reading frequency");
        } else if (isnan(pzem[i].pf)) {
            Serial.println("Error reading power factor");
        }
    
    }

/// envoie des valeurs 

if ( envoie >= 5 ) { 
    for(int i = 0; i < NUM_PZEMS; i++)
    {
    mqtt(String(idx[i].voltage),String(pzem[i].voltage));
    mqtt(String(idx[i].current),String(pzem[i].current));
    mqtt(String(idx[i].power),String(pzem[i].power));
    mqtt(String(idx[i].energy),String(pzem[i].energy));
    mqtt(String(idx[i].pf),String(pzem[i].pf));
      }

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
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
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
