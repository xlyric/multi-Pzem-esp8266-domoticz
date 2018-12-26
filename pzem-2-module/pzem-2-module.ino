
/*  installation 
PZEM004T to Domoticz
Evolution by Cyril Poissonnier 2018 ( cyril.poissonnier.pzem@gmail.com ) 
Update 2018-12-25

Powering Pzem : GND and VU on ESP8266

Pzem 1 on D1-D2 pin
Pzem 2 on D3-D4 pin 
..... Pzem X on Dx-Dy pin 

Reset pin connected to D0 pin 
*/

/* Pzem Connexion
 *  ------------------------
 *  |VDD              AC-IN| LOAD (80-260V -50/60Hz)
 *  |RX               AC-IN| LOAD (80-260V -50/60Hz) 
 *  |TX               CT   | 0-100A 
 *  |GND              CT   | 0-100A
 *  ------------------------
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
const String IDX_PE     = "__XX__"; //idx virtual capteur effective power  
/////   Pin parameter
const int rx1 = D1; // D1
const int tx1 = D2; // D2


////// 2nd PZEM004T Parameter
const String IDX_U2     = "__XX__"; //idx virtual capteur Voltage             
const String IDX_I2     = "__XX__"; //idx virtual capteur intensity              
const String IDX_W2     = "__XX__"; //idx virtual capteur power and energy  
const String IDX_PE2     = "__XX__"; //idx virtual capteur effective power  
/////  pin parameter
const int rx2 = D3; // D3 
const int tx2 = D4; // D4

////// Xth PZEM004T Parameter /// Add X blocs 
/*const String IDX_Ux     = "__XX__"; //idx virtual capteur Voltage             
const String IDX_Ix     = "__XX__"; //idx virtual capteur intensity              
const String IDX_Wx     = "__XX__"; //idx virtual capteur power and energy  
const String IDX_PEx     = "__XX__"; //idx virtual capteur effective power  
/////  pin parameter
const int rx2 = DX; // D3 
const int tx2 = DY; // D4
*/



//  pzem com and wait state
#define SLEEP_DELAY_IN_SECONDS  30 //interval sending data

///////////////////// Main Code don't modify after //////////////////
////////// Just Pzem_function ( rx1,tx1, IDX_U, IDX_I, IDX_W, IDX_PE ); with you parameters ) 



/////////////////////////
////////// main program 
/////////////////////////
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h> 
#include <PZEM004T.h>

// initialisation du PZEM004T
//PZEM004T pzem(rx1,tx1);  // RX,TX borne D1 and D2
//PZEM004T pzem2(rx2,tx2);  // RX,TX borne D3 and D4

IPAddress ip(192,168,1,1);
WiFiClient domoticz_client;

//-------------------------------------------------------------------------------------------------

int nbtent=0;
int timebeforereset=1000;

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
//  pzem.setAddress(ip);
  
}

void loop(void){    
  //// Pzem 1 state
  Serial.println("Pzem1");  
  
  /// verification if wifi is not down else reset 
    if (WiFi.status() != WL_CONNECTED ) 
  {
    ESP.deepSleep(5 * 1000000, WAKE_RF_DEFAULT);
  }

 
/////////////////////        Modify here ////////////////////   
//////////////  Pzem  state and send /////////
Pzem_function ( rx1,tx1, IDX_U, IDX_I, IDX_W, IDX_PE );

  
//////////////  Pzem 2 state and send /////////
Pzem_function ( rx2,tx2, IDX_U2, IDX_I2, IDX_W2, IDX_PE2 );


//////////////  Pzem X state and send /////////
//Pzem_function ( rxx,txx, IDX_Ux, IDX_Ix, IDX_Wx, IDX_PEx );


////////////////// don't modify after ///////////////////

/////////  wait before restart mesure 

 
  
   //// correction ESP hang after a long time  
  timebeforereset--;
  if (timebeforereset <=0 ) 
  {
    ESP.reset() ; 
  }
  
  Serial.println("sleep of "+String(SLEEP_DELAY_IN_SECONDS)+" seconds...");
  delay(SLEEP_DELAY_IN_SECONDS * 1000);
  
  }

///////////// End Of loop 
