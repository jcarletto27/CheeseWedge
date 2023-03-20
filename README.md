# CheeseWedge
NFC based keyboard Wedge using PN532, RP2040W (Pico W), and a i2c 128x32 oled


##Features
1. Currently Supports Mifare Classic 1K
    * Read   (Plain text NDEF)
    * Write  (Plain text NDEF)
    * Format to NDEF  
2. NTAG Support Planned
    * Read  
3. HID Keyboard
4. 128x32 oled for Status and Read data
5. Web Server with input for writing to tag, Selecting Modes (Read, Write, Format)
    * Bulk write mode planned  
6. USB Mass Storage Mode
    * wifi.txt
      ```cpp
      SSID:YOUR_SSID_HERE
      Pass:YOUR_PASS_HERE
      ```


---
###Libraries:

  * [Adafruit TinyUSB](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)

  * [Adafruit PN532](https://github.com/adafruit/Adafruit-PN532)

  * [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) 

  * [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)

  * [AsyncTCP_RP2040W](https://github.com/khoih-prog/AsyncTCP_RP2040W)
