#ifndef SENSORS_H
#define SENSORS_H

//#define ball_debug

#include "Adafruit_BNO055.h"
#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"

#define Pin_S1 11
#define Pin_S2 12
#define Pin_S3 24

#define Pin_TS 21

#define SW1 !digitalRead(Pin_S1)
#define SW2 !digitalRead(Pin_S2)
#define SW3 !digitalRead(Pin_S3)

#define TS !digitalRead(Pin_TS)

#define Pin_ballcatch A6


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
  const byte pin[NUM_balls] = {A0,A1,A3,A5,A7,A9,A11,A13,A15,A14,A12,A10,A8,A6,A4,A2};  //ピン番号
  //const byte pin[NUM_balls] = {A0,A2,A4,A6,A8,A10,A12,A14,A15,A13,A11,A9,A7,A5,A3,A1};  //ピン番号
  const int _th = 800;      //反応限界
};

class LINE {
 public:
  bool isOnline = 0;   //ライン上か
  bool isHalfout = 0;  //半分以上外

  int goalchance_count=0;

  int dir = 1000;//コートの方向
  int sdir=1000;             //
  int x, y;            //位置
  void get_value();    //値取得
  int value32[NUM_lines];
  int value[4][NUM_lines / 4];
  void LEDset(int s);   //LED操作
  int value_angel[12];

  int Num_angel;//反応した個数 エンジェル
  int Num_white; //反応した個数

  int mem_linedir;

  void get();

  void begin(){
    for(int i=0;i<4;i++){
      pinMode(_pin[i],INPUT);
    }
  }

 private:
  bool _LED = true; //発光するか
  byte _pin[4] = {A0,A1,A2,A3};
  const byte ledpin = 13; //LED制御ピン
  int _th[NUM_lines]; //閾値
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
  double dir0;
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




int findLongestZeroGapWithAngle(int arr[12], float &centerAngleDeg,int &MAX);



#endif