//ピン配置
#define Pin_MUX1 A18 //マルチプレクサ
#define Pin_MUX2 A19
#define Pin_MUX3 A20

#define Pin_Ball1 A17
#define Pin_Ball2 A16

#define Pin_Line1 A0
#define Pin_Line2 A1
#define Pin_Line3 A2
#define Pin_Line4 A3

#define Pin_LineLED 13

#define Pin_kicker 2

#define buzzer 30 //ブザー

#define Pin_S1 11
#define Pin_S2 12
#define Pin_S3 24

#define Pin_TS 21

const int Pin_motor[8] = {22,23,5,6,7,8,9,10};


#define Pin_ballcatch A6

#pragma once
inline void pins_init(){
  pinMode(Pin_MUX1, OUTPUT);
  pinMode(Pin_MUX2, OUTPUT);
  pinMode(Pin_MUX3, OUTPUT);

  pinMode(Pin_kicker, OUTPUT);

  pinMode(A21, INPUT); //マルチプレクサ とジャンパ
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