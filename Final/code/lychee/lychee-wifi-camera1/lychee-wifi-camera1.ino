//
// Lychee Arduino type code, to service GET requests for camera feed
// Uses ESP32Interface WIFI to connect to clients.
// BUG - httpserver fails to respond after random GET requests (ex, 50)
// So, a workaround was attempted to reset the whole core via NVIC
//
// 2018

#include <Arduino.h>
#include <Camera.h>
#include <SD.h>
#include <HTTPServer.h>
#include <SdUsbConnect.h>
#include <ESP32Interface.h>

ESP32Interface wifi;
#define WIFI_SSID ""
#define WIFI_PW ""

Camera camera(640, 480);
SdUsbConnect storage("storage");
int numRequests = 0;

static int snapshot_req(const char ** pp_data) {
  size_t size = 0;
  
  size = camera.createJpeg();
  *pp_data = (const char*)camera.getJpegAdr();
  
  ++numRequests;

  return size;
}

void setup(void) {

  Serial.begin(9600);
  // SD & USB
  Serial.print("Finding SD card..");
  storage.wait_connect();
  Serial.println("Connected to SD card");

  camera.begin();

  Serial.print("Connecting Wi-Fi..");
  wifi.connect(WIFI_SSID, WIFI_PW, NSAPI_SECURITY_WPA_WPA2);
  Serial.print("MAC Address is ");
  Serial.println(wifi.get_mac_address());
  Serial.print("IP Address is ");
  Serial.println(wifi.get_ip_address());
  Serial.print("NetMask is ");
  Serial.println(wifi.get_netmask());
  Serial.print("Gateway Address is ");
  Serial.println(wifi.get_gateway());
  Serial.println("Network Setup OK\r\n");

  SnapshotHandler::attach_req(&snapshot_req);
  HTTPServerAddHandler<SnapshotHandler>("/camera"); //Camera
  FSHandler::mount("/storage", "/");
  HTTPServerAddHandler<FSHandler>("/");
  HTTPServerStart(&wifi, 80);
}

void loop() 
{
  // WAR for socket issues. Reset every N
  if(numRequests > 20)
  {
    Serial.println("Resetting");
    NVIC_SystemReset();
  }    
}

