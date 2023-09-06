// #define DEBUG 1

// Included libraries
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
// Included sketch files
#include "OTAPageHtml.h"
#include "MainCode.h"

WebServer server(80);

TaskHandle_t webServerTaskHandle;

void setup() {

#ifdef DEBUG
  Serial.begin(115200);
  while(!Serial) yield();
  Serial.println();
  Serial.print("mainTask running on core ");
  Serial.println(xPortGetCoreID());
#endif

  // Execute setup from MainCode.h
  setup1();

  // Handle web server requests on separate core
  // Main code runs on core 1, so web server runs on core 0
  xTaskCreatePinnedToCore(
    webServerTask,         /* Task function. */
    "webServer",           /* name of task. */
    16384,                 /* Stack size of task */
    NULL,                  /* parameter of the task */
    1,                     /* priority of the task */
    &webServerTaskHandle,  /* Task handle to keep track of task */
    0                      /* pin task to core 0 */
  );
  delay(500);

}

void returnSuccess(String msg) {
  server.send(200, "text/plain", msg);
}

void returnFailure(String msg) {
  server.send(500, "text/plain", msg);
}

uint8_t connectToWiFi() {
  // Delete old config
  WiFi.disconnect(true);
  // Try connecting WiFi
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESP32LED");
  bool isConnected = false;
  for (int i = 0; i < NETWORKS_COUNT; i++) {
#ifdef DEBUG
    Serial.print("Connecting to ");
    Serial.print(SSIDs[i]);
#endif
    WiFi.begin(SSIDs[i], passwords[i]);
    for (int j = 0; j < 10; j++) {
      if (WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG
        Serial.print(".");
#endif
        delay(200);
      } else {
        isConnected = true;
        break;
      }
    }
#ifdef DEBUG
    Serial.println();
#endif
    if (isConnected == true) {
#ifdef DEBUG
      Serial.print("Connected to ");
      Serial.println(SSIDs[i]);
      Serial.print("ESP32 hostname: ");
      Serial.println(WiFi.getHostname());
      Serial.print("Ready! Access at: http://");
      Serial.println(WiFi.localIP());
#endif
      connectedToWiFiCallback();
      break;
    }
  }
  // If not connected, block device
  if (isConnected == false) {
#ifdef DEBUG
    Serial.println("Unable to connect to WiFi");
#endif
  }
  return WiFi.status();
}

void webServerTask(void * pvParameters) {
#ifdef DEBUG
  Serial.print("webServerTask running on core ");
  Serial.println(xPortGetCoreID());
#endif

  // Must establish WiFi connection
  while (connectToWiFi() != WL_CONNECTED) {
#ifdef DEBUG
    Serial.println("Trying again...");
#endif
  }

  // OTA upgrade page (protected by basic auth)
  server.on("/", HTTP_GET, []() {
    if (!server.authenticate(authUser, authPass)) {
      return server.requestAuthentication();
    }
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", otaPageHtml);
  });
  // Discovery endpoint
  server.on("/esp32_present", HTTP_GET, []() {
    returnSuccess("Present");
  });
  // Firmware version endpoint
  server.on("/fw_version", HTTP_GET, []() {
    returnSuccess(FW_VERSION);
  });
  // firmware upload endpoint
  server.on("/upgrade", HTTP_POST, []() {
    returnSuccess(Update.hasError() ? "FAIL" : "OK");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
#ifdef DEBUG
        Serial.println("Disabling WDT on core 0, so device doesn't reboot mid update...");
#endif
        disableCore0WDT();
#ifdef DEBUG
        Serial.printf("Upgrade started, FW file: %s\r\n", upload.filename.c_str());
#endif
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
#ifdef DEBUG
          Update.printError(Serial);
#endif
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
#ifdef DEBUG
          Update.printError(Serial);
#endif
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
#ifdef DEBUG
          Serial.printf("Upgrade completed, transfered: %u bytes\r\nRebooting device...\r\n", upload.totalSize);
#endif
          enableCore0WDT();
        } else {
#ifdef DEBUG
          Update.printError(Serial);
#endif
        }
      }
    }
  );
  // LED commands endpoint
  server.on("/process_command", HTTP_POST, []() {
    if (server.args() == 0) {
      returnFailure("No command sent");
    } else {
      String cmd = server.arg("command");
      if (cmd.length() == 0) {
        returnFailure("Empty command");
      } else {
        returnSuccess("Success");
        processCommand(cmd);
      }
    }
  });
  // Start web server
  server.begin();

  while(true) {
    if (WiFi.status() != WL_CONNECTED) {
#ifdef DEBUG
      Serial.println("WiFi connection lost, reconnecting...");
#endif
      // Must establish WiFi connection
      while (connectToWiFi() != WL_CONNECTED) {
#ifdef DEBUG
        Serial.println("Trying again...");
#endif
      }
    }
    server.handleClient();
    delay(10); // MUST HAVE THIS DELAY
  }
}

void loop() {
  // Execute loop from Code.h
  loop1();
}
