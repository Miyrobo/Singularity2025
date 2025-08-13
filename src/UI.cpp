#include "UI.h"
#include "Pins.h"
#include "Wire.h"
#include "sensors.h"
#include "EEPROM.h"
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

void SETTING::load(){
  this->goalcolor = EEPROM.read(eeprom_adrs);
  this->movespeed = EEPROM.read(eeprom_adrs + 1);
  this->usecamera = EEPROM.read(eeprom_adrs + 2);
  this->balltype  = EEPROM.read(eeprom_adrs + 3);
}

void SETTING::save(){
  if(eeprom_count<50){ //書き込み過多防止
    EEPROM.write(eeprom_adrs,this->goalcolor);
    EEPROM.write(eeprom_adrs+1,this->movespeed);
    EEPROM.write(eeprom_adrs+2,this->usecamera);
    EEPROM.write(eeprom_adrs+3,this->balltype);
    eeprom_count++;
  }
}

TIMER tim;
void Startup_sound(){
  tim.reset();
  tone(buzzer, 2714, 120);
  int i = 0;
  while (tim.get() < 300) {
    if (SW1 || SW2 || SW3) {
      noTone(buzzer);
      tone(buzzer, 2500,100);
      break;
    }
    if (tim.get() > 200 && i == 2) {
      noTone(buzzer);
      tone(buzzer, 3047);
      i++;
    } else if (tim.get() > 100 && i == 1) {
      noTone(buzzer);
      tone(buzzer, 2876);
      i++;
    } else if (i == 0) {
      noTone(buzzer);
      tone(buzzer, 2714);
      i++;
    }
  }
  noTone(buzzer);
}

PUSHSWITCH::PUSHSWITCH(int pinno) {
  pin = pinno;
  pinMode(pin, INPUT);
}

void PUSHSWITCH::update() {
  bool currentRead = read();

  if (currentRead != lastReadState) {
    lastDebounceTime = millis();  // 状態が変化 → タイマーリセット
  }

  if ((millis() - lastDebounceTime) >= debounceDelay) {
    if (currentRead != lastStableState) {
      lastStableState = currentRead;  // 状態確定
    }
  }

  lastReadState = currentRead;
}

bool PUSHSWITCH::read() { return digitalRead(pin); }

bool PUSHSWITCH::pushed() {
  update();

  bool result = false;

  if (lastStableState != lastReportedState) {
    if (lastReportedState == 1 && lastStableState == 0) {
      result = true;  // 押された瞬間
    }
    lastReportedState = lastStableState;
  }

  return result;
}