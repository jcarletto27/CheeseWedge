
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

        if (TinyUSBDevice.suspended() && ndefReady) {
          TinyUSBDevice.remoteWakeup();
        }




      } else {
        if (uidLength == 7) {
          char fullData[48];
          Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");
          uint8_t data[48];

          int counter = 0;

          for (uint8_t i = 4; i < 17; i++) {
            nfc.sendAck();
            success = nfc.ntag2xx_ReadPage(i, data);

            if (success) {
              for (int x = 0; x < 4; x++) {
                fullData[counter] = (char)data[x];
                counter++;
              }
            } else {
              Serial.println("Unable to read the requested page!");
            }
          }

          if (success && !hasWrittenToWedge) {
            ndefData = processTextBytes(fullData);
            Serial.println(ndefData);
            ndefReady = true;
          }
        } else {
          //Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
        }

        //Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
      }
    }
    if (ndefReady && !hasWrittenToWedge) {

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
  }
  if (opMode == "WRITE") {  //Write Mode

    secondLine = "SCANNER IS IN WRITE MODE";

    uint8_t success;  // Flag to check if there was an error with the PN532
    //uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;           // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    bool authenticated = false;  // Flag to indicate if the sector is authenticated

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {
      if (memcmp(lastReadUID, uid, 7)) {
        memcpy(lastReadUID, uid, 7);  //lastReadUID = uid
        Serial.println("Detected new tag");


        if (uidLength != 4) {
          Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
          return;
        }
        Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

        if (!success) {
          Serial.println("Unable to format the card for NDEF");
          return;
        }

        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keys[1]);

        if (!success) {
          for (int tryKey = 0; tryKey < 16; tryKey++) {
            if (!success) {
              delay(10);
              success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keys[tryKey]);
              if (success) {
                sucKey = tryKey;
                break;
              }
            } else {
              //previous key was successful, do nothing
            }
          }
        }
        if (!success) {
          Serial.println("Unable to authenticate block 0 to enable card formatting!");
        } else {
          Serial.println("success in using Key " + sucKey);
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

        if (bulkWriteDataMode && currentBulkWriteIteration < bulkWriteArrayCount) {
          cardData = "en" + bulkWriteArray[currentBulkWriteIteration];
          Serial.println(cardData);
        }


        if (cardData.length() > 38) {
          Serial.println("URI is too long ... must be less than 38 characters!");
          return;
        }
        Serial.println("Updating sector 1 with URI as NDEF Message");

        success = nfc.mifareclassic_WriteNDEFTEXT(1, cardData.c_str());

        if (success) {
          if (!bulkWriteDataMode) {  //normal mode
            Serial.println("NDEF URI Record written to sector 1");
            Serial.println("");
            opMode = "READ";
            Serial.println("Currently in READ mode");
          } else {
            Serial.print("Success in Writing NDEF");

            currentBulkWriteIteration++;  //continue bulk write
            if (bulkWriteArray[currentBulkWriteIteration] == "") {
              bulkWriteDataMode = false;
              opMode = "READ";
              delay(1000);
            }
          }


        } else {
          Serial.println("NDEF Record creation failed! :(");
        }
      } else {
        Serial.println("Preexisting Tag");
        delay(1000);
      }

      //Serial.flush();
    }
  }
  if (opMode == "FORMAT") {
    secondLine = "SCANNER IS IN FORMAT MODE";
    uint8_t success;                          // Flag to check if there was an error with the PN532
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    bool authenticated = false;               // Flag to indicate if the sector is authenticate



    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    if (uidLength != 4) {
      return;
    }
    Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 0, 0, keys[0]);

    if (!success) {
      for (int tryKey = 0; tryKey < 16; tryKey++) {
        if (!success) {
          delay(10);
          success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 0, 0, keys[tryKey]);
          if (success) {
            sucKey = tryKey;
            break;
          }
        } else {
          //previous key was successful, do nothing
        }
      }
    }
    if (!success) {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
    } else {
      Serial.println("success in using Key " + sucKey);
    }

    if (!success) {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
    }

    success = nfc.mifareclassic_FormatNDEF();

    if (!success) {
      Serial.println("Unable to format the card for NDEF");
    } else {
      opMode = "WRITE";
      Serial.println("Currently in WRITE mode");
    }
    Serial.println("Card has been formatted for NDEF data using MAD1");
    //Serial.flush();
  }
}



uint8_t authenticateBlock(uint32_t block, int firstKey) {
  String sucKey = "";
  uint8_t success;

  success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, block, 0, keys[firstKey]);

  if (!success) {
    for (int tryKey = 0; tryKey < 16; tryKey++) {
      if (!success) {
        delay(10);
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, block, 0, keys[tryKey]);
        if (success) {
          sucKey = tryKey;
          break;
        }
      } else {
        //previous key was successful, do nothing
      }
    }
  }
  if (!success) {
    Serial.println("Unable to authenticate block 0 to enable card formatting!");
  } else {
    Serial.println("success in using Key " + sucKey);
  }
  return success;
}

String processTextBytes(char textBytes[]) {
  int length = strlen(textBytes);

  String retValues = "";
  bool foundStart = false;
  for (uint8_t i = 0; i < length; i++) {

    if (textBytes[i] == 0x54 && textBytes[i + 1] == 0x02) {
      Serial.println("Found start of text block");
      //found a text record
      i = i + 4;  //skip this record, the 0x02, and the E(0x65) N(0x6E)
      foundStart = true;
    }

    if (foundStart && textBytes[i] != 0xFE) {
      retValues = retValues + textBytes[i];
    }

    if (textBytes[i] == 0xFE) {
      Serial.println("Found end of text block");
    }
  }
  return trimString(retValues);
}
