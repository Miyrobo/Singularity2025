#ifndef DEVICE_H
#define DEVICE_H
#include "Pins.h"
#include "Arduino.h"
#include "sensors.h"
#include "move.h"



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