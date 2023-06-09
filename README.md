# CheeseWedge
NFC based keyboard Wedge using PN532, RP2040W (Pico W), and a i2c 128x32 oled


## Features
* Currently Supported Tag type(s)
    * Mifare Classic 1K
      * Read   (Plain text NDEF)
      * Write  (Plain text NDEF)
      * Format to NDEF  

* HID Keyboard

* Web Server
   * Bulk Upload

* 128x32 oled for Status and Read data


* Web Server with input for writing to tag, Selecting Modes (Read, Write, Format)

* Web Server
   * Bulk Upload

* USB Mass Storage Mode
    * wifi.txt
      ```cpp
      SSID:YOUR_SSID_HERE
      PASS:YOUR_PASS_HERE
      ```
* NTAG Support
   * Read support for content less then 37
      
## Planned Features



---
### Libraries:

  * [Adafruit TinyUSB](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)
  * [Adafruit PN532 - Modified for Text ndefs and NTAG sendAck ](https://github.com/jcarletto27/Adafruit-PN532)
  * [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) 
  * [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
  * [AsyncTCP_RP2040W](https://github.com/khoih-prog/AsyncTCP_RP2040W)
  


---

### BOM
* Arduino Pico W or equivalent RP2040W board
* Elechouse PN532 NFC Board
* .9 Inch 128x32 oled
* 5x7 cm universal Perfboard with corner mounting holes
