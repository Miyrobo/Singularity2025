#ifndef MOVE_H
#define MOVE_H
#include <Arduino.h>
#include "sensors.h"

class MOTOR {
 public:
  MOTOR(); //コントラクタ 初期化処理
  int m_speed[4];
  void set_power(int, int, int, int);           //回転速度を直接指定
  void cal_power(int dir, int speed);           //方向 & 速度
  void cal_power(int dir, int speed, int rot);  // 回転を加える  +で時計回り
  void stop();
  void pwm_out();

  bool brake=true; //ブレーキかけるか

 private:
  const int _frequency = 20000; //PWM周波数 20kHz
  int _pin[8]; //モーターのピン Pins.hの内容コピー
  const int _angle[4] = {-45, 45, 135, -135};  // モーター取り付け角度
  const float _corr[4] = {1, -1, -1, -1};       // 回転補正
};

class MOVE {
 public:
  int dir;    // 進む方向
  int speed;  // スピード
  int rot;    // 回転速度
  int kickdir=0; //キックする方向
  void carryball(int balldir);
  void carryball(int balldir,int balldistance);
  void carryball(Sensors& s);
  void avoid_line(const Sensors& s); //ラインの回避行動

  int toGoalPING(const Sensors& s);
 private:
};

class PID{
 public:
  int run(double target,double value);  // 姿勢制御
  int run(double a);  // 姿勢制御
  // int run(double a,double vel);  // 姿勢制御
  double stack = 0.0;  //積分量

  void setgain(float newKp,float newKi,float newKd){
    if(newKp>0)this->Kp=newKp;
    if(newKi>0)this->Ki=newKi;
    if(newKd>0)this->Kd=newKd;
  }
  float Kp = 1.5, Ki = 1.0, Kd = 1.0;  // PID制御係数
  double get_v(){return v;}
  double P,I,D;
 private:
  
  double b; //前回の値
  double da;  //変化量
  double v; //時間変化率

  short sign = 0;
  
  unsigned long t0, t1, dt;  //[us]
};


void kicker(bool out);

#endif



// motor.set_power(50,0,0,0);
//   motor.pwm_out();
//   delay(1000);

//   motor.stop();
//   motor.pwm_out();
//   delay(500);

//   motor.set_power(0,50,0,0);
//   motor.pwm_out();
//   delay(1000);

//   motor.stop();
//   motor.pwm_out();
//   delay(500);

//   motor.set_power(0,0,50,0);
//   motor.pwm_out();
//   delay(1000);

//   motor.stop();
//   motor.pwm_out();
//   delay(500);

//   motor.set_power(0,0,0,50);
//   motor.pwm_out();
//   delay(1000);

//   motor.stop();
//   motor.pwm_out();
//   delay(500);