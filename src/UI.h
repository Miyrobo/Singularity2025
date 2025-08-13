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

#endif