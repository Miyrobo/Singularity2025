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
    int distance_th = 3000;
    if (balldir <= 30 && balldir >= -30) {
      if (balldistance < distance_th) {
        a = 50;
        this->speed = 80;
      } else
        a = 15;
    } else if (balldir <= 60 && balldir >= -60) {
      if (balldistance < distance_th) {
        a = 60;
        this->speed = 80;
      } else
        a = 30;
    } else {
      a = 80;
      if (balldistance >= distance_th) a = 45;
    }
    if (s.line.Num_white > 0 && a > 50) {
      a = 50;
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

  if (line.dir != 1000 && line.Num_angel>1 || line.isHalfout) {  // エンジェルライン2個以上反応あり
    if (line.isHalfout) {
      dir_move=line.dir;
      speed=80;
    } else {
      float x, y;
      if (dir_move == 1000) speed = 0;
      x = -sin(radians(line.dir - dir_move)) * speed;
      y = cos(radians(line.dir - dir_move)) * speed;

      y = 0;

      y = 80.0;  // ライン内側に戻る強さ

      dir_move = degrees(atan2(x, y)) + line.dir - gyro.dir;
      speed = sqrt(x * x + y * y);
      if (abs(speed) < 20) speed = 0;
    }

  } else if (line.Num_white > 0) {
    float m_vector[4] = {0};
    float x = sin(radians(dir_move)) * speed,
          y = cos(radians(dir_move)) * speed;
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
      if (line.depth[i]) {
        if (line.depth[i] <= 1)
          m_vector[i] *= 0.3;  // 30%に減速
        else if (line.depth[i] >= 4)
          m_vector[i] = -40;  // 少し回避
        else
          m_vector[i] = 0;
      }
    }

    x = m_vector[1] - m_vector[3];
    y = m_vector[0] - m_vector[2];
    dir_move = degrees(atan2(x, y));
    speed = sqrt(x * x + y * y);
    if (x * y == 0) speed *= 1.5;
    if (abs(speed) < 20) speed = 0;
  }

  this->dir = dir_move;
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
    da = a - b;         // 今回-前回
    b = a;              // 前回の値
    if (abs(a) < 30) {  // 偏差が小さいときのみ積分
      stack += Ki * a * (double)dt / 1000 / 100;  // Iのみ先に係数をかける
    }
    v = da / (double)dt * 1000 * 100;  // 微分
  }

  if (stack > 50.0)
    stack = 50.0;
  else if (stack < -50.0)
    stack = -50.0;
  if (abs(a) < 0.5) stack = 0.0;  // 小さすぎる偏差は無視
  return -(Kp * a + stack + Kd * v);
}

void kicker(bool out) { digitalWrite(Pin_kicker, out); }