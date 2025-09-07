#include "move.h"

#include "device.h"

MOTOR::MOTOR() {             // 初期化処理
  analogWriteResolution(8);  // 8bit = 0〜255
  for (int i = 0; i < 8; i++) {
    _pin[i] = Pin_motor[i];
    analogWriteFrequency(_pin[i], _frequency);  // 周波数設定
    analogWrite(_pin[i], 0);                    // 初期値 = OFF
  }
}

void MOTOR::cal_power(int dir,
                      int speed) {  // モーター出力を計算(進行方向, スピード)
  for (int i = 0; i < 4; i++) {
    m_speed[i] = (int)(sin((dir - _angle[i]) / 57.3) * speed);
    if (dir == 1000) m_speed[i] = 0;
  }

  // 最大値をspeedに
  int maxval = 0;
  if (dir != 1000 && speed != 0) {
    for (int i = 0; i < 4; i++) {
      if (abs(m_speed[i]) > maxval) maxval = abs(m_speed[i]);
    }
    if (maxval != 0)
      for (int i = 0; i < 4; i++)
        m_speed[i] = (int)(m_speed[i] * ((float)speed / maxval));
  }
}

void MOTOR::cal_power(
    int dir, int speed,
    int rot) {  // モーター出力を計算 移動と同時に回転(進行方向, スピード,
                // 回転速度)
  for (int i = 0; i < 4; i++) {
    m_speed[i] = (int)(sin((dir - _angle[i]) / 57.3) * speed);
    if (dir == 1000) m_speed[i] = 0;
  }
  // 最大値をspeedに
  int maxval = 0;
  if (dir != 1000 && speed != 0) {
    for (int i = 0; i < 4; i++) {
      if (abs(m_speed[i]) > maxval) maxval = abs(m_speed[i]);
    }
    if (maxval != 0)
      for (int i = 0; i < 4; i++)
        m_speed[i] = (int)(m_speed[i] * ((float)speed / maxval));
  }

  for (int i = 0; i < 4; i++) {
    m_speed[i] += rot;
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
      if (pwm == 0 && !brake) {
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

void MOVE::carryball(int balldir, int balldistance) {
  if (balldir <= 5 && balldir >= -5) {
    this->dir = 0;
  } else if (balldir <= 15 && balldir >= -15) {
    if (balldir > 0) {
      this->dir = balldir + 10;
    } else {
      this->dir = balldir - 10;
    }
  } else {
    int a;
    int distance_th = 3000;
    if (balldir <= 30 && balldir >= -30) {
      if (balldistance < distance_th) {
        a = 50;
        this->speed = 60;
      } else
        a = 15;
    } else if (balldir <= 60 && balldir >= -60) {
      if (balldistance < distance_th) {
        a = 60;
        this->speed = 60;
      } else
        a = 30;
    } else {
      a = 80;
      if (balldistance >= distance_th) a = 45;
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
  } else if (balldir <= 13 && balldir >= -13) {
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
  if (!ball.isExist) {
    this->speed = 0;
    this->dir = 1000;
    return;
  }
  int balldir = ball.dir;
  int balldistance = ball.distance;
  if (balldir <= 5 && balldir >= -5) {
    this->dir = 0;
  } else if (balldir <= 15 && balldir >= -15) {
    if (balldir > 0) {
      this->dir = balldir + 10;
    } else {
      this->dir = balldir - 10;
    }
  } else {
    int a;
    int distance_th = 3200;
    if (balldir <= 30 && balldir >= -30) {
      if (balldistance < distance_th) {
        a = 50;
        this->speed = 80;
        if(ball.distance<200)this->speed = 50;
      } else
        a = 15;
    } else if (balldir <= 60 && balldir >= -60) {
      if (balldistance < distance_th) {
        a = 60;
        this->speed = 80;
        if(ball.distance<200)this->speed = 50;
      } else
        a = 30;
    } else {
      a = 80;
      if (balldistance >= distance_th) a = 45;
    }
    if (s.line.Num_white > 0 && a > 50) {
      a = 50;
    }
    if (s.line.Num_white > 0 && a > 30 && abs(ball.dir) > 90) {
      a = 30;
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
  // BALL& ball = s.ball;
  LINE& line = s.line;
  BNO& gyro = s.gyro;
  ULTRASONIC& ping = s.ping;
  if (dir_move == 1000) this->speed = 0;
  int speed = this->speed;

  if (line.Num_angel >= 2 ||
      line.isHalfout) {  // エンジェルライン2個以上反応あり
    if (line.isHalfout) {
      dir_move =
          line.dir;  // 半分以上外に出ているなら、フィールド内への復帰最優先
      speed = 90;
    } else {
      float x, y;
      x = -sin(radians(line.dir - dir_move + gyro.dir)) * speed;  // ラインに垂直な方向
      y = cos(radians(line.dir - dir_move + gyro.dir)) * speed;   //ラインに平行な方向

      if (line.lmax >= 8) {
        y = 50.0;
      } else {
        y = 80.0;
      }

      // if(y<0){
      //   if(line.lmax>=9 || (line.lmax>=6 && line.Num_angel>=4 && 0)){
      //     y=0.0;
      //   }else if(line.lmax>=6){
      //     y = 40.0;  // フィールド内に戻る強さ
      //   }else{
      //     y = 80.0;  // フィールド内に戻る強さ
      //   }
      //   if(line.Num_angel>=4)x=0.0;
      // }

      dir_move = degrees(atan2(x, y)) + line.dir -
                 gyro.dir;  // ※ (x,y)はline.dirを基準にした座標系のため
      speed = sqrt(x * x + y * y);
      if (abs(speed) < 20) speed = 0;
    }

  } else if (line.Num_white >
             0) {  // エンジェルライン反応なし、十字ライン反応あり
    float m_vector[4] = {0};                   // {前,右,後,左}
    float x = sin(radians(dir_move)) * speed,  // 進行方向をx,yに分解
        y = cos(radians(dir_move)) * speed;

    // 4つのベクトルに分解
    if (x > 0) {
      m_vector[1] = x;
    } else {
      m_vector[3] = -x;
    }
    if (y > 0) {
      m_vector[0] = y;
    } else {
      m_vector[2] = -y;
    }

    for (int i = 0; i < 4; i++) {
      // if(line.depth[i]){
      //   if(millis() - line.time_onLine > 200){
      //     m_vector[i] *= 0.5;  // 50%に減速
      //   }else{
      //     m_vector[i] = 0;
      //   }
      // }

      if (line.depth[i]) {
        if (line.depth[i] <= 1)       // 最も外側のラインが反応
          m_vector[i] *= 0.5;         // 50%に減速
        else if (line.depth[i] <= 2)  // 2番目に外側のラインが反応
          m_vector[i] *= 0.4;         // 40%に減速
        else if (line.depth[i] >= 4)  // 十字部分内側が反応
          m_vector[i] = -40;          // 少し回避
        else                          // 十字部分反応あり
          m_vector[i] = 0;  // ラインアウト方向への移動を制限
      }
    }
    if (line.depth[0] == 0 && ping.value[2]<35) {
      if (line.depth[1] && ping.value[1] > 60 && m_vector[0]<80) {
        m_vector[2] = -80;
      } else if (line.depth[3] && ping.value[0] > 60 && m_vector[0]<80) {
        m_vector[2] = -80;
      }
    }

    x = m_vector[1] - m_vector[3];  // m_vector4つを足し合わせる →
                                    // 新しい進行方向
    y = m_vector[0] - m_vector[2];
    dir_move = degrees(atan2(x, y));
    speed = sqrt(x * x + y * y);
    if (x * y == 0) speed *= 1.5;  // 少しスピードアップ
    if (speed > this->speed) speed = this->speed;
    if (abs(speed) < 20) speed = 0;  // 再計算後、速度が遅い場合は停止
  }

  this->dir = dir_move;
  this->speed = speed;
}

TIMER timer0;

int MOVE::toGoalPING(const Sensors& s){
  LINE& line = s.line;
  BALL& ball = s.ball;
  ULTRASONIC& ping = s.ping;
  int kickdir = 0;
  if (timer0.get() < 100000 && ball.isExist && abs(ball.dir)< 60 && line.Num_white==0) {  // ボール捕捉時
    if (ping.value[1] < 60 && ping.value[0] > 85) {        // ゴールは左  
      if(ping.value[2]>100 ||1) //敵陣
      kickdir = -30;
      else
        kickdir = -10;
    } else if (ping.value[1] > 85 && ping.value[0] < 60) {
      if(ping.value[2]>100 ||1) //敵陣
        kickdir = 30;
      else
        kickdir = 10;
    } else{

    }
  } else {
  }
  return kickdir;
}

int PID::run(double target,double value){
  if (target > 0) {
    if (sign == -1) stack = 0;
    sign = 1;
  } else if (target < 0) {
    if (sign == 1) stack = 0;
    sign = -1;
  }
  t1 = micros();
  dt = t1 - t0;
  if (dt >= 5000) {  // 5msごと
    t0 = t1;
    da = value - b;                                   // 今回-前回
    b = value;                                        // 前回の値
    if (abs(target) < 30) {                            // 偏差が小さいときのみ積分
      stack += Ki * target * (double)dt / 1000 / 100;  // Iのみ先に係数をかける
    }
    const double alpha = 0.0;  // 0.0～1.0
    
    double new_v = da / (double)dt * 1000.0 * 100.0;  // 微分

    v = v*alpha + (1-alpha)*new_v;
  }

  if (stack > 50.0)
    stack = 50.0;
  else if (stack < -50.0)
    stack = -50.0;
  if (abs(target) <= 0.2) stack = 0.0;  // 小さすぎる偏差は無視
  P=Kp * target;I=stack;D=Kd*v;
  return -(Kp * target + stack + Kd * v);
}

int PID::run(double a) {
  if (a > 0) {
    if (sign == -1) stack = 0;
    sign = 1;
  } else if (a < 0) {
    if (sign == 1) stack = 0;
    sign = -1;
  }
  t1 = micros();
  dt = t1 - t0;
  if (dt >= 5000) {  // 5msごと
    t0 = t1;
    da = a - b;                                   // 今回-前回
    b = a;                                        // 前回の値
    if (abs(a) < 30) {                            // 偏差が小さいときのみ積分
      stack += Ki * a * (double)dt / 1000 / 100;  // Iのみ先に係数をかける
    }
    const double alpha = 0.0;  // 0.0～1.0
    
    double new_v = da / (double)dt * 1000.0 * 100.0;  // 微分

    v = v*alpha + (1-alpha)*new_v;
  }

  if (stack > 50.0)
    stack = 50.0;
  else if (stack < -50.0)
    stack = -50.0;
  if (abs(a) <= 0.2) stack = 0.0;  // 小さすぎる偏差は無視
  P=Kp * a;I=stack;D=Kd*v;
  return -(Kp * a + stack + Kd * v);
}

// int PID::run(double a,double vel) {
//   if (a > 0) {
//     if (sign == -1) stack = 0;
//     sign = 1;
//   } else if (a < 0) {
//     if (sign == 1) stack = 0;
//     sign = -1;
//   }
//   t1 = micros();
//   dt = t1 - t0;
//   if (dt >= 5000) {  // 5msごと
//     t0 = t1;
//     da = a - b;                                   // 今回-前回
//     b = a;                                        // 前回の値
//     if (abs(a) < 30) {                            // 偏差が小さいときのみ積分
//       stack += Ki * a * (double)dt / 1000 / 100;  // Iのみ先に係数をかける
//     }
//     const double alpha = 0;  // 0.0～1.0
    

//   }
//   v=vel/10.0;

//   if (stack > 50.0)
//     stack = 50.0;
//   else if (stack < -50.0)
//     stack = -50.0;
//   if (abs(a) <= 0.2) stack = 0.0;  // 小さすぎる偏差は無視
//   P=Kp * a;I=stack;D=Kd*v;
//   return -(Kp * a + stack + Kd * v);
// }

void kicker(bool out) { digitalWrite(Pin_kicker, out); }