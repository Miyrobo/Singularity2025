#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define SCREEN_ADDRESS \
  0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

extern Adafruit_SSD1306 display;

void Display_Singularityinit();

void Startup_sound(); //起動音

//ユーザー設定
#define BALL_IS_ORANGE 0
#define BALL_IS_IR 1
class SETTING{
  public:
    void load();
    void save();

    u_int8_t goalcolor;
    u_int8_t movespeed;
    u_int8_t usecamera;
    u_int8_t balltype;
    u_int8_t useuss;
    u_int8_t neopixel;
  private:
    const int eeprom_adrs=40;
    int eeprom_count=0;
};

// 押しボタン用クラス
class PUSHSWITCH {
private:
  int pin;
  bool state = 0;
  bool lastStableState = 1;
  bool lastReadState = 1;
  unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 10; // ms
  bool lastReportedState = 1;

  void update();

public:
  PUSHSWITCH(int pinno);

  bool read();    // 安定化された現在の状態を返す
  bool pushed();  // 押された瞬間（エッジ）を検出
};

#include <FastLED.h>

#define NUM_LEDS 12   // LEDの数
#define LED_PIN  25   // ピン番号固定
#define BRIGHTNESS  15

#define NeoPixel_ON 0
#define NeoPixel_OFF 1
#define NeoPixel_Auto 2

class NeoPixel {
public:
  NeoPixel(int numLeds = NUM_LEDS, uint8_t brightness = BRIGHTNESS);

  void begin();             // 初期化
  void update();            // アニメーション更新
  void setColorAll(CRGB color);// 全部同じ色にする
  void rainbow();           // レインボーパターン
  void clear();             // 消灯

  void show();

  void setLedDirection(int dir, int n, CRGB color);

  CRGB* LEDs = new CRGB[NUM_LEDS];

private:
  int _numLeds;
  uint8_t _brightness;
  CRGB* _leds;
  unsigned long _lastUpdate;
  int _hue;

  int r=0;
};


#endif