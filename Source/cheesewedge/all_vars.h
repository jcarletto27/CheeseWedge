
//-------------------------------------WEB Config-------------------------------------//
#if !(defined(ARDUINO_RASPBERRY_PI_PICO_W))
#error For RASPBERRY_PI_PICO_W only
#endif
#define _RP2040W_AWS_LOGLEVEL_ 1
#include <AsyncWebServer_RP2040W.h>
#include <pico/cyw43_arch.h>



String webServerAddress = "";

char ssid[] = "";      // your network SSID (name)
char pass[] = "";  // your network password (use for WPA, or use as key for WEP), length must be 8+
String strSSID = "";
String strPass = "";
int status = WL_IDLE_STATUS;
AsyncWebServer server(80);
const char* input_parameter1 = "input_string";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Tag Writer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FF0000;}
  </style>
  </head><body>
  <h2>Tag Data Entry</h2> 
  <form action="/get">
    Enter a text value: <input type="text" name="input_string" maxlength="37">
    <input type="submit" value="Submit">
  </form>
  <br>
  <form action="/readMode">
  <input type="submit" value=" Read Mode "> 
  </form>
  <p>   </p>
  <form action="/writeMode">
  <input type="submit" value=" Write Mode "> 
  </form>
  <p>   </p>
  <form action="/format">
  <input type="submit" value=" Format Mode "> 
  </form>
</body></html>)rawliteral";
//-------------------------------------WEB Config-------------------------------------//

//-------------------------------------MSC Config-------------------------------------//
Adafruit_SPIFlash flash(&flashTransport);
FatVolume fatfs;
FatFile root;
FatFile file;
Adafruit_USBD_MSC usb_msc;
bool fs_formatted = false;
bool fs_changed = true;
bool flashIsReady = false;
String contents = "";
//-------------------------------------MSC Config-------------------------------------//


//-------------------------------------NFC Config-------------------------------------//
#define PN532_SCK (18)
#define PN532_MISO (16)
#define PN532_MOSI (19)
#define PN532_SS (17)
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);
String opMode = "READ";
String cardData = "enSAMPLE TEXT";
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t lastReadUID[] = { 0, 0, 0, 0, 0, 0, 0 };
bool ndefReady = false;
String ndefData = "";
int previousNDEF_Length = 0;
bool hasWrittenToWedge = false;
long millisLastRead = 0;
//-------------------------------------NFC Config-------------------------------------//



//-------------------------------------I2C Screen Config-------------------------------------//
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);
String topLine = "";
String secondLine = "";
int currx, minX, maxLoops, currentLoops, speed;
//-------------------------------------I2C Screen Config-------------------------------------//

//-------------------------------------HID Config-------------------------------------//
u_int8_t enter_keycode[6] = { HID_KEY_ENTER };
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};
Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);
//-------------------------------------HID Config-------------------------------------//