#include "instance.h"
#include "Pins.h"

//クラス インスタンス
MOTOR motor;
BALL ball;
BNO gyro;
ULTRASONIC ping;
MOVE move;
CAMERA openmv;
PID pid;
LINE line;
PUSHSWITCH sw1(Pin_S1);
PUSHSWITCH sw2(Pin_S2);
PUSHSWITCH sw3(Pin_S3);

TIMER timer[20];
TIMER linetim;
TIMER pingset;

//ロボットの状態をまとめる構造体
Sensors sensors{ball, gyro, ping, openmv, line};
Actuator act{motor, move, pid};

SETTING mysetting;

NeoPixel ledring;