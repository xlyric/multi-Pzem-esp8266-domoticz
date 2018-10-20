


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

////// function Init And Read Pzem //// 
int Pzem_function( int RX_Pzem , int TX_Pzem , String Voltage_IDX, String Amperage_IDX, String Power_IDX, String Eff_Power_IDX )
{
  // init PZem 
PZEM004T Pzem_ID(RX_Pzem,TX_Pzem);
Pzem_ID.setAddress(ip);

//// serial print Pzem RX  
Serial.println("Pzem");  
Serial.print(RX_Pzem);
Serial.print(" ");
Serial.println(TX_Pzem);

//// Pzem information Reception

  float Voltage = Pzem_ID.voltage(ip);
  if (Voltage < 0.0) Voltage = 0.0;
  Serial.print(Voltage);Serial.print("V; ");
  delay(10); 
  float Amperage = Pzem_ID.current(ip);
  if(Amperage >= 0.0){ Serial.print(Amperage);Serial.print("A; "); }
  delay(10);   
  float Power = Pzem_ID.power(ip);
  if(Power >= 0.0){ Serial.print(Power);Serial.print("W; "); }
  delay(10);   
  float Power_C = Pzem_ID.energy(ip);
  if(Power_C >= 0.0){ Serial.print(Power_C);Serial.print("Wh; "); }
  Serial.println();
  float Eff_Power = Amperage * Voltage ; 
  
  
  EnvoieDomoticz(Voltage_IDX,String(Voltage)); // voltage
  EnvoieDomoticz(Amperage_IDX,String(Amperage)); // intensity
  EnvoieDomoticz(Power_IDX,String(Power)+ ";" +String(Power_C)); // power
  EnvoieDomoticz(Eff_Power_IDX,String(Eff_Power)); // effective power
  delay(100);
  
}
