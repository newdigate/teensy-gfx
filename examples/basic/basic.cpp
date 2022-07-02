#include <Arduino.h>
#include "View.h"

#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F


View display = View(128,128);

void setup() {

  Serial.begin(9600);

  delay(10);
  display.setRotation(1);
  display.fillScreen(ST77XX_BLACK);
}
float f = 0;

void loop() {
    f += 0.01;
    int16_t scrollY = round(60.0 * sin(f));
    display.fillScreen(ST77XX_BLUE);
    display.fillRect(32, 32, 64,64, ST77XX_RED);
    display.setTextColor(ST77XX_WHITE);
    display.drawString("XXXXX",0,0);
    delay(1);
}

int main() {
    setup();
    while (true) {
        loop();
    }
}