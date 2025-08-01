#include "UI.h"
#include "Wire.h"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, -1);

void Display_Singularityinit(){
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(1);
  display.setCursor(0, 60);
  display.println("Singularity");
  display.display();
}
