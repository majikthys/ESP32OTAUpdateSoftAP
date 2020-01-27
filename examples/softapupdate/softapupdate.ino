/**
softapupdate.ino based on ESP32 core arduino examples:
https://github.com/espressif/arduino-esp32/blob/master/libraries/ArduinoOTA/examples/OTAWebUpdater/OTAWebUpdater.ino

//TODO make softapupdate a lib

Copyright 2020 jeremy franklin-ross
See License.md for terms (GNU Lesser General Public License v2.1)

**/
 
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <FS.h>
#include <SPIFFS.h>

//TODO move passwords to a .h file
#define FORMAT_SPIFFS_IF_FAILED true
#define VERSION "0.0.9"
#define WIFI_SSID "TEST_SSID"
#define WIFI_PASSWORD "TEST_PASSWORD"
// OTA Upload Form:
// TODO Add note that WIFI reboot require reconnection after upload to confirm 
#define FORM_HTML  "<form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>" \
                       "<input type='file' name='update'>" \
                       "<input type='submit' value='Update'>" \
                    "</form>" \
                    "<div>THIS IS VERSION " VERSION "</div>" \

WebServer server(80);
 
/**
 * TODO move this away
 * List directories on SPIFFS
 */
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}



/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);
  if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  listDir(SPIFFS, "/", 0);
  
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  // Make this device appear on a deterministic IP address (192.168.42.1) 
  delay(100); // <- Supposedly necessary hack
  if (!WiFi.softAPConfig(IPAddress(192,168,42,1), IPAddress(192,168,42,1),  IPAddress(255,255,255,0))) {
    Serial.println("SOFT AP CONFIG FAILED");
    exit;
  }

  Serial.println();
  Serial.println("software version: " VERSION);
  Serial.println("WIFI_SSID: " WIFI_SSID);
  Serial.println("WIFI_PASSWORD: " WIFI_PASSWORD);
  Serial.print("url: http://");
  Serial.println(WiFi.softAPIP());

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    Serial.println("server.on(/)");
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", FORM_HTML);
  });

  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    Serial.println("server.on(/update)");
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    delay(1500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Serial.println("!Update.begin(UPDATE_SIZE_UNKNOWN) \nERROR:");
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Serial.println("Update.write(upload.buf, upload.currentSize) != upload.currentSize)...  \nERROR:");
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}
 
void loop(void) {
  server.handleClient();
  delay(1);
}
