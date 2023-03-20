
void beginNFC() {
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    drawText("Couldn't find RFID", 1);
    while (1)
      ;  // halt
  }
}

void nfcLastLoop() {
  if (millisLastRead + 10000 < millis()) {
    hasWrittenToWedge = false;
    millisLastRead = millis();
  }

  scrollText(secondLine, 2);
}

void nfcCurrentTextLoop() {
  if (currentLoops > maxLoops) {
    scrollText(" ", 2);
    currentLoops = 0;
    speed = 2;
  }
}

void nfcTagPolling() {
  if (opMode == "READ") {
    uint8_t ndefLength = 0;
    int ndefDataStartPos = 0;
    uint8_t success;
    uint8_t uidLength;           // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    uint8_t currentblock;        // Counter to keep track of which block we're on
    bool authenticated = false;  // Flag to indicate if the sector is authenticated
    uint8_t data[16];            // Array to store block data during reads

    // Keyb on NDEF and Mifare Classic should be the same
    uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);

    if (success) {
      Serial.println("TAG DETECTED");
      if (memcmp(lastReadUID, uid, 7)) {
        memcpy(lastReadUID, uid, 7);  //lastReadUID = uid
        ndefData = "";
        ndefReady = false;
        previousNDEF_Length = 0;
        ndefLength = 0;
        hasWrittenToWedge = false;
        Serial.println("Detected new tag, resetting");
      }

      if (uidLength == 4) {

        // We probably have a Mifare Classic card ...
        for (currentblock = 4; currentblock < 8; currentblock++) {
          if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;
          // If the sector hasn't been authenticated, do so first
          if (!authenticated) {
            success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentblock, 1, keyuniversal);

            if (success) {
              authenticated = true;
            } else {
            }
          }
          if (!authenticated) {
          } else {
            success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
            if (success) {
              if (currentblock == 4) {
                for (int x = 0; x < 16; x++) {
                  if (data[x] == 0x54 && data[x + 1] == 0x02) {
                    ndefLength = data[x - 1] - 3;
                    ndefDataStartPos = x + 4;
                  }
                }
              }
              if ((currentblock + 1) % 4 == 0) {
              } else {
                for (int b = 0; b < 16; b++) {
                  if (currentblock == 4 && b < ndefDataStartPos) {
                    b = ndefDataStartPos;
                  }
                  if (ndefLength > 0 && ndefData.length() < ndefLength) {
                    ndefData += (char)data[b];
                  }
                  if (previousNDEF_Length == ndefData.length()) {
                    ndefReady = true;
                    //Serial.println("Success, reached end of the ndefData");
                  }
                  previousNDEF_Length = ndefData.length();
                }
              }

            } else {
              Serial.print("Block ");
              Serial.print(currentblock, DEC);
              Serial.println(" unable to read this block");
            }
          }
        }

        if (ndefData != "")
          secondLine = ndefData + "  ";

        if ((currx * 2) < minX) currx = 20;
        speed = 2;


        Serial.flush();
        if (TinyUSBDevice.suspended() && ndefReady) {
          // Wake up host if we are in suspend mode
          // and REMOTE_WAKEUP feature is enabled by host
          TinyUSBDevice.remoteWakeup();
        }

        // if (!usb_hid.ready()) {
        //   Serial.print("HID not ready.....");
        //   return;  //still transferring last call
        // }
        if (ndefReady && !hasWrittenToWedge) {
          Serial.println("Is HID Ready? " + usb_hid.ready());
          Serial.println("Printing NDEF Data ");
          for (int iter = 0; iter < strlen(ndefData.c_str()); iter++) {
            char c = ndefData[iter];
            Serial.print(c);
            usb_hid.keyboardPress(0, c);
            delay(10);
            usb_hid.keyboardRelease(0);
            delay(10);
          }
          Serial.println("");
          delay(10);
          usb_hid.keyboardReport(0, 0, enter_keycode);
          delay(10);
          usb_hid.keyboardRelease(0);
          ndefReady = false;
          ndefData = "";
          previousNDEF_Length = 0;
          hasWrittenToWedge = true;
          millisLastRead = millis();
        }

      } else {
        Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
      }
    }
  }
  if (opMode == "WRITE") {  //Write Mode

    secondLine = "SCANNER IS IN WRITE MODE";

    uint8_t success;                          // Flag to check if there was an error with the PN532
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    bool authenticated = false;               // Flag to indicate if the sector is authenticated

    // Use the default NDEF keys (these would have have set by mifareclassic_formatndef.pde!)
    uint8_t keya[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
    uint8_t keyb[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
    uint8_t universal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (success) {
      if (uidLength != 4) {
        Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
        return;
      }
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");


      if (!success) {
        Serial.println("Unable to format the card for NDEF");
        return;
      }

      Serial.println("Card has been formatted for NDEF data using MAD1");


      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keyb);
      if (!success) {
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);  //Backup plan
      } else {
        Serial.println("success in using Key b");
      }
      if (!success) {
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, universal);  //Backup plan for the backup plan
      } else {
        Serial.println("success in using Key a");
      }
      if (!success) {
        Serial.println("Unable to authenticate block 4 ... is this card NDEF formatted?");
        opMode = "FORMAT";
        Serial.println("Currently in FORMAT mode");
        return;
      } else {
        Serial.println("success in using Key Universal");
      }
      Serial.println("Authentication succeeded (seems to be an NDEF/NFC Forum tag) ...");
      if (cardData.length() > 38) {
        Serial.println("URI is too long ... must be less than 38 characters!");
        return;
      }
      Serial.println("Updating sector 1 with URI as NDEF Message");

      success = nfc.mifareclassic_WriteNDEFTEXT(1, cardData.c_str());

      if (success) {
        Serial.println("NDEF URI Record written to sector 1");
        Serial.println("");
        opMode = "READ";
        Serial.println("Currently in READ mode");
      } else {
        Serial.println("NDEF Record creation failed! :(");
      }
    }

    Serial.flush();
  }
  if (opMode == "FORMAT") {
    secondLine = "SCANNER IS IN FORMAT MODE";
    uint8_t success;                          // Flag to check if there was an error with the PN532
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    bool authenticated = false;               // Flag to indicate if the sector is authenticate
    uint8_t keya[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    uint8_t keys[][6] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                          { 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0 },
                          { 0xa1, 0xb1, 0xc1, 0xd1, 0xe1, 0xf1 },
                          { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 },
                          { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5 },
                          { 0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd },
                          { 0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a },
                          { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
                          { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
                          { 0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7 },
                          { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff },
                          { 0x71, 0x4c, 0x5c, 0x88, 0x6e, 0x97 },
                          { 0x58, 0x7e, 0xe5, 0xf9, 0x35, 0x0f },
                          { 0xa0, 0x47, 0x8c, 0xc3, 0x90, 0x91 },
                          { 0x53, 0x3c, 0xb6, 0xc7, 0x23, 0xf6 },
                          { 0x8f, 0xd0, 0xa4, 0xf2, 0x56, 0xe9 } };

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (uidLength != 4) {
      return;
    }
    Serial.println("Seems to be a Mifare Classic card (4 byte UID)");
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 0, 0, keya);
    String sucKey = "";
    for (int tryKey = 0; tryKey < sizeof(keys); tryKey++) {
      if (!success) {
        delay(10);
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 0, 0, keys[tryKey]);
        if(success){
          sucKey = tryKey;
        }
      } else {
        //previous key was successful, do nothing
      }
    }


    if (!success) {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
    } else {
      Serial.println("success in using Key " + sucKey);
    }

    success = nfc.mifareclassic_FormatNDEF();

    if (!success) {
      Serial.println("Unable to format the card for NDEF");
    } else {
      opMode = "WRITE";
      Serial.println("Currently in WRITE mode");
    }
    Serial.println("Card has been formatted for NDEF data using MAD1");
    Serial.flush();
  }
}