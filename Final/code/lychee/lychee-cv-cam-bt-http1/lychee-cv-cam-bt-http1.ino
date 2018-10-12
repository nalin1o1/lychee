//
// Uses BT to control platform.
// Uses ESP32Interface WIFI to connect to clients.
// Together, BT-serial4, AND esp32 do NOT work (no BT after wifi)
//
// 2018
//
#include <Arduino.h>
#include <Camera.h>
#include <SD.h>
#include <HTTPServer.h>
#include <mbed_rpc.h>
#include <SdUsbConnect.h>
#include <ESP32Interface.h>
 
ESP32Interface wifi;
#define WIFI_SSID "prabindh-n"
#define WIFI_PW "narrowroad556"
 
Camera camera(320, 240);
SdUsbConnect storage("storage");

int delayMilliSec = 500;
char sendVal = 'r';
int STEPS_PER360 = 10;
int currentRotationStep = 0;
int currentFrontStep = 0;
bool bUseESP32 = true;

static int snapshot_req(const char ** pp_data) {
  size_t size = camera.createJpeg();
  *pp_data = (const char*)camera.getJpegAdr();
  return size;
}
 
void setup(void) {
 
  Serial.begin(9600);
  Serial.println("Starts.");
  // No BT + ESP32Interface
  if(false == bUseESP32)
  {
      // BT - on D0/D1 - UART4
      Serial4.begin(38400);
      delay(delayMilliSec);
  }

  // SD & USB
  Serial.print("Finding storage..");
  storage.wait_connect();
  Serial.println("done");
 
  camera.begin();
 
  // This loop will fail BT Serial4 (BT does not work simultaneously with HTTP)
  if(true == bUseESP32)
  {
      Serial.print("Connecting Wi-Fi..");
      wifi.connect(WIFI_SSID, WIFI_PW, NSAPI_SECURITY_WPA_WPA2);
      Serial.println("done");  
     
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
}
 
void loop()
{
    // BUG - This loop will not work, along with ESP32Interface if enabled
    if(false == bUseESP32)
    {
        // For external to internal (small object) mapping, only move Platform
        //Move forward, then rotate a bit
        sendVal = 'w';
        Serial4.write(sendVal);
        delay(2000); // Run for 2 sec (1 sec doesnt work ? Why ?)
        sendVal = '0';
        Serial4.write(sendVal);
        delay(2000); // Run for 2 sec (1 sec doesnt work ? Why ?)        
        sendVal = 'a';
        Serial4.write(sendVal);
        delay(2000); // Run for 2 sec (1 sec doesnt work ? Why ?)
        sendVal = '0';
        Serial4.write(sendVal);         
        delay(2000); // Run for 2 sec (1 sec doesnt work ? Why ?)
    }
}