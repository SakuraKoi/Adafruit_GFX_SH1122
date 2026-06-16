#include <Adafruit_SH1122.h>

// Adafruit_SH1122(cs, dc, rst, mosi, sclk)
// Uses hardware SPI. MOSI and SCLK default to the board's hardware SPI pins.
// On ESP32/ESP8266, pass custom pin numbers to remap hardware SPI pins:
//   Adafruit_SH1122 display(10, 9, 8, 23, 18);  // CS, DC, RST, MOSI=23, SCLK=18
Adafruit_SH1122 display(10, 9, 8);

void setup() {
    display.begin();

    display.fillScreen(SH1122_BLACK);
    display.drawRect(0, 0, 256, 64, SH1122_WHITE);
    display.setCursor(10, 10);
    display.setTextColor(SH1122_WHITE);
    display.setTextSize(1);
    display.println("SH1122 256x64");
    display.setCursor(10, 30);
    display.println("Adafruit-GFX");
    display.display();
    
    delay(2000);

    display.clearDisplay();
    for (uint16_t i = 0; i < 256; i++)
    {
        display.drawLine(i, 0, i, 63, (i % 16));
    }
    display.display();
}

void loop() {
}
