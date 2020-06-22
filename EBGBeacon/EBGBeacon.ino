//ExposeBillGates Beacon - Wifi Beacon to inform nearby devices of the 
//Corbett report documentry on Bill Gates
//
//By Octium 
//Based on examples by Ivan Grokhotkov & Hristo Gochkov (See licence.h for details)
//
//CorbettReport.com/Gates

#include "licence.h"

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SDFS.h>
#include <FS.h>

#define DEBUG Serial

FS* fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();
static bool fsOK;
File uploadFile;

const byte DNS_PORT = 53;
const char *ssid = "There's a war on for your mind";

IPAddress apIP(192, 168, 1, 1);

DNSServer dnsServer;

ESP8266WebServer webServer(80);

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";

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

  // Upload file
  // - first callback is called after the request has ended with all parsed arguments
  // - second callback handles file upload at that location
  webServer.on("/upload",  HTTP_POST, replyOK, handleFileUpload);
   
  webServer.onNotFound(handleNotFound);
   
  webServer.begin();

  // FILESYSTEM INIT
  fileSystemConfig.setAutoFormat(true);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  DEBUG.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));  

  dumpFileSystem();
}

void dumpFileSystem(void)
{
  Dir dir = fileSystem->openDir("");
  DEBUG.println(F("List of files at root of filesystem:"));
  while (dir.next()) {
    String fileInfo = dir.fileName() + (dir.isDirectory() ? " [DIR]" : String(" (") + dir.fileSize() + "b)");
    DEBUG.println(fileInfo);
  }
  DEBUG.println();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  //DEBUG.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
}

////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK() {
  webServer.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  webServer.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  webServer.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  DEBUG.println(msg);
  webServer.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  DEBUG.println(msg);
  webServer.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}


void handleNotFound(void) {
      String uri = ESP8266WebServer::urlDecode(webServer.uri()); // required to read paths with blanks
      
      //Rewrite / Handle alternative paths
      if (uri.endsWith("/")) {
        uri += "index.html";
      }
      
      if (handleFileRead(uri)) {
        return;
      }

      //File not found of file system, try for firmware version
      if (uri.endsWith("/index.htm") || uri.endsWith("/index.html"))
      {
        webServer.send(200, "text/html", FPSTR(INDEX_PAGE));
      }
      
      // Could not find a file, so redirect to landing page
      const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.1.1/index.html\" /></head><body><p>redirecting...</p></body>";
      webServer.send(200, "text/html", metaRefreshStr);
}
 
/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  DEBUG.println(String("handleFileRead: ") + path);
  if (!fsOK) {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  String contentType;
  if (webServer.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
    if (webServer.streamFile(file, contentType) != file.size()) {
      DEBUG.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}

void handleFileUpload() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }
  
  HTTPUpload& upload = webServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DEBUG.println(String("handleFileUpload Name: ") + filename);
    uploadFile = fileSystem->open(filename, "w");
    if (!uploadFile) {
      return replyServerError(F("CREATE FAILED"));
    }
    DEBUG.println(String("Upload: START, filename: ") + filename);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize) {
        return replyServerError(F("WRITE FAILED"));
      }
    }
   DEBUG.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    DEBUG.println(String("Upload: END, Size: ") + upload.totalSize);
  }
}
