//ExposeBillGates Beacon - Wifi Beacon to inform nearby devices of the 
//Corbett report documentry on Bill Gates
//
//By Octium (Bassed on example by Ivan Grokhotkov)
//

#include <ESP8266WiFi.h>

void setup()
{
  Serial.begin(115200);
  Serial.println();

  Serial.print("Setting EBG Beacon (soft-AP Mode)");
  boolean result = WiFi.softAP("CorbettReport.com/Gates");
  if(result)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }
}

void loop()
{
  Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
  delay(3000);
}
