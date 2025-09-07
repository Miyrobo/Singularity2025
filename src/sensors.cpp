#include "sensors.h"

#include "Arduino.h"
#include "EEPROM.h"
#include "Pins.h"
#include "device.h"

// HardwareSerial Serial_arduino(Serial2);   //サブマイコン用のUARTの番号

Adafruit_BNO055 bno055 = Adafruit_BNO055(-1, 0x28, &BNO_I2C);

#define OPENMV Serial4

void BALL::get() {  // ボールの位置取得
  x = 0;
  y = 0;
  num = 0;
  max = 0;
  maxn = -1;
  total = 0;

  for (int i = 0; i < 8; i++) {
    if (i >= 4) {
      digitalWrite(Pin_MUX3, 1);
    } else {
      digitalWrite(Pin_MUX3, 0);
    }
    if (i % 4 >= 2) {
      digitalWrite(Pin_MUX2, 1);
    } else {
      digitalWrite(Pin_MUX2, 0);
    }
    if (i % 2 > 0) {
      digitalWrite(Pin_MUX1, 1);
    } else {
      digitalWrite(Pin_MUX1, 0);
    }
    //delayMicroseconds(10);
    value[i] = analogRead(Pin_Ball1);
    total += 1023 - value[i];
    value[i + 8] = analogRead(Pin_Ball2);
    total += 1023 - value[i + 8];
    // delayMicroseconds(1);
  }

  for (int i = 0; i < NUM_balls; i++) {
    int v = value[i];
    if (v < _th) {
      value[i] = _th - v;
      num++;
      if (value[i] > max) {
        max = value[i];
        maxn = i;
      }
    } else {
      value[i] = 0;
    }
    x = x + (SIN16_1000[i] * ((double)value[i] / 1000.0));
    y = y + (SIN16_1000[(i + 4) % 16] * ((double)value[i] / 1000.0));
  }

  // dir = atan2(x,y) * 57.3;
  distance = sqrt(x * x + y * y);
  distance = 0;
  if (num >= 1) {
    isExist = true;
    x = 0;
    y = 0;
    for (int i = 0; i < 16; i++) {
      if (abs(maxn - i) <= 2 || abs(maxn - i) >= 14) {  // 最大 & 2つ隣まで
        x += (SIN16_1000[i % 16] * ((double)value[i % 16] / 1000.0));
        y += (SIN16_1000[(i + 4) % 16] * ((double)value[i % 16] / 1000.0));
        if (maxn == i)
          ;
        distance += (value[i % 16] > _th
                         ? 0
                         : _th - value[i % 16]);  // 最大のセンサで距離を求める
      }
    }
    // distance-=1500;
    if (distance < 0) distance = 0;

    dir = atan2(x, y) * 57.3;
    // Serial.println(dir);
  } else {
    isExist = false;
    dir = 1000;
    distance = -1;
  }

#ifdef ball_debug
  if (Serial.read() == 'B' || 1) {
    Serial.print('B');
    for (int i = 0; i < NUM_balls; i++) {
      Serial.print(value[i] / 4);
      Serial.print(',');
    }
    Serial.print(dir);
    Serial.print(',');
    Serial.print((int)distance);
    Serial.print(',');
    Serial.print('\n');
    // Serial.println("");
    // Serial.print("  ");
    // Serial.print(x);
    // Serial.print("  ");
    // Serial.println(y);
  }
#endif
}

//---------------------------------------------------------------------------------------------
void BNO::setup() {
  bno055.begin();
  BNO_I2C.setClock(400000);  // 400kHzに設定
  bno055.getTemp();
  bno055.setExtCrystalUse(true);
  reset();
}
//---------------------------------------------------------------------------------------------
void BNO::get() {
  imu::Vector<3> euler = bno055.getVector(Adafruit_BNO055::VECTOR_EULER);
  //imu::Vector<3> gyro = bno055.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  
  ypr[0] = euler.x();
  ypr[1] = euler.y();
  ypr[2] = euler.z();
  //vel=-gyro.z();
  dir = ypr[0] - dir0;
  if (dir > 180)
    dir -= 360;
  else if (dir < -179)
    dir += 360;
}
void BNO::getvel(){
  imu::Vector<3> accel = bno055.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  
  // 前回からの経過時間を計算
  static unsigned long lastTime = millis();
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0f; // [s]
  lastTime = now;

  // 加速度をメンバ変数に格納
  ax = accel.x();  
  ay = accel.y();

  if(abs(ax)<0.05)ax=0;
  if(abs(ay)<0.05)ay=0;

  // 積分して速度を更新
  vx -= ax * dt;
  vy -= ay * dt;

  // ドリフト対策（ほんの少し減衰をかける）
  //if(abs(ax)<0.1)
    vx *= 0.98;
  //if(abs(ay)<0.1)
    vy *= 0.98;


}
//---------------------------------------------------------------------------------------------
void BNO::reset() {  // 攻め方向リセット
  this->get();
  dir0 = ypr[0];
}
//---------------------------------------------------------------------------------------------
void BNO::updateCalibration() {
  bno055.getCalibration(&cal_sys, &cal_gyro, &cal_accel, &cal_mag);
}

