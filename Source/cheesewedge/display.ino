

void initiateDisplay() {

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  maxLoops = 1;
  display.clearDisplay();
  currx = display.width();
}

void scrollText(String text, uint8_t size) {
  secondLine = text;
  minX = -12 * strlen(text.c_str());
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(topLine);
  display.setTextWrap(false);
  display.setCursor(currx, 10);
  display.setTextSize(2);
  display.print(text);
  display.display();  // Show initial text
  currx = currx - speed;
  if (currx < minX) {
    currx = display.width();
    currentLoops++;
  };
  //}
}



void drawText(String text, uint8_t size) {
  display.stopscroll();
  display.clearDisplay();
  display.setTextWrap(true);
  display.setTextSize(size);  // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println(F(text.c_str()));
  display.display();  // Show initial text
  delay(1000);
}
