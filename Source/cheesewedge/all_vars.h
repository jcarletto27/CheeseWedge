
//-------------------------------------WEB Config-------------------------------------//
#if !(defined(ARDUINO_RASPBERRY_PI_PICO_W))
#error For RASPBERRY_PI_PICO_W only
#endif
#define _RP2040W_AWS_LOGLEVEL_ 1
#include <AsyncWebServer_RP2040W.h>
#include <pico/cyw43_arch.h>



String webServerAddress = "";

char ssid[] = "";  // your network SSID (name)
char pass[] = "";  // your network password (use for WPA, or use as key for WEP), length must be 8+
String strSSID = "";
String strPass = "";
int status = WL_IDLE_STATUS;
AsyncWebServer server(80);
const char* input_parameter1 = "input_string";
const char* bulk_input_string = "bulk_input_string";
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
  <br><br><br>
  <form action="/bulk_write">
    Bulk Write values: <textarea id="bulk_input_string" name="bulk_input_string" rows="4" cols="37" maxlength="3700" required="true"></textarea>
    <input type="submit" value="Submit">
  </form>
  <br>
  
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
uint8_t keys[][6] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                      { 0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7 },
                      { 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0 },
                      { 0xa1, 0xb1, 0xc1, 0xd1, 0xe1, 0xf1 },
                      { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 },
                      { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5 },
                      { 0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd },
                      { 0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a },
                      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                      { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },                      
                      { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
                      { 0x71, 0x4c, 0x5c, 0x88, 0x6e, 0x97 },
                      { 0x58, 0x7e, 0xe5, 0xf9, 0x35, 0x0f },
                      { 0xa0, 0x47, 0x8c, 0xc3, 0x90, 0x91 },
                      { 0x53, 0x3c, 0xb6, 0xc7, 0x23, 0xf6 },
                      { 0x8f, 0xd0, 0xa4, 0xf2, 0x56, 0xe9 } };
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t lastReadUID[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;  // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
bool ndefReady = false;
String ndefData = "";
int previousNDEF_Length = 0;
bool hasWrittenToWedge = false;
long millisLastRead = 0;
String bulkWriteData = "";
int bulkWriteArrayCount = 100;
String bulkWriteArray[100];
char delimiter_key = '\n';
int currentBulkWriteIteration = 0;
bool bulkWriteDataMode = false;
String sucKey = "";
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