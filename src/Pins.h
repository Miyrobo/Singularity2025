#include "Arduino.h"
//ピン配置
#define Pin_MUX1 A18 //マルチプレクサ
#define Pin_MUX2 A19
#define Pin_MUX3 A20

#define Pin_Ball1 A17 //ボールセンサ マルチプレクサ使用
#define Pin_Ball2 A16

#define Pin_Line1 A0 //ラインセンサ マルチプレクサ使用
#define Pin_Line2 A3
#define Pin_Line3 A2
#define Pin_Line4 A1

#define Pin_LineLED 13 //ラインのLED点灯状態切り替え

#define Pin_kicker 2 //ソレノイドキッカー出力

#define buzzer 30 //ブザー

#define Pin_S1 11 //タクトスイッチ
#define Pin_S2 12
#define Pin_S3 24

#define Pin_TS 21 //トグルスイッチ

#define BNO_I2C Wire
#define DISPLAY_I2C Wire2

#define OpenMV_UART Serial4
#define ESP32_UART Serial5

const int Pin_motor[8] = {22,23,5,6,7,8,9,10}; //モーター


#define Pin_ballcatch A6 //ボール捕捉

#pragma once
inline void pins_init(){
  pinMode(Pin_MUX1, OUTPUT);
  pinMode(Pin_MUX2, OUTPUT);
  pinMode(Pin_MUX3, OUTPUT);

  pinMode(Pin_kicker, OUTPUT);

  pinMode(A21, INPUT); //マルチプレクサ とジャンパ接続
  pinMode(A22, INPUT); //

  pinMode(Pin_Line1, INPUT);
  pinMode(Pin_Line2, INPUT);
  pinMode(Pin_Line3, INPUT);
  pinMode(Pin_Line4, INPUT);

  pinMode(Pin_Ball1, INPUT);
  pinMode(Pin_Ball2, INPUT);



  pinMode(Pin_LineLED, OUTPUT);

  pinMode(Pin_S1, INPUT);
  pinMode(Pin_S2, INPUT);
  pinMode(Pin_S3, INPUT);
  pinMode(Pin_TS, INPUT);

  pinMode(Pin_ballcatch, INPUT);

  pinMode(buzzer, OUTPUT);
}