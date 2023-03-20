#include "SPI.h"
#include "SdFat.h"
#include "Adafruit_SPIFlash.h"
#include "Adafruit_TinyUSB.h"
#include "flash_config.h"
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PN532.h>
#include "all_vars.h"




// the setup function runs once when you press reset or power the board
void setup() {

  //Must be before Serial//
  initiateFlash();
  initiateHID();
  //Must be before Serial//


  Serial.begin(9600);
  //while (!Serial) delay(10);  // wait for native usb
  Serial.println("Serial is Live, Flash is Live, HID is Live");


  initiateDisplay();  //i2c Display


  flashDetails();  //Print flash sizes and details


  //WIFI Setup//
  checkWIFIStatus();
  getWIFIDetailsFromFile();
  beginWifi();
  //WIFI Setup//

  beginNFC();



  //Instantiate Web Server//
  serverBoot();
  //Instantiate Web Server//

  setUnitReady(false);  //WIFI is good, hide Drive;
  Serial.println("Ending Setup");
}



void loop() {

  //NFC maintenance//
  nfcLastLoop();
  nfcTagPolling();
  nfcCurrentTextLoop();
  //NFC maintenance//


  //File System maintenance//
  fsMissingFormat();
  fsHasChanged();
  flashToggleCheck();
  //File System maintenance//
  
}