#ifndef INSTANCE_H
#define INSTANCE_H

#include <UI.h>
#include <move.h>
#include <sensors.h>
#include <device.h>

extern MOTOR motor;
extern BALL ball;
extern BNO gyro;
extern ULTRASONIC ping;
extern MOVE move;
extern CAMERA openmv;
extern PID pid;
extern LINE line;
extern PUSHSWITCH sw1;
extern PUSHSWITCH sw2;
extern PUSHSWITCH sw3;

extern TIMER timer[20];
extern TIMER linetim;
extern TIMER pingset;

extern Sensors sensors;
extern Actuator act;

extern SETTING mysetting;

#endif