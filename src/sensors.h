#ifndef SENSORS_H
#define SENSORS_H

//#define ball_debug

#include "Adafruit_BNO055.h"
#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"
#include "Pins.h"
//#inclue "device.h" は相互依存のためできない

#pragma once
struct Actuator; // 構造体 Actuator の前方宣言
struct Sensors;


#define SW1 !digitalRead(Pin_S1)
#define SW2 !digitalRead(Pin_S2)
#define SW3 !digitalRead(Pin_S3)

#define TS !digitalRead(Pin_TS)


#define NUM_balls 16
#define NUM_lines 32

class TIMER{
  public:
    void reset();
    unsigned long get();
  private:
    unsigned long s_tim;
};




const int SIN16_1000[16] = {0,   383,  707,  924,  1000,   924,
                          707, 383,  0, -383, -707, -924,
                          -1000, -924, -707, -383};

class BALL {
 public:
  int value[NUM_balls];
  void get();
  int dir;          //ボールの方向
  double distance;  //ボールまでの距離
  int x, y;
  int max;
  int maxn;
  int num;  //見えている数
  bool isExist;   //ボールがあるか
  int total; //合計
 private:
  const byte pin[NUM_balls] = {Pin_Ball1,Pin_Ball2};  //ピン番号
  const int _th = 800;      //反応限界
};

class LINE {
 public:
  LINE();
  //状態変数
  bool isOnline = 0;   //ライン上か
  bool isHalfout = 0;  //半分以上外
  int dir = 1000;               //コートの方向
  int rawdir=1000;             //センサー生データ　方向
  int sdir=1000;             //センサ生データ+傾き
  int x, y;            //位置
  int depth[4]; //どこまで深く侵入したか 0:反応なし, 5:エンジェル反応
  int s[4][NUM_lines / 4]; //反応あり→1, 反応なし→0
  int s_angel[12];
  int value32[NUM_lines];
  int value[4][NUM_lines / 4];
  int value_angel[12];
  int Num_angel;//反応した個数 エンジェル
  int Num_white; //反応した個数

  int lmax; //エンジェル　もっとも広い反応していない連続

  int mem_linedir;

  int goalchance_count=0;


  void get_value();    //値取得

  int check(Sensors& c_sensors);
  int check_angel();

  void avoid(Sensors& c_sensors,Actuator& c_act); //ライン回避

  void LEDset(int s);   //LED操作
  
  //void get();

  int _th[NUM_lines]; //閾値
 private:
  bool _LED = true; //発光するか
  byte _pin[4] = {Pin_Line1,Pin_Line2,Pin_Line3,Pin_Line4};
  const byte ledpin = Pin_LineLED; //LED制御ピン
  
};

class BNO {
 public:
  double ypr[3];
  void setup();
  void get();
  void reset();
  double dir;

  //キャリブレーション値の確認
  void updateCalibration();
  uint8_t cal_sys;
  uint8_t cal_gyro;
  uint8_t cal_accel;
  uint8_t cal_mag;

 private:
  double dir0; //攻め方向
};

class ULTRASONIC{
  public:
    int value[4];
    void get_all(); //全部測定
    int get(int n); //1つ測定
  private:
    const byte echo_pin[4]={28,27,29,26};  //前から時計回り
    const byte trig_pin[4]={28,27,29,26};
    unsigned long timeout = 20000;  //タイムアウト (us)
};

// class CAMERA {
//  public:
//   int x, y, h, w;
//   int color;
//   bool canSee;
//   void get();

// };

#define OPENMV_YELLOW 0
#define OPENMV_BLUE 1

class CAMERA{
  public:
    void begin();
    int get_goal();
    int get_own();
    int own_goal;
    int opp_goal;
    void set_color(int c);

    void getorangeball();
    int orangeX = -1;  // オレンジ色のx座標
    int orangeY = -1;  // オレンジ色のy座標
    int orangedir;
    int orangedistance;
    bool orangeDetected = false; // 検出フラグ

    int ocount=0;//見失ってからのカウント

  private:
    
    int goal_color;
    int OPENMV_timeout = 100;
    const int OPENMV_NOTFOUND = 255;
};



// 押しボタン用クラス
class PUSHSWITCH {
private:
  int pin;
  bool state = 0;
  bool lastStableState = 1;
  bool lastReadState = 1;
  unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 10; // ms
  bool lastReportedState = 1;

  void update();

public:
  PUSHSWITCH(int pinno);

  bool read();    // 安定化された現在の状態を返す
  bool pushed();  // 押された瞬間（エッジ）を検出
};

int findLongestZeroGapWithAngle(int arr[12], float &centerAngleDeg,int &MAX);



#endif