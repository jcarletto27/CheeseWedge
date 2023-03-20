// void notFound(AsyncWebServerRequest* request) {
//   request->send(404, "text/plain", "Not found");
// }

void checkWIFIStatus() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }
}

void beginWifi() {
  if (strSSID != "" && strPass != "") {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(strSSID);

    char ssidCharArray[strSSID.length() + 1];
    strSSID.toCharArray(ssidCharArray, strSSID.length() + 1);

    char passCharArray[strPass.length() + 1];
    strPass.toCharArray(passCharArray, strPass.length() + 1);

    status = WiFi.begin(ssidCharArray, passCharArray);
  } else {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }



  while (status != WL_CONNECTED) {
    status = WiFi.status();
    delay(500);
  }


  webServerAddress = "IP Address: http://" + WiFi.localIP().toString() + "/";
  topLine = "  --- CONNECTED --- ";
  secondLine = webServerAddress + "   ";
  speed = 2;
}




void serverBoot() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String input_message;
    String input_parameter;

    if (request->hasParam(input_parameter1)) {
      input_message = request->getParam(input_parameter1)->value();
      //Output to LCD, and Write to NFC
      input_parameter = input_parameter1;
    }

    else {
      input_message = "No message sent";
      input_parameter = "none";
    }
    Serial.println(input_message);
    ;
    cardData = "en" + input_message;
    opMode = "WRITE";
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" + input_parameter + ") with value: " + input_message + "<br>" + "<meta http-equiv=\"Refresh\" content=\"2; url=\'http://" + WiFi.localIP().toString() + "'\" />");
  });

  server.on("/readMode", HTTP_GET, [](AsyncWebServerRequest* request) {
    opMode = "READ";
    Serial.println("Currently in READ mode");
    request->send(200, "text/html", "Setting to scanner to READ Mode <br> <meta http-equiv=\"Refresh\" content=\"2; url=\'http://" + WiFi.localIP().toString() + "'\" />");
  });

  server.on("/writeMode", HTTP_GET, [](AsyncWebServerRequest* request) {
    opMode = "WRITE";
    Serial.println("Currently in WRITE mode");
    request->send(200, "text/html", "Setting to scanner to WRITE Mode <br> <meta http-equiv=\"Refresh\" content=\"2; url=\'http://" + WiFi.localIP().toString() + "'\" />");
  });
  server.on("/format", HTTP_GET, [](AsyncWebServerRequest* request) {
    opMode = "FORMAT";
    Serial.println("Currently in FORMAT mode");
    request->send(200, "text/html", "Setting to scanner to FORMAT Mode <br> <meta http-equiv=\"Refresh\" content=\"2; url=\'http://" + WiFi.localIP().toString() + "'\" />");
  });
  
  //server.onNotFound(handleNotFound);
  server.begin();

  Serial.print(F("HTTP Async_PostServer started @ IP : "));
  Serial.println(WiFi.localIP());
  Serial.println("Serial established, beginning HID and Ghosting Flash Memory");
}

// void handleNotFound(AsyncWebServerRequest* request) {
//   String message = "File Not Found\n\n";
//   message += "URI: ";
//   message += request->url();
//   message += "\nMethod: ";
//   message += (request->method() == HTTP_GET) ? "GET" : "POST";
//   message += "\nArguments: ";
//   message += request->args();
//   message += "\n";

//   for (uint8_t i = 0; i < request->args(); i++) {
//     message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
//   }

//   request->send(404, "text/plain", message);
// }