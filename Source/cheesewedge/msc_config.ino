// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

void setUnitReady(bool val) {
  usb_msc.setUnitReady(val);  //Once connected over wifi hide drive
  flashIsReady = false;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb(void) {
  // sync with flash
  flash.syncBlocks();
  // clear file system's cache to force refresh
  fatfs.cacheClear();
  fs_changed = true;
}

void flashToggleCheck() {
  if (BOOTSEL) {
    // Wait for BOOTSEL to be released
    int count = 0;
    while (BOOTSEL) {
      if (flashIsReady && count < 1 && BOOTSEL) {  //deactivate Flash
        usb_msc.setUnitReady(false);
        flashIsReady = false;
        Serial.println("Killing the Flash");
        count++;
      }
      if (!flashIsReady && count < 1 && BOOTSEL) {  //reactivate flash --- DEFAULT ON BOOT
        Serial.println("Attempting to resurrect the Flash");
        flashIsReady = true;
        initiateFlash();
        count++;
      }
      delay(1);
    }


    count = 0;
  }
}


String splitSubString(String text, String delimiter) {
  char delimiterEnd = '\n';
  unsigned int startIndex = text.indexOf(delimiter);
  unsigned int endIndex = text.indexOf(delimiterEnd, startIndex);
  String out = text.substring(startIndex + delimiter.length(), endIndex);
  out = trimString(out);
  return out;
}

String trimString(String text) {  //Sanitize me
  String out = "";
  for (int i = 0; i < text.length(); i++) {
    char cur = text[i];
    if (cur > 31 && cur < 127) {
      out = out + cur;
    }
  }
  return out;
}


void updateWIFICreds(String txtContents) {

  strSSID = splitSubString(txtContents, "SSID:");
  Serial.println("Found SSID : " + strSSID);
  strPass = splitSubString(txtContents, "PASS:");
  Serial.println("Found Pass : " + strPass);
}

void getWIFIDetailsFromFile() {
  if (fs_changed) {
    fs_changed = false;
    Serial.println("Opening root");
    if (!root.open("/")) {
      Serial.println("open root failed");
      return;
    }
    Serial.println("Flash contents:");
    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while (file.openNext(&root, O_RDONLY)) {
      int fileSize = file.printFileSize(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if (file.isDir()) {
        // Indicate a directory.
        Serial.write('/');
      } else {
        Serial.println("");
        int curByte = 0;
        contents = "";
        while (curByte != -1) {
          curByte = file.read();
          if (curByte != -1) {
            contents = contents + (char)curByte;
          }
        }
        contents = contents + '\n';
        Serial.println(contents);
        updateWIFICreds(contents);
      }
      Serial.println();
      file.close();
    }
    root.close();
    Serial.println();
    delay(5000);  // refresh every 1 second
  }
}


void flashDetails() {
  Serial.println("Adafruit TinyUSB Mass Storage External Flash example");
  Serial.print("JEDEC ID: 0x");
  Serial.println(flash.getJEDECID(), HEX);
  Serial.print("Flash size: ");
  Serial.print(flash.size() / 1024);
  Serial.println(" KB");
}

void initiateFlash() {
  flash.begin();
  usb_msc.setID("Adafruit", "External Flash", "1.0");
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  usb_msc.setCapacity(flash.size() / 512, 512);
  usb_msc.setUnitReady(true);
  usb_msc.begin();
  if (!fs_formatted) fs_formatted = fatfs.begin(&flash);
  fs_changed = true;
  flashIsReady = true;
}

void fsMissingFormat() {
  if (!fs_formatted) {

    fs_formatted = fatfs.begin(&flash);
    if (!fs_formatted) {
      Serial.println("Failed to init files system, flash may not be formatted");
      Serial.println("Please format it as FAT12 with your PC or using Adafruit_SPIFlash's SdFat_format example:");
      Serial.println("- https://github.com/adafruit/Adafruit_SPIFlash/tree/master/examples/SdFat_format");
      Serial.println();
      delay(1000);
      return;
    }
  }
}

void fsHasChanged() {
  if (fs_changed) {
    fs_changed = false;
    Serial.println("Opening root");
    if (!root.open("/")) {
      Serial.println("open root failed");
      return;
    }
    Serial.println("Flash contents:");
    // Open next file in root.
    // Warning, openNext starts at the current directory position
    // so a rewind of the directory may be required.
    while (file.openNext(&root, O_RDONLY)) {
      int fileSize = file.printFileSize(&Serial);
      Serial.write(' ');
      file.printName(&Serial);
      if (file.isDir()) {
        // Indicate a directory.
        Serial.write('/');
      } else {
        Serial.println("");
        int curByte = 0;
        contents = "";
        while (curByte != -1) {
          curByte = file.read();
          if (curByte != -1) {
            contents = contents + (char)curByte;
          }
        }
        Serial.println(contents);
      }
      Serial.println();
      file.close();
    }
    root.close();
    Serial.println();
    delay(1000);  // refresh every 1 second
  }
}