/*
 PZEM004T vers Domoticz GET
Evolution by Cyril Poissonnier 2018 ( cyril.poissonnier@gmail.com ) 

Alimentation Pzem : GND and  VU

Pzem 1 on D1-D2 pin
Pzem 2 on D3-D4 pin 

Reset pin connected to D0 pin 



 
*/
//wifi parameter

const char* ssid     = "__WifiSID__"; // Nom du reseau WIFI utilise
const char* password = "__Password__"; // Mot de passe du reseau WIFI utilise

//  Domoticz parameter
const char* domoticz_server = "__Domoticz IP__"; //adresse IP de domoticz
const String IDX_U     = "23"; //idx du capteur virtuels tension             
const String IDX_I     = "22"; //idx du capteur virtuels intensite            
const String IDX_W     = "24"; //idx du capteur virtuels puissance et energie 

const String IDX_U2     = "27"; //idx du capteur virtuels tension             
const String IDX_I2     = "26"; //idx du capteur virtuels intensite            
const String IDX_W2     = "28"; //idx du capteur virtuels puissance et energie 

//  pzem com and wait state
#define SLEEP_DELAY_IN_SECONDS  1 //interval envoi des données

const int rx1 = 4;
const int tx1 = 5;

const int rx2 = 2;
const int tx2 = 0;

/////////////////////////
////////// main program 
/////////////////////////
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h> 
#include <PZEM004T.h>

// initialisation du PZEM004T
PZEM004T pzem(rx1,tx1);  // RX,TX borne D1 and D2
PZEM004T pzem2(rx2,tx2);  // RX,TX borne D3 and D4

IPAddress ip(192,168,1,1);
WiFiClient domoticz_client;

//-------------------------------------------------------------------------------------------------

int nbtent=0;


void setup(void) {
  Serial.begin(115200);
  
  // Connexion au WiFi
  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    if (nbtent<10){
      nbtent++ ;   
      delay(1000);
      Serial.print(".");
    }
    else{
      Serial.println("Reset");
      ESP.deepSleep(2 * 1000000, WAKE_RF_DEFAULT);
    }    
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //initialisation ttl avec PZEM004T
  pzem.setAddress(ip);
  
}

void loop(void){    
  //// Pzem 1 state
  Serial.println("Pzem1");  

  float v = pzem.voltage(ip);
  if (v < 0.0) v = 0.0;
  Serial.print(v);Serial.print("V; ");
  delay(10); 
  float i = pzem.current(ip);
  if(i >= 0.0){ Serial.print(i);Serial.print("A; "); }
  delay(10);   
  float p = pzem.power(ip);
  if(p >= 0.0){ Serial.print(p);Serial.print("W; "); }
  delay(10);   
  float e = pzem.energy(ip);
  if(e >= 0.0){ Serial.print(e);Serial.print("Wh; "); }
  Serial.println();

  EnvoieDomoticz(IDX_U,String(v)); // tension
  EnvoieDomoticz(IDX_I,String(i)); // intensite
  EnvoieDomoticz(IDX_W,String(p)+ ";" +String(e)); // puissance

  delay(100);
//////////////  Pzem 2 state /////////


  Serial.println("Pzem2 ");  
float v2 = pzem2.voltage(ip);
  if (v2 < 0.0) v2 = 0.0;
  Serial.print(v2);Serial.print("V; ");
  delay(10); 
  float i2 = pzem2.current(ip);
  if(i2 >= 0.0){ Serial.print(i2);Serial.print("A; "); }
  delay(10);   
  float p2 = pzem2.power(ip);
  if(p2 >= 0.0){ Serial.print(p2);Serial.print("W; "); }
  delay(10);   
  float e2 = pzem2.energy(ip);
  if(e2 >= 0.0){ Serial.print(e2);Serial.print("Wh; "); }
  Serial.println();

  EnvoieDomoticz(IDX_U2,String(v2)); // tension
  EnvoieDomoticz(IDX_I2,String(i2)); // intensite
  EnvoieDomoticz(IDX_W2,String(p2)+ ";" +String(e2)); // puissance

  delay(100);
/////////////////////////



  Serial.println("sleep of "+String(SLEEP_DELAY_IN_SECONDS)+" seconds...");
  delay(SLEEP_DELAY_IN_SECONDS * 1000);
  }


int EnvoieDomoticz(String IDX, String Svalue) {
  domoticz_client.connect(domoticz_server, 8080);
  delay(500);

  int Resultat=0 ;
  int NbTentEnvoie=0;
  
  String deel1 = "GET /json.htm?type=command&param=udevice&idx="+IDX+"&nvalue=0&svalue=";
  String deel3 = " HTTP/1.1\r\nHost: www.local.lan\r\nConnection: keep-alive\r\nAccept: */*\r\n\r\n";  
  String dataInput = String(deel1 + Svalue + deel3);
  while (Resultat!=1) {
    if (NbTentEnvoie<10){
      NbTentEnvoie++ ;
      Serial.print("_");
      domoticz_client.println(dataInput);
      delay(500);
      // Si le serveur repond lire toute les lignes
      while(domoticz_client.available()){
        String line = domoticz_client.readStringUntil('\r');
        if (line.lastIndexOf("status")>0 and line.lastIndexOf("OK")>0){
          Serial.println("Status OK");
          Resultat = 1 ;
        }
      }
    }
  else{
      Serial.println("Reset");
      ESP.deepSleep(2 * 1000000, WAKE_RF_DEFAULT);
    }  
    
  }  
  // deconexion
  domoticz_client.println("Connection: close");
  domoticz_client.println();  
  return(Resultat);
}
