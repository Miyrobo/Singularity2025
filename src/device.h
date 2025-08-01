#ifndef DEVICE_H
#define DEVICE_H
#include "Arduino.h"
#include "sensors.h"
#include "move.h"

#define Pin_MPA A18 //マルチプレクサ
#define Pin_MPB A19
#define Pin_MPC A20

#define Pin_kicker 2

#define buzzer 30 //ブザー


// センサー関連まとめ
struct Sensors {
  BALL& ball;
  BNO& gyro;
  ULTRASONIC& ping;
  CAMERA& openmv;
  LINE& line;
};

// 駆動・制御関連まとめ
struct Actuator {
  MOTOR& motor;
  MOVE& move;
  PID& pid;
};



#endif