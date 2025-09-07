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
  this->useuss = EEPROM.read(eeprom_adrs + 4);
  this->neopixel = EEPROM.read(eeprom_adrs + 5);
}

void SETTING::save(){
  if(eeprom_count<50){ //書き込み過多防止
    EEPROM.write(eeprom_adrs,this->goalcolor);
    EEPROM.write(eeprom_adrs+1,this->movespeed);
    EEPROM.write(eeprom_adrs+2,this->usecamera);
    EEPROM.write(eeprom_adrs+3,this->balltype);
    EEPROM.write(eeprom_adrs+4,this->useuss);
    EEPROM.write(eeprom_adrs+5,this->neopixel);
    
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

//NeoPixel
NeoPixel::NeoPixel(int numLeds, uint8_t brightness)
  : _numLeds(numLeds), _brightness(brightness), _lastUpdate(0), _hue(0) {
  _leds = new CRGB[_numLeds];
}

void NeoPixel::begin() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(_leds, _numLeds);
  FastLED.setBrightness(_brightness);
  clear();
  FastLED.show();
}

void NeoPixel::update() {
  unsigned long now = millis();
  if (now - _lastUpdate > 10) {  // 10msごとに更新
    _hue+=5;  // 全体を回転させる
    _lastUpdate = now;
    rainbow();
    FastLED.show();
  }
}

void NeoPixel::setColorAll(CRGB color) {
  for (int i = 0; i < _numLeds; i++) {
    LEDs[i] = color;
  }
  //FastLED.show();
}

void NeoPixel::rainbow() {
  for (int i = 0; i < _numLeds; i++) {
    // i の位置ごとに色相をずらす
    _leds[i] = CHSV((-_hue + ((i) * 255 / _numLeds)) % 255, 255, 255);
  }
  
}


void NeoPixel::clear() {
  fill_solid(LEDs, _numLeds, CRGB::Black);
  //FastLED.show();
}

void NeoPixel::show(){
  for(int i=0;i<_numLeds;i++){
    _leds[i] = LEDs[(i+(_numLeds/2))%_numLeds];
  }
  FastLED.show();
}


void NeoPixel::setLedDirection(int dir, int n, CRGB color) {
    // dirを0～359に正規化
    dir = (dir % 360 + 360) % 360;

    // dirに一番近いLEDのインデックスを計算
    int center = (dir + 15) / 30 % NUM_LEDS;  // 四捨五入

    // 中心LEDから±(n/2)個を点灯
    for (int i = -n/2; i < (n+1)/2; i++) {
        int idx = (center + i + NUM_LEDS) % NUM_LEDS; // 負数にも対応
        LEDs[idx] = color;
    }
}