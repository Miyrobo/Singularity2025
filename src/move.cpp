#include "move.h"
#include "device.h"

void MOTOR::setup(){
  analogWriteResolution(8); // 8bit = 0〜255
  for (int i = 0; i < 8; i++) {
    analogWriteFrequency(_pin[i], _frequency); // 周波数設定
    analogWrite(_pin[i], 0); // 初期値 = OFF
  }
}

void MOTOR::cal_power(int dir, int speed) {  //モーター出力を計算(進行方向, スピード)
  for (int i = 0; i < 4; i++) {
    m_speed[i] = (int)(sin((dir - _angle[i]) / 57.3) * speed);
    if(dir==1000)m_speed[i] = 0;
  }
}

void MOTOR::cal_power(int dir, int speed, int rot) { //モーター出力を計算 移動と同時に回転(進行方向, スピード, 回転速度)
  for (int i = 0; i < 4; i++) {
    m_speed[i] = (int)(sin((dir - _angle[i]) / 57.3) * speed) + rot;
    if(dir==1000)m_speed[i] = rot;
  }
  if(dir==0 && speed>=80 && abs(rot)<5){
    for (int i = 0; i < 4; i++) {
      m_speed[i] = (int)(sin((0 - _angle[i]) / 57.3) * speed);
    }
  }
}

void MOTOR::stop() {
  for (int i = 0; i < 4; i++) {
    m_speed[i] = 0;
  }
}

void MOTOR::pwm_out() {
  int pwm;
  for (int i = 0; i < 4; i++) {
    pwm = m_speed[i] * _corr[i] * 2.55;
    if (pwm > 0) {
      if (pwm >= 50 && 0) {
        if (pwm > 255) pwm = 255;
        analogWrite(_pin[i * 2], 0);
        analogWrite(_pin[i * 2 + 1], pwm);
      } else {
        if (pwm > 255) pwm = 255;
        analogWrite(_pin[i * 2], 255 - pwm);
        analogWrite(_pin[i * 2 + 1], 255);
      }

    } else {
      pwm = -pwm;
      if (pwm==0 && !brake) {
        if (pwm > 255) pwm = 255;
        analogWrite(_pin[i * 2], pwm);
        analogWrite(_pin[i * 2 + 1], 0);
      } else {
        if (pwm > 255) pwm = 255;
        analogWrite(_pin[i * 2], 255);
        analogWrite(_pin[i * 2 + 1], 255 - pwm);
      }
    }
  }
}

void MOTOR::set_power(int m1, int m2, int m3, int m4) {
  m_speed[0] = m1;
  m_speed[1] = m2;
  m_speed[2] = m3;
  m_speed[3] = m4;
}

void MOVE::carryball(int balldir,int balldistance) {
  if (balldir <= 5 && balldir >= -5) {
    this->dir = 0;
  }else if(balldir <= 15 && balldir >= -15){
    if(balldir>0){
      this->dir = balldir + 10;
    }else{
      this->dir = balldir - 10;
    }
  } else {
    int a;
    int distance_th =3000;
    if (balldir <= 30 && balldir >= -30) {
      if(balldistance < distance_th){
        a = 50;
        this->speed = 40;
      }else a=15;
    } else if (balldir <= 60 && balldir >= -60) {
      if(balldistance < distance_th){
        a = 60;
        this->speed=40;
      }else a=30;
    } else {
      a = 80;
      if(balldistance>=distance_th)a=45;
    }
    if (balldir > 0) {
      this->dir = balldir + a;
    } else {
      this->dir = balldir - a;
    }
  }
}

void MOVE::carryball(int balldir) {
  if (balldir <= 5 && balldir >= -5) {
    this->dir = 0;
  }else if(balldir <= 13 && balldir >= -13){
    this->dir = balldir;
  } else {
    int a;
    if (balldir <= 30 && balldir >= -30) {
      a = 20;
      this->speed = 40;
    } else if (balldir <= 60 && balldir >= -60) {
      a = 55;
    } else {
      a = 70;
    }
    if (balldir > 0) {
      this->dir = balldir + a;
    } else {
      this->dir = balldir - a;
    }
  }
}

int PID::run(double a) {
  if(a>0){
    if(sign==-1)stack=0;
    sign=1;
  }else if(a<0){
    if(sign==1)stack=0;
    sign=-1;
  }
  t1 = micros();
  dt = t1 - t0;
  if(dt>=5000){ //5msごと
    t0 = t1;
    da = a - b; //今回-前回
    b = a;  // 前回の値
    if(abs(a)<30){ //偏差が小さいときのみ積分
      stack += Ki * a * (double)dt / 1000 / 100; //Iのみ先に係数をかける
    }
    v = da / (double)dt * 1000 * 100; //微分
  }
  
  if (stack > 50.0)
    stack = 50.0;
  else if (stack < -50.0)
    stack = -50.0;
  if(abs(a)<0.5) stack=0.0; //小さすぎる偏差は無視
  return -(Kp * a + stack + Kd * v);
}

void kicker(bool out) { digitalWrite(Pin_kicker, out); }