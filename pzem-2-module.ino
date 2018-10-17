/*  installation 
PZEM004T to Domoticz
Evolution by Cyril Poissonnier 2018 ( cyril.poissonnier@gmail.com ) 

Powering Pzem : GND and VU

Pzem 1 on D1-D2 pin
Pzem 2 on D3-D4 pin 

Reset pin connected to D0 pin 
*/

/////////////////////////   Parameter Code  /////////////////////////////
//wifi parameter

const char* ssid     = "__WifiSID__"; // Wifi SID
const char* password = "__Password__"; // Wifi Password

//  Domoticz parameter
const char* domoticz_server = "__Domoticz IP__"; //adress IP of domoticz

////// First PZEM004T Parameter
const String IDX_U     = "__XX__"; //idx virtual capteur Voltage             
const String IDX_I     = "__XX__"; //idx virtual capteur intensity            
const String IDX_W     = "__XX__"; //idx virtual capteur power and energy  
/////   Pin parameter
const int rx1 = 4; // D1
const int tx1 = 5; // D2


////// 2nd PZEM004T Parameter
const String IDX_U2     = "__XX__"; //idx virtual capteur Voltage             
const String IDX_I2     = "__XX__"; //idx virtual capteur intensity              
const String IDX_W2     = "__XX__"; //idx virtual capteur power and energy  
/////  pin parameter
const int rx2 = 2; // D3 
const int tx2 = 0; // D4

//  pzem com and wait state
#define SLEEP_DELAY_IN_SECONDS  1 //interval sending data

///////////////////// Main Code don't modify after //////////////////



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

  //initialisation ttl with PZEM004T
  pzem.setAddress(ip);
  
}

void loop(void){    
  //// Pzem 1 state
  Serial.println("Pzem1");  
  
  /// verification if wifi is not down else reset 
    if (WiFi.status() != WL_CONNECTED ) 
  {
    ESP.deepSleep(5 * 1000000, WAKE_RF_DEFAULT);
  }

  

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
  /// effective power 
  float pe = i * v ; 
  
  
  EnvoieDomoticz(IDX_U,String(v)); // voltage
  EnvoieDomoticz(IDX_I,String(i)); // intensy
  EnvoieDomoticz(IDX_W,String(p)+ ";" +String(e)); // power
  EnvoieDomoticz(IDX_PE,String(pe)); // effective power
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
  float pe2 = i2 * v2 ; 
  
  
  EnvoieDomoticz(IDX_U2,String(v2)); // voltage
  EnvoieDomoticz(IDX_I2,String(i2)); // intensity
  EnvoieDomoticz(IDX_W2,String(p2)+ ";" +String(e2)); // power
  EnvoieDomoticz(IDX_PE2,String(pe2)); // effective power
  delay(100);
/////////////////////////


  Serial.println("sleep of "+String(SLEEP_DELAY_IN_SECONDS)+" seconds...");
  delay(SLEEP_DELAY_IN_SECONDS * 1000);
  }

//////// function for sending data

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
