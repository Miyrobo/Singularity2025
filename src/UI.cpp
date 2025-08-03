#include "UI.h"
#include "Pins.h"
#include "Wire.h"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &DISPLAY_I2C, -1);

void Display_Singularityinit(){
  DISPLAY_I2C.setClock(400000);  // 400kHzに設定
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(1);
  display.setCursor(0, 60);
  display.println("Singularity");
  display.display();
}
