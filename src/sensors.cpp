#include "sensors.h"
#include "Pins.h"
#include "device.h"

// HardwareSerial Serial_arduino(Serial2);   //サブマイコン用のUARTの番号

Adafruit_BNO055 bno055 = Adafruit_BNO055(-1, 0x28);

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
    //delayMicroseconds(1);
    value[i]=analogRead(Pin_Ball1);
    total+=1023-value[i];
    value[i+8]=analogRead(Pin_Ball2);
    total+=1023-value[i+8];
    //delayMicroseconds(1);
  }

  // Serial.print(value[0]);
  // Serial.print("  ");
  // Serial.print(value[4]);
  // Serial.print("  ");
  // Serial.print(value[8]);
  // Serial.print("  ");
  // Serial.print(value[12]);

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

  //dir = atan2(x,y) * 57.3;
  distance = sqrt(x * x + y * y);
  distance = 0;
  if (num >= 1) {
    isExist = true;
    x=0;y=0;
    //for(int i=maxn+15;i<=maxn+17;i++){
    for(int i=0;i<16;i++){
      if(abs(maxn-i)<=2 || abs(maxn-i)>=14){ //最大 & 2つ隣まで
        x+=(SIN16_1000[i%16] * ((double)value[i%16] / 1000.0));
        y+=(SIN16_1000[(i + 4) % 16] * ((double)value[i%16] / 1000.0));
        if(maxn==i);
          distance+=(value[i%16] > _th ? 0 : _th-value[i%16]); //最大のセンサで距離を求める
      }
    }
    //distance-=1500;
    if(distance<0)distance=0;

    dir = atan2(x,y) * 57.3;
    //Serial.println(dir);
  } else {
    isExist = false;
    dir=1000;
    distance = -1;
  }

  // if (maxn >= 0) {
  //   dir = maxn * 360 / NUM_balls;
  //   if(dir > 180)dir-=360;
  // } else {
  //   dir = 1000;
  // }

#ifdef ball_debug
if(Serial.read()=='B' || 1){
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

void BNO::setup() {
  bno055.begin();
  Wire.setClock(400000);  // 400kHzに設定
  bno055.getTemp();
  bno055.setExtCrystalUse(true);
  reset();
}

void BNO::get() {
  imu::Vector<3> euler = bno055.getVector(Adafruit_BNO055::VECTOR_EULER);
  ypr[0] = euler.x();
  ypr[1] = euler.y();
  ypr[2] = euler.z();
  dir = ypr[0] - dir0;
  if (dir > 180)
    dir -= 360;
  else if (dir < -179)
    dir += 360;
}

void BNO::reset() {  // 攻め方向リセット
  this->get();
  dir0 = ypr[0];
}

void BNO::updateCalibration() {
  bno055.getCalibration(&cal_sys, &cal_gyro, &cal_accel, &cal_mag);
}

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
    delayMicroseconds(1);
    value32[i] = analogRead(A0);
    value32[i+8] = analogRead(A1);
    value32[i+16] = analogRead(A2);
    value32[i+24] = analogRead(A3);
    //delayMicroseconds(1);
  }

  for(int i=0;i<4;i++){
    for(int j=0;j<8;j++){
      value[i][j] = value32[i*8 + j];
    }
  }
}

void LINE::LEDset(int s = -1) {  // ラインのLED操作
  if (s == -1) {
    s = this->_LED;
  }
  digitalWrite(ledpin, s);
}

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
  int duration = pulseIn(echo_pin[n], HIGH,timeout);  //応答がなかったら0

  // パルスの長さを半分に分割
  duration = duration / 2;
  // cmに変換
  value[n] = int(duration / 29);
  return value[n];
}

void ULTRASONIC::get_all() {
  for(int i=0;i<4;i++){
    get(i);
  }
}

unsigned long TIMER::get(){
  return millis()-s_tim;
}

void TIMER::reset(){
  s_tim=millis();
}

TIMER openmvtime;

int CAMERA::get_goal(){
  // if(goal_color == OPENMV_YELLOW){
  //   OPENMV.write('y');
  // }else{
  //   OPENMV.write('b');
  // }
  // openmvtime.reset();
  // while(!Serial.available()){
  //   if(openmvtime.get()>OPENMV_timeout){
  //     opp_goal = 1000;
  //     return 1000;
  //   }
  // } // データが来るまで待つ

  // int C=OPENMV.read();

  // if(C==OPENMV_NOTFOUND){
  //   opp_goal = 1000;
  // }else{
  //   opp_goal = C*2;
  // }
  // return opp_goal;

  if(OPENMV.available()){
    return OPENMV.read();
  }
}


void CAMERA::getorangeball(){
  while (OPENMV.available()) {
    char header = OPENMV.read();
    if (header == 'S') {
        // 受信待ち
        while (OPENMV.available() < 2) delay(1);
        orangeX = OPENMV.read()-86;
        orangeY = OPENMV.read()-60;
        orangeDetected = true;
        orangedir = atan2(orangeX, orangeY) * 57.3;
        orangedistance = sqrt(orangeX*orangeX + orangeY*orangeY);
        ocount=0; //カウントリセット
        break;  // ループから抜ける
    }
    else if (header == 'N') {
      ocount++;
      if(ocount<3){

      }else{
        orangeDetected = false;
        orangeX = 0;
        orangeY = 0;
        orangedir = 1000;
      }
        
      break;
    }
    // 'S'でも'N'でもないバイトは破棄してループ継続
}


}

void CAMERA::set_color(int c){
  goal_color=c;
}

void CAMERA::begin(){
  OPENMV.begin(9600);
}


PUSHSWITCH::PUSHSWITCH(int pinno) {
  pin = pinno;
  pinMode(pin, INPUT);
}

void PUSHSWITCH::update() {
  bool currentRead = read();

  if (currentRead != lastReadState) {
    lastDebounceTime = millis();  // 状態が変化 → タイマーリセット
  }

  if ((millis() - lastDebounceTime) >= debounceDelay) {
    if (currentRead != lastStableState) {
      lastStableState = currentRead;  // 状態確定
    }
  }

  lastReadState = currentRead;
}

bool PUSHSWITCH::read() {
  return digitalRead(pin);
}

bool PUSHSWITCH::pushed() {
  update();

  if (lastReportedState == 1 && lastStableState == 0) {
    lastReportedState = 0;
    return true;  // 押された瞬間
  }

  if (lastStableState == 1) {
    lastReportedState = 1;
  }

  return false;
}

// Created with the help of ChatGPT (OpenAI) - 2025/07/20
// Function to find the longest gap of zeros and return center angle
// 最も長い未検出(0)区間の中央角度を返す
// - centerAngleDeg: 結果格納（°）
// - 戻り値: 0=正常, 1=全検出, 2=全未検出
int findLongestZeroGapWithAngle(int arr[12], float &centerAngleDeg,int &MAX) {
  int sum = 0;
  MAX=0;
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
  MAX=maxLen;

  return 0; // 正常に検出
}