//=============================================================================================
LINE::LINE() {  // 閾値
  for (int i = 0; i < 4; i++) {
    pinMode(_pin[i], INPUT);  // ピン設定
  }
  for (int i = 0; i < 32; i++) {
    _th[i] = EEPROM.read(i) * 4;
  }
}
//---------------------------------------------------------------------------------------------
void LINE::get_value() {
  for (int i = 0; i < 8; i++) {
    if (i >= 4) {
      digitalWrite(Pin_MUX3, 1);
    } else {
      digitalWrite(Pin_MUX3, 0);
    }
    if (i % 4 >= 2) {
      digitalWrite(Pin_MUX2, 1);
    } else {
      digitalWrite(Pin_MUX2, 0);
    }
    if (i % 2 > 0) {
      digitalWrite(Pin_MUX1, 1);
    } else {
      digitalWrite(Pin_MUX1, 0);
    }
    //delayMicroseconds(10);
    float rate=0.0;
    value32[i] = value32[i]*rate+(1-rate)*analogRead(Pin_Line1);
    value32[i + 8] = value32[i + 8]*rate+(1-rate)*analogRead(Pin_Line2);
    value32[i + 16] = value32[i + 16]*rate+(1-rate)*analogRead(Pin_Line3);
    value32[i + 24] = value32[i + 24]*rate+(1-rate)*analogRead(Pin_Line4);
    // delayMicroseconds(1);
  }
  Num_white = 0;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      value[i][j] = value32[i * 8 + j];
      if (value[i][j] > _th[i * 8 + j]) {
        s[i][j] = true;
        Num_white++;
      } else {
        s[i][j] = false;
      }
    }
  }
  if (Num_white == 0 && !isHalfout) this->time_onLine = millis();
  // エンジェル
  for (int i = 0; i < 4; i++) {
    s_angel[(i * 3 + 11) % 12] = s[i][5];
    s_angel[i * 3 + 0] = s[i][6];
    s_angel[i * 3 + 1] = s[i][7];
    value_angel[(i * 3 + 11) % 12] = value[i][5];
    value_angel[i * 3 + 0] = value[i][6];
    value_angel[i * 3 + 1] = value[i][7];
  }
}
//---------------------------------------------------------------------------------------------
int LINE::check(Sensors& c_sensors) {
  // ラインの侵入深さチェック
  for (int i = 0; i < 4; i++) {
    if (s[i][5] + s[i][6] + s[i][7])  // エンジェル部分反応
      depth[i] = 5;
    else if (s[i][4])
      depth[i] = 4;
    else if (s[i][3])
      depth[i] = 3;
    else if (s[i][2])
      depth[i] = 2;
    else if (s[i][1] + s[i][0])
      depth[i] = 1;
    else
      depth[i] = 0;
  }

  // エンジェルチェック
  if (check_angel()) {  // エンジェル反応ありなら
    sdir = rawdir - c_sensors.gyro.dir;
    if (lmax >= 5 || dir == 1000) {
      int diff = ((mem_linedir - sdir + 540) % 360) - 180;

      if (abs(diff) > 100 && mem_linedir != 1000) {  // 前回との差が100°以上
        isHalfout = !isHalfout;                      // 半分以上外に出たと判定
        // tone(buzzer,2000,50);
      }

      if (isHalfout) {
        dir = sdir + 180;
      } else {
        dir = sdir;
      }
      if (dir > 180) dir -= 360;
      mem_linedir = sdir;
    } else {                                        // もっとも広い間隔が4以下
      int diff = ((dir - sdir + 540) % 360) - 180;  //
      if (abs(diff) < 90) {
        dir = sdir;
      } else {
        dir = (sdir + 180) % 360;
      }
    }
  } else {
    sdir = 1000;
    if (!isHalfout) {
      dir = 1000;
    }
    mem_linedir = 1000;
  }

  if (Num_white > 0) {
    return 1;
  }
  return 0;
}
//---------------------------------------------------------------------------------------------
int LINE::check_angel() {
  Num_angel = 0;  // 反応数カウント
  for (int i = 0; i < 12; i++) {
    if (s_angel[i]) {
      Num_angel++;
    }
  }

  if (Num_angel > 0 && Num_angel < 12) {
    float angle;
    findLongestZeroGapWithAngle(s_angel, angle,
                                lmax);  // もっとも広い反応していない間隔を返す
    rawdir = angle;
  } else {
    rawdir = 1000;
    return 0;
  }
  return Num_angel;
}
//---------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------
void LINE::LEDset(int s = -1) {  // ラインのLED操作
  if (s == -1) {
    s = this->_LED;
  }
  digitalWrite(ledpin, s);
}
//---------------------------------------------------------------------------------------------

