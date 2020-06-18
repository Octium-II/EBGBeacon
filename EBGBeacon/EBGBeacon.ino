//ExposeBillGates Beacon - Wifi Beacon to inform nearby devices of the 
//Corbett report documentry on Bill Gates
//
//By Octium (Bassed on example by Ivan Grokhotkov)
//
//CorbettReport.com/Gates

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#define DEBUG Serial

const byte DNS_PORT = 53;
const char *ssid = "CorbettReport.com/Gates";

IPAddress apIP(192, 168, 1, 1);

DNSServer dnsServer;

ESP8266WebServer webServer(80);

// Include "Built in" web pages
#include "index.h"

void setup() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  //start debug port
  DEBUG.begin(115200);
  DEBUG.print("\n");
  DEBUG.setDebugOutput(true);
  DEBUG.print("Wifi Beacon Started.\n");

  webServer.on("/index.html", handleRoot);
    
  //redirect all traffic to index.html
  webServer.onNotFound([]() {
      const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.1.1/index.html\" /></head><body><p>redirecting...</p></body>";
      webServer.send(200, "text/html", metaRefreshStr);
  });  
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  //DEBUG.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
}

void handleRoot() {
  String s = String(index_page);
  webServer.send(200, "text/html", s);
}
