#include "move.h"
#include "device.h"

MOTOR::MOTOR(){ //初期化処理
  analogWriteResolution(8); // 8bit = 0〜255
  for (int i = 0; i < 8; i++) {
    _pin[i] = Pin_motor[i];
    analogWriteFrequency(_pin[i], _frequency); // 周波数設定
    analogWrite(_pin[i], 0); // 初期値 = OFF
  }
}

void MOTOR::cal_power(int dir, int speed) {  //モーター出力を計算(進行方向, スピード)
  for (int i = 0; i < 4; i++) {
    m_speed[i] = (int)(sin((dir - _angle[i]) / 57.3) * speed);
    if(dir==1000)m_speed[i] = 0;
  }

  //最大値をspeedに
  int maxval = 0;
  if(dir!=1000 && speed!=0){
    for(int i=0; i<4; i++){
      if(abs(m_speed[i])>maxval)
        maxval=abs(m_speed[i]);
    }
    if(maxval!=0)
      for(int i=0; i<4; i++)
        m_speed[i] = (int)(m_speed[i] * ((float)speed / maxval));
  }
}

void MOTOR::cal_power(int dir, int speed, int rot) { //モーター出力を計算 移動と同時に回転(進行方向, スピード, 回転速度)
  for (int i = 0; i < 4; i++) {
    m_speed[i] = (int)(sin((dir - _angle[i]) / 57.3) * speed);
    if(dir==1000)m_speed[i] = 0;
  }
  //最大値をspeedに
  int maxval = 0;
  if(dir!=1000 && speed!=0){
    for(int i=0; i<4; i++){
      if(abs(m_speed[i])>maxval)
        maxval=abs(m_speed[i]);
    }
    if(maxval!=0)
      for(int i=0; i<4; i++)
        m_speed[i] = (int)(m_speed[i] * ((float)speed / maxval));
  }
  
  for (int i = 0; i < 4; i++) {
    m_speed[i]+=rot;
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
        this->speed = 60;
      }else a=15;
    } else if (balldir <= 60 && balldir >= -60) {
      if(balldistance < distance_th){
        a = 60;
        this->speed=60;
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

void MOVE::carryball(Sensors& s) {
  BALL& ball = s.ball;
  int balldir = ball.dir;
  int balldistance = ball.distance;
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
        this->speed = 60;
      }else a=15;
    } else if (balldir <= 60 && balldir >= -60) {
      if(balldistance < distance_th){
        a = 60;
        this->speed=60;
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

void MOVE::avoid_line(const Sensors& s) {  // ライン回避
  int dir_move = this->dir;
  BALL& ball = s.ball;
  LINE& line = s.line;
  BNO& gyro = s.gyro;

  if(line.dir!=1000){
    dir_move = -line.dir-gyro.dir;
    speed=80;
    if(abs((speed-dir_move+360+180)%360)<90 && !line.isHalfout)
      speed = 50;
  }else
  if (dir_move != 1000 && 1) { //ライン十字部分処理
    if (line.depth[0]) {
      if(abs(ball.dir)<20 && ball.distance < 1500){
        line.goalchance_count++;
      }else{
        line.goalchance_count=0;
      }
      speed = 80;
      if (dir_move < 90 && dir_move > -90) {
        
        if (dir_move < 20 && dir_move > -20) {
          speed = 30;
          if(line.depth[0]>=2){
            dir_move = 1000;
          }
          if(line.depth[0]>=4){
            dir_move=180;
          }
        } else if (dir_move > 0) {
          if(dir_move < 30 && dir_move > -30) speed = 50;
          dir_move = 90;
        } else {
          if(dir_move < 30 && dir_move > -30) speed = 50;
          dir_move = -90;
        }
      }
      if(line.goalchance_count > 600){
        carryball(ball.dir);
        dir_move=this->dir;
        speed=50;
      }
    }else{
      //line.line.goalchance_count=0;
    }
    if (line.depth[1]) { //左
      speed = 80;
      if (dir_move < 0 && dir_move > -180) {
        if(ball.dir + gyro.dir > 0){
          dir_move = ball.dir;
        }else if (dir_move > -110 && dir_move < -70  && (abs(ball.dir) >= 25)) {
          speed = 30; //25/7/14 停止から変更
          if(line.depth[1]<=2){

          }else if(line.depth[1]>=4){
            dir_move = 90;
          }else{
            dir_move = 1000;
          }
        } else if (dir_move > -90 || abs(ball.dir) < 25) {
          if(dir_move > -120 && dir_move < -70){
            speed = 50;
          }
          dir_move=0;
          if(line.depth[1]<=2){
            dir_move=-10;
          }else if(line.depth[1]>=4){
            dir_move=10;
          }
          if(line.depth[0]){
            dir_move=1000;
          }
        } else {
          if(dir_move > -120 && dir_move < -70){
            speed = 50;
          }
          dir_move=180;
          if(line.depth[1]<=2){
            dir_move=-170;
          }else if(line.depth[1]>=4){
            dir_move=170;
          }
          if(line.depth[2]){
            dir_move=1000;
          }
        }
      }
    }
    if (line.depth[2]) {
      if (dir_move > 90 || dir_move < -90) {
        speed = 80;
        // if (dir_move > 150 || dir_move < -150) {
        //   dir_move = 1000;
        // } else if (dir_move > 0) {
        //   dir_move = 90;
        // } else {
        //   dir_move = -90;
        // }
        if(ball.dir < 90 && ball.dir > -90){
          if(ball.dir>0){
            dir_move = 90;
            if(line.depth[2]<=2){
              dir_move = 100;
            }else if(line.depth[2]>=4){
              dir_move = 80;
            }
            if(line.depth[3]){
              dir_move=1000;
            }
          }else{
            dir_move = -90;
            if(line.depth[2]<=2){
              dir_move = -100;
            }else if(line.depth[2]>=4){
              dir_move = -80;
            }
            if(line.depth[1]){
              dir_move=1000;
            }
          }
        }else if (ball.dir > 160 || ball.dir < -160) { //ボール真後ろ
          dir_move = 1000;
        } else if (ball.dir > 0) { //ボール右
          dir_move = 90;
          if(line.depth[2]<=2){
            dir_move = 100;
          }else if(line.depth[2]>=4){
            dir_move = 80;
          }
          if(line.depth[3]){
            dir_move=1000;
          }
        } else { //ボール左
          dir_move = -90;
          if(line.depth[2]<=2){
            dir_move = -100;
          }else if(line.depth[2]>=4){
            dir_move = -80;
          }
          if(line.depth[1]){
            dir_move=1000;
          }
        }
        
      }
    }
    if (line.depth[3]) {
      if (dir_move > 0 && dir_move < 180) {
        speed = 80;
        //dir_move=balldir+15; //裏技 ボールフィールド内なのに停止する事象の軽減
        if(ball.dir + gyro.dir < 0){
          dir_move = ball.dir;
        }else if (dir_move < 100 && dir_move > 80 && (abs(ball.dir) >= 25)) {
          speed = 30; //25/7/14 停止から変更
          if(line.depth[3]<=2){
            //速度だけ下げてそのままの動き
          }else if(line.depth[3]>=4){
            dir_move = -90;
          }else{
            dir_move = 1000;
          }
        } else if (dir_move < 90 || abs(ball.dir)<25) {
          dir_move = 0;
          if(line.depth[3]<=2){
            dir_move=10;
          }else if(line.depth[3]>=4){
            dir_move=-10;
          }
          if(line.depth[0]){
            dir_move=1000;
          }
        } else {
          dir_move = -180;
          if(line.depth[3]<=2){
            dir_move=170;
          }else if(line.depth[3]>=4){
            dir_move=-170;
          }
          if(line.depth[3]){
            dir_move=1000;
          }
        }
      }
    }
  }
  this->dir = dir_move;
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