//=============================================================================================
int ULTRASONIC::get(int n) {
  // ピンをOUTPUTに設定（パルス送信のため）
  pinMode(trig_pin[n], OUTPUT);
  // LOWパルスを送信
  digitalWrite(trig_pin[n], LOW);
  delayMicroseconds(2);
  // HIGHパルスを送信
  digitalWrite(trig_pin[n], HIGH);
  // 5uSパルスを送信してPingSensorを起動
  delayMicroseconds(5);
  digitalWrite(trig_pin[n], LOW);

  // 入力パルスを読み取るためにデジタルピンをINPUTに変更（シグナルピンを入力に切り替え）
  pinMode(echo_pin[n], INPUT);

  // 入力パルスの長さを測定
  int duration = pulseIn(echo_pin[n], HIGH, timeout);  // 応答がなかったら0

  // パルスの長さを半分に分割
  duration = duration / 2;
  // cmに変換
  value[n] = int(duration / 29);
  last_update[n] = millis();
  return value[n];
}

void ULTRASONIC::get_all() {
  for (int i = 0; i < 4; i++) {
    get(i);
  }
}

unsigned long TIMER::get() { return millis() - s_tim; }

void TIMER::reset() { s_tim = millis(); }

TIMER openmvtime;

void CAMERA::update() {
  // ヘッダ'S'を探す
  while (OpenMV_UART.available()) {
    if (OpenMV_UART.read() == 'S') {
      // 6バイト揃うまで待つ
      while (OpenMV_UART.available() < 6) {
      }
      uint8_t ox = OpenMV_UART.read();
      uint8_t oy = OpenMV_UART.read();
      uint8_t bx = OpenMV_UART.read();
      uint8_t by = OpenMV_UART.read();
      uint8_t yx = OpenMV_UART.read();
      uint8_t yy = OpenMV_UART.read();

      orange = {ox - center_x / 2, oy - center_y / 2, !(ox == 0 && oy == 0)};
      blue = {bx - center_x / 2, by - center_y / 2, !(bx == 0 && by == 0)};
      yellow = {yx - center_x / 2, yy - center_y / 2, !(yx == 0 && yy == 0)};

      if (orange.found) {
        orange.dir = atan2(orange.x, orange.y) * 57.2958f;
        orange.distance = sqrt(orange.x * orange.x + orange.y * orange.y);
      }
      if (blue.found) {
        blue.dir = atan2(blue.x, blue.y) * 57.2958f;
        blue.distance = sqrt(blue.x * blue.x + blue.y * blue.y);
      }
      if (yellow.found) {
        yellow.dir = atan2(yellow.x, yellow.y) * 57.2958f;
        yellow.distance = sqrt(yellow.x * yellow.x + yellow.y * yellow.y);
      }
      break;
    }
  }
}

void CAMERA::set_color(int c) { goal_color = c; }

void CAMERA::start() { OpenMV_UART.write('1'); }

void CAMERA::stop() { OpenMV_UART.write('0'); }

void CAMERA::setup() {
  OpenMV_UART.begin(9600);
  start();
}

// 最も長い未検出(0)区間の中央角度を返す
// - centerAngleDeg: 結果格納（°）
// - 戻り値: 0=正常, 1=全検出, 2=全未検出
int findLongestZeroGapWithAngle(int arr[12], float& centerAngleDeg, int& MAX) {
  int sum = 0;
  MAX = 0;
  for (int i = 0; i < 12; i++) {
    sum += arr[i];
  }

  if (sum == 12) {
    // 全センサーが反応している → 全検出
    return 1;
  }
  if (sum == 0) {
    // 全センサーが反応していない → 未検出モード
    return 2;
  }

  // 環状なので配列を2倍にしてリングを展開
  int doubleArr[24];
  for (int i = 0; i < 24; i++) {
    doubleArr[i] = arr[i % 12];
  }

  // 最大の0の連続区間を探す
  int maxLen = 0, maxStart = 0;
  int currentLen = 0, currentStart = 0;

  for (int i = 0; i < 24; i++) {
    if (doubleArr[i] == 0) {
      if (currentLen == 0) currentStart = i;
      currentLen++;
      if (currentLen > maxLen && currentLen <= 12) {
        maxLen = currentLen;
        maxStart = currentStart;
      }
    } else {
      currentLen = 0;
    }
  }

  float midIdx = maxStart + (maxLen - 1) / 2.0;
  int angleIdx = ((int)round(midIdx)) % 12;
  centerAngleDeg = angleIdx * 30.0f;
  MAX = maxLen;

  return 0;  // 正常に検出
}