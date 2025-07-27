#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <move.h>
#include <device.h>
#include <sensors.h>

#define MAX_Speed 100



#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define SCREEN_ADDRESS \
  0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire2, -1);


MOTOR motor;
BALL ball;
BNO gyro;
ULTRASONIC ping;
MOVE move;
CAMERA openmv;
PID pid;

TIMER timer[20];
TIMER linetim;
TIMER pingset;

LINE line;

TIMER timfps;
int fps=0;

int z = -1;  // ラインの戻る方向
int dir_move;
bool kick = 0;

int hold_th;

void sensormonitor();
int mode = 0;  // センサーモニターモード

int line_th[32];

void setup() {
  //ピン設定
  pinMode(Pin_MPA, OUTPUT);
  pinMode(Pin_MPB, OUTPUT);
  pinMode(Pin_MPC, OUTPUT);

  pinMode(Pin_kicker, OUTPUT);

  pinMode(A21, INPUT);
  pinMode(A22, INPUT);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  pinMode(A16, INPUT);
  pinMode(A17, INPUT);

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);

  pinMode(13, OUTPUT);

  pinMode(Pin_S1, INPUT);
  pinMode(Pin_S2, INPUT);
  pinMode(Pin_S3, INPUT);
  pinMode(Pin_TS, INPUT);

  pinMode(Pin_ballcatch, INPUT);

  pinMode(buzzer, OUTPUT);
  tone(buzzer, 2714, 120);
  motor.setup();
  openmv.begin();
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.println("Singularity");
  display.display();
  analogReadAveraging(5);

  Serial.begin(9600);

  motor.stop();
  motor.pwm_out();

  gyro.setup();



  for (int i = 0; i < 32; i++) {
    line_th[i] = EEPROM.read(i) * 4;
  }
  //EEPROM.write(34, 100);
  hold_th = EEPROM.read(34);

  timer[0].reset();
  //起動音
  tone(buzzer, 2714, 120);
  int i = 0;
  while (timer[0].get() < 300) {
    if (SW1 || SW2 || SW3) {
      noTone(buzzer);
      tone(buzzer, 2500,100);
      break;
    }
    if (timer[0].get() > 200 && i == 2) {
      noTone(buzzer);
      tone(buzzer, 3047);
      i++;
    } else if (timer[0].get() > 100 && i == 1) {
      noTone(buzzer);
      tone(buzzer, 2876);
      i++;
    } else if (i == 0) {
      noTone(buzzer);
      tone(buzzer, 2714);
      i++;
    }
  }
  noTone(buzzer);
  while (SW1 || SW2 || SW3)
    ;
}

//#define kadodassyutu//2025/6/22
TIMER timer_lift; //持ち上げ検出タイマ
void loop() {
  fps++;
  if(timfps.get()>1000){ //1秒間の処理速度表示
    display.clearDisplay();
    display.setCursor(40,25);
    display.setTextSize(4);
    display.print(fps);
    display.display();
    fps=0;
    timfps.reset();
  }
  motor.brake=false;
  sensormonitor();
  motor.brake=true;

//ライン読み取り
  //int line[4][8] = {0};
  int line_maxn[4]={0}; //反応してる中で最も内側
  line.get_value();

  int line_s[4] = {0};
  int line_n[4] = {0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      // if ((line[i][j] > 500 && j != 3) || line[i][j] > 850) {
      //   line_s[i] = 1;
      // } else if (j < 2 && line[i][j] > 230) {
      //   line_s[i] = 1;
      // }

      // if (i == 0 && 1) {
      //   Serial.print(line[i][j]);
      //   Serial.print("  ");
      // }
      if (line.value[i][j] > line_th[i * 8 + j]) {
        if(j>line_maxn[i]){
          line_maxn[i]=j;
        }
        line_s[i] = 1;
        if (j >= 6) { //エンジェル部分のみ
          line_n[i] = 1;
        }
      }
    }
  }


  if (pingset.get() > 200) { //超音波を200ms間隔で読み取り
    ping.get(0);
    ping.get(1);
    ping.get(2);
    pingset.reset();
  }

  if (line_s[0] + line_s[1] + line_s[2] + line_s[3]) {
    // tone(buzzer, 2000);
  } else {
    noTone(buzzer);
  }
  
  gyro.get();

  //オレンジボール
  openmv.getorangeball();
  ball.dir=openmv.orangedir;
  ball.isExist=openmv.orangeDetected;
  if(!ball.isExist)
    ball.get();

  if (ball.isExist) { //ボール見えた
    // int z;
    // if (ball.dir < 15 && ball.dir > -15) {
    //   dir_move = 0;
    // } else if (ball.dir > 0) {
    //   dir_move = ball.dir + 50;
    // } else {
    //   dir_move = ball.dir - 50;
    // }
    if(move.kickdir == 0){
      move.carryball(ball.dir + gyro.dir,ball.distance);
      dir_move=move.dir-gyro.dir;
    }else{
      move.carryball(ball.dir,ball.distance);
      dir_move=move.dir;
    }

    if (dir_move > 180)
      dir_move -= 360;
    else if (dir_move < -180)
      dir_move += 360;
    // motor.cal_power(dir_move, 60, -gyro.dir / 2);

    timer[13].reset();
  } else {
    dir_move = 1000;
    // motor.cal_power(0, 0, -gyro.dir / 2);
  }
  int speed = MAX_Speed;

  if(timer[13].get()>100){ //ボールが見えずに0.1秒経過
    // ping.get(0); //左
    // ping.get(1); //右
    // ping.get(2);

    int pingdiff = ping.value[0]-ping.value[1]; //右に行くほど+
    speed = 80;
    if(ping.value[2]>70){
      if(pingdiff > 20){
        dir_move = -135;
      }else if(pingdiff < -20){
        dir_move = 135;
      }else{
        dir_move=180;
      }
    }else{
      if(pingdiff > 20){
        dir_move = -90;
      }else if(pingdiff < -20){
        dir_move = 90;
      }else{
        dir_move=1000;
      }
    }
    
  }
  


  // if(ball1<ball2-100){
  //   motor.cal_power(90, 60, -gyro.dir / 2);
  // }else if(ball2<ball1-100){
  //   motor.cal_power(-90, 60, -gyro.dir / 2);
  // }else{
  //   motor.stop();
  // }
  
  if(abs(ball.dir)<15 && line.goalchance_count<240)line.goalchance_count++;
  line.goalchance_count=0; //昨日無効化
  if (dir_move != 1000 && 1) { //ライン十字部分処理
    if (line_s[0]) {
      if(abs(ball.dir)<20 && ball.distance < 1500){
        line.goalchance_count++;
      }else{
        line.goalchance_count=0;
      }
      speed = 80;
      if (dir_move < 90 && dir_move > -90) {
        
        if (dir_move < 20 && dir_move > -20) {
          speed = 30;
          if(line_maxn[0]>=2){
            dir_move = 1000;
          }
          if(line_maxn[0]>=4){
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
      if(line.goalchance_count > 250){
        move.carryball(ball.dir);
        dir_move=move.dir;
        speed=50;
      }
    }else{
      //line.goalchance_count=0;
    }
    if (line_s[1]) { //左
      speed = 80;
      if (dir_move < 0 && dir_move > -180) {
        if(ball.dir + gyro.dir > 0){
          dir_move = ball.dir;
        }else if (dir_move > -110 && dir_move < -70  && (abs(ball.dir) >= 25)) {
          speed = 30; //25/7/14 停止から変更
          if(line_maxn[1]<=2){

          }else if(line_maxn[1]>=4){
            dir_move = 90;
          }else{
            dir_move = 1000;
          }
        } else if (dir_move > -90 || abs(ball.dir) < 25) {
          if(dir_move > -120 && dir_move < -70){
            speed = 50;
          }
          dir_move=0;
          if(line_maxn[1]<=2){
            dir_move=-10;
          }else if(line_maxn[1]>=4){
            dir_move=10;
          }
          if(line_s[0]){
            dir_move=1000;
          }
        } else {
          if(dir_move > -120 && dir_move < -70){
            speed = 50;
          }
          dir_move=180;
          if(line_maxn[1]<=2){
            dir_move=-170;
          }else if(line_maxn[1]>=4){
            dir_move=170;
          }
          if(line_s[2]){
            dir_move=1000;
          }
        }
        #ifdef kadodassyutu
          if(ping.value[0]<450){
            dir_move = 0;
          }
        #endif
      }
    }
    if (line_s[2]) {
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
            if(line_maxn[2]<=2){
              dir_move = 100;
            }else if(line_maxn[2]>=4){
              dir_move = 80;
            }
            if(line_s[3]){
              dir_move=1000;
            }
          }else{
            dir_move = -90;
            if(line_maxn[2]<=2){
              dir_move = -100;
            }else if(line_maxn[2]>=4){
              dir_move = -80;
            }
            if(line_s[1]){
              dir_move=1000;
            }
          }
        }else if (ball.dir > 160 || ball.dir < -160) { //ボール真後ろ
          dir_move = 1000;
        } else if (ball.dir > 0) { //ボール右
          dir_move = 90;
          if(line_maxn[2]<=2){
            dir_move = 100;
          }else if(line_maxn[2]>=4){
            dir_move = 80;
          }
          if(line_s[3]){
            dir_move=1000;
          }
        } else { //ボール左
          dir_move = -90;
          if(line_maxn[2]<=2){
            dir_move = -100;
          }else if(line_maxn[2]>=4){
            dir_move = -80;
          }
          if(line_s[1]){
            dir_move=1000;
          }
        }
        
      }
    }
    if (line_s[3]) {
      if (dir_move > 0 && dir_move < 180) {
        speed = 80;
        //dir_move=ball.dir+15; //裏技 ボールフィールド内なのに停止する事象の軽減
        if(ball.dir + gyro.dir < 0){
          dir_move = ball.dir;
        }else if (dir_move < 100 && dir_move > 80 && (abs(ball.dir) >= 25)) {
          speed = 30; //25/7/14 停止から変更
          if(line_maxn[3]<=2){
            //速度だけ下げてそのままの動き
          }else if(line_maxn[3]>=4){
            dir_move = -90;
          }else{
            dir_move = 1000;
          }
        } else if (dir_move < 90 || abs(ball.dir)<25) {
          dir_move = 0;
          if(line_maxn[3]<=2){
            dir_move=10;
          }else if(line_maxn[3]>=4){
            dir_move=-10;
          }
          if(line_s[0]){
            dir_move=1000;
          }
        } else {
          dir_move = -180;
          if(line_maxn[3]<=2){
            dir_move=170;
          }else if(line_maxn[3]>=4){
            dir_move=-170;
          }
          if(line_s[3]){
            dir_move=1000;
          }
        }
        #ifdef kadodassyutu
          if(ping.value[1]<450){
            dir_move = 0;
          }
        #endif
      }
    }
  }
/*
  if (z == -1) {
    if (line_n[0]) {
      linetim.reset();
      z = 180;
    } else if (line_n[1]) {
      linetim.reset();
      z = 90;
    } else if (line_n[2]) {
      linetim.reset();
      z = 0;
    } else if (line_n[3]) {
      linetim.reset();
      z = -90;
    }
  }
*/
  if (analogRead(Pin_ballcatch) < hold_th) {
    timer[11].reset(); //ボール捕捉中
  }else{
    timer[7].reset();
  }
  int kickdir = 0;
  if (timer[11].get() < 500 && ball.isExist && 0) {  // ボール捕捉時 今は無効化
    if (ping.value[1] < 60 && ping.value[0] > 85) {        // ゴールは左  
      if(ping.value[2]>100 ||1) //敵陣
      kickdir = -30;
      else
        kickdir = -10;
      motor.cal_power(dir_move, speed, pid.run((gyro.dir - kickdir)/6));
    } else if (ping.value[1] > 85 && ping.value[0] < 60) {
      if(ping.value[2]>100 ||1) //敵陣
        kickdir = 30;
      else
        kickdir = 10;
      motor.cal_power(dir_move, speed, pid.run((gyro.dir - kickdir)/4));
    } else
      motor.cal_power(dir_move, speed, pid.run(gyro.dir/4));
  } else {
    motor.cal_power(dir_move, speed, pid.run(gyro.dir));
  }
  move.kickdir=kickdir;
  //motor.cal_power(dir_move, speed, pid.run(gyro.dir));

  // if(timer[0].get()<1300){
  //   if(timer[0].get()<300)dir_move=180;
  //   motor.cal_power(dir_move, speed, (gyro.dir-55) / 2);
  //   tone(buzzer,500);
  // }



  //エンジェルライン処理 25/07/20
  for(int i=0;i<4;i++){
    line.value_angel[(i*3 + 11)%12] = line.value[i][7]>line_th[i*8+7]?1:0;
    line.value_angel[i*3 + 0]       = line.value[i][6]>line_th[i*8+6]?1:0;
    line.value_angel[i*3 + 1]       = line.value[i][5]>line_th[i*8+5]?1:0;
  }
  line.Num_angel=0;
  for(int i=0;i<12;i++){
    if(line.value_angel[i]){
      line.Num_angel++;
    }
  }

  float line__angle;
  int lmax;
  if(findLongestZeroGapWithAngle(line.value_angel,line__angle,lmax)==0 && line.Num_angel>=1){ //1~11のエンジェル反応
    
    
    line.sdir=line__angle-gyro.dir;
    if(lmax>=5 || line.dir==1000){
      
      int diff = ((line.mem_linedir - line.sdir + 540) % 360) - 180;

      if(abs(diff)>100 && line.mem_linedir!=1000){ //前回との差が100°以上
        line.isHalfout=!line.isHalfout; //半分以上外に出たと判定
        //tone(buzzer,2000,50);
      }

      if(line.isHalfout){
        line.dir=line.sdir+180;
      }else{
        line.dir=line.sdir;
      }
      if(line.dir>180)line.dir-=360;
      line.mem_linedir=line.sdir;
    }else{ //もっとも広い間隔が4以下
      int diff = ((line.dir - line.sdir + 540) % 360) - 180; //
      if(abs(diff)<90){
        line.dir=line.sdir;
      }else{
        line.dir=(line.sdir+180)%360;
      }
    }
    
    

    //display.drawLine(64, 32, 64 + cos(line.dir * PI / 180.0) * 5, 32 - sin(line.dir * PI / 180.0) * 5, SSD1306_WHITE);
  }else{
    line.sdir=1000;
    if(!line.isHalfout){
      line.dir=1000;
    }
    line.mem_linedir=1000;
  }
  if(line.dir!=1000){
    if(abs(ball.dir)<20 && !(line.isHalfout && line.Num_angel<1) && line.goalchance_count > 250){
      move.carryball(ball.dir);
      z=move.dir;
      speed=50; //ゴールまであと少し押し込み
    }else
      z=-line.dir-gyro.dir;
      if(line.isHalfout && line.Num_angel<1) line.goalchance_count=-250;
  }

  //エンジェルライン処理ここまで


  if (z != -1 && TS) {
    if(abs((z-dir_move+360+180)%360)<90 && line.isHalfout) //回避方向と進行方向が逆なら減速
      speed=50;
    // if(abs((z-dir_move+360+180)%360)<45 && line.isHalfout){ //回避方向と進行方向が逆
    //   z=1000;//停止
    //   motor.cal_power(z, speed, pid.run(gyro.dir));
    // }else if(abs((z-dir_move+360+90)%360)<45 && line.isHalfout){ //回避方向と進行方向
    //   z+=90;
    //   motor.cal_power(z, speed, pid.run(gyro.dir));
    // }else if(abs((z-dir_move+360+270)%360)<45 && line.isHalfout){ //回避方向と進行方向
    //   z+=270;
    //   motor.cal_power(z, speed, pid.run(gyro.dir));
    // }else{
    //   //なにもしない
    // 
    if(line.isHalfout)speed=80;
    motor.cal_power(z, speed, pid.run(gyro.dir));
    //motor.pwm_out();
    // tone(buzzer, 2000, 100);
  }
  if(line_s[0] || line_s[1] || line_s[2] || line_s[3]){ //どれかのセンサーが反応
    timer[16].reset();
  }
  if (linetim.get() > 20) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3]) z = -1;
  }
  if (z == 0 && linetim.get() > 5) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[0]) {
      z = -1;
    }
  }
  if (z == 1 && linetim.get() > 5) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[1]) {
      z = -1;
    }
  }
  if (z == 2 && linetim.get() > 5) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[2]) {
      z = -1;
    }
  }
  if (z == 3 && linetim.get() > 5) {
    if (!line_n[0] && !line_n[1] && !line_n[2] && !line_n[3] && line_s[3]) {
      z = -1;
    }
  }

  if (!TS) {
    motor.stop();
  }

  // if (analogRead(Pin_ballcatch) > hold_th) { //もう一つの処理に統合
  //   timer[7].reset();
  // } else {
  //   // if(line_s[0]){ //角判定
  //   //   timer[0].reset();
  //   // }
  // }
  if (timer[7].get() > 100 && !kick && timer[9].get() > 600 && abs(ball.dir) <15 &&
      abs(gyro.dir - kickdir) < 10) {  //キックする条件
    kicker(1);
    timer[8].reset();
    kick = 1;
  }

  if (timer[8].get() > 100 && kick) { //0.1秒後キッカーをオフに
    kicker(0);
    timer[7].reset();
    kick = 0;
    timer[9].reset();
  }
  if(gyro.ypr[2] < -6){
    digitalWrite(13, LOW);
    timer_lift.reset();
  }else{
    digitalWrite(13, HIGH);
  }
  if (timer_lift.get()<100) {  //持ち上げで停止
    motor.stop();
    line.isHalfout = false; //押し出し復帰処理
  } else {
    
  }

  motor.pwm_out();

}
//メインループここまで


#define MODE_MAX 5

int lp = 0;
void sensormonitor() {
  display.setTextSize(2);
  if(!TS){
  while (!TS) {
    //digitalWrite(13, HIGH);
    gyro.get();
    if (gyro.ypr[2] < -5 && mode!=1) {
      digitalWrite(13, LOW);
    } else {
      digitalWrite(13, HIGH);
    }
    display.setRotation(0);
    motor.stop();
    if(line.dir!=1000){
      //motor.cal_power(-line.dir,50);
    }
    motor.pwm_out();
    kicker(0);
    display.setTextSize(2);

    if (SW1) {
      tone(buzzer, 1077, 100);
      mode++;
      if (mode > MODE_MAX) {
        mode = 0;
      }
      delay(100);
      while (SW1)
        ;
    }
    if (SW3) {
      tone(buzzer, 2000, 100);
      gyro.reset();
      while (SW3)
        ;
    }

    if (mode == 0) {  // ボールモニター
      ball.get();
      display.clearDisplay();
      int SCALE = 15;
      for (int i = 0; i < 16; i++) {
        display.drawLine(
            cos(i * 2 * PI / 16) * ball.value[i] / SCALE + 64,
            sin(i * 2 * PI / 16) * ball.value[i] / SCALE + 32,
            cos((i + 1) * 2 * PI / 16) * ball.value[(i + 1) % 16] / SCALE + 64,
            sin((i + 1) * 2 * PI / 16) * ball.value[(i + 1) % 16] / SCALE + 32,
            SSD1306_WHITE);
      }
      if (ball.isExist)
        display.drawLine(64, 32, -sin(radians(ball.dir - 90)) * 25 + 64,
                         cos(radians(ball.dir - 90)) * 25 + 32, SSD1306_WHITE);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println(ball.dir);
      display.setCursor(60, 0);
      display.println((int)ball.distance);
      display.display();
    } else if (mode == 1) {  // ラインモニター
      display.clearDisplay();
      line.get_value();

      //十字部分 閾値判定
      int line_m[4][4] = {0};
      for (int i = 0; i < 4; i++) {
        if (line.value[i][2] > line_th[i * 8 + 2]) {
          line_m[i][0] = 1;
        }
        if (line.value[i][3] > line_th[i * 8 + 3]) {
          line_m[i][1] = 1;
        }
        if (line.value[i][4] > line_th[i * 8 + 4]) {
          line_m[i][2] = 1;
        }
        if (line.value[i][6] > line_th[i * 8 + 6]) { //5ではなく6
          line_m[i][3] = 1;
        }
      }
      //十字部分 描画
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          if (line_m[i][j] > 0) {
            display.fillRect(64 + cos(i * PI / 2) * (5 - j) * 6 - 3,
                             32 - sin(i * PI / 2) * (5 - j) * 6 - 3, 6, 6,
                             SSD1306_WHITE);
          } else {
            display.fillRect(64 + cos(i * PI / 2) * (5 - j) * 6 - 1,
                             32 - sin(i * PI / 2) * (5 - j) * 6 - 1, 2, 2,
                             SSD1306_WHITE);
          }
          if (lp == i * 4 + j) {
            display.drawCircle(64 + cos(i * PI / 2) * (5 - j) * 6,
                               32 - sin(i * PI / 2) * (5 - j) * 6, 7,
                               SSD1306_WHITE);
          }
        }

        //外側部分 描画
        if (line.value[i][1] > line_th[i * 8 +1]) {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 + sin(i * PI / 2) * 6 - 3,
              32 - sin(i * PI / 2) * (5 - 0) * 6 + cos(i * PI / 2) * 6 - 3, 6,
              6, SSD1306_WHITE);
        } else {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 + sin(i * PI / 2) * 6 - 1,
              32 - sin(i * PI / 2) * (5 - 0) * 6 + cos(i * PI / 2) * 6 - 1, 2,
              2, SSD1306_WHITE);
        }

        //円部分 描画
        if (line.value[i][0] > line_th[i * 8 + 0]) {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 - sin(i * PI / 2) * 6 - 3,
              32 - sin(i * PI / 2) * (5 - 0) * 6 - cos(i * PI / 2) * 6 - 3, 6,
              6, SSD1306_WHITE);
        } else {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 - sin(i * PI / 2) * 6 - 1,
              32 - sin(i * PI / 2) * (5 - 0) * 6 - cos(i * PI / 2) * 6 - 1, 2,
              2, SSD1306_WHITE);
        }

        if (line.value[i][5] > line_th[i * 8 + 5]) { //6ではなく5
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 3,
                           32 - sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 1,
                           32 - sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
        if (line.value[i][7] > line_th[i * 8 + 7]) {
          display.fillRect(64 + cos((i / 2.0 - 1.0 / 6.0) * PI) * 12 - 3,
                           32 - sin((i / 2.0 - 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 - 1.0 / 6.0) * PI) * 12 - 1,
                           32 - sin((i / 2.0 - 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
      }
      //エンジェルライン処理 25/07/20
      for(int i=0;i<4;i++){
        line.value_angel[(i*3 + 11)%12] = line.value[i][7]>line_th[i*8+7]?1:0;
        line.value_angel[i*3 + 0]       = line.value[i][6]>line_th[i*8+6]?1:0;
        line.value_angel[i*3 + 1]       = line.value[i][5]>line_th[i*8+5]?1:0;
      }

      float line__angle;
      int max;
      if(findLongestZeroGapWithAngle(line.value_angel,line__angle,max)==0){
        int mem_linedir=line.sdir;
        line.sdir=line__angle;
        int diff = ((mem_linedir - line.sdir + 540) % 360) - 180;

        if(abs(diff)>120 && mem_linedir!=1000){ //前回との差が120°以上
          line.isHalfout=!line.isHalfout;
        }

        if(line.isHalfout){
          line.dir=line.sdir+180;
        }else{
          line.dir=line.sdir;
        }
        if(line.dir>180)line.dir-=360;

        display.drawLine(64, 32, 64 + cos(line.dir * PI / 180.0) * 10, 32 - sin(line.dir * PI / 180.0) * 10, SSD1306_WHITE);
      }else{
        line.sdir=1000;
        if(!line.isHalfout){
          line.dir=1000;
        }
      }
      
      
      
      //シリアル出力
      for(int i=0;i<31;i++){
        Serial.print(line.value32[i]);
        Serial.print(",");
      }
      Serial.println(line.value32[31]);
      
      display.setTextSize(1);
      display.setCursor(0, 40);
      display.print(max);
      display.setCursor(0, 0);
      display.print(line.value[lp / 4][lp % 4 + 2]);
      display.setCursor(0, 20);
      int l1,l2;
      l1=lp/4;l2=lp % 4 + 2;
      display.print(line_th[l1*8+l2]);
      if (SW2) {
        lp++;
        if (lp > 15) lp = 0; //もともと15
        delay(50);
        tone(buzzer, 2714, 10);
        while (SW2)
          ;
      }
      display.display();
    } else if (mode == 2) {
      display.clearDisplay();
      display.setCursor(0, 0);
      // display.print(analogRead(Pin_ballcatch));
      display.println("Gyro");
      
      display.println(gyro.dir);
      gyro.updateCalibration();
      String line = "S:" + String(gyro.cal_sys) + "  "
              + "G:" + String(gyro.cal_gyro) + "  "
              + "A:" + String(gyro.cal_accel) + "  "
              + "M:" + String(gyro.cal_mag);

      display.print(line);
      // display.println(gyro.ypr[1]);
      // display.println(gyro.ypr[2]);
      display.display();
    } else if (mode == 3) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("line set");
      display.display();

      if (SW2) {
        int min[32];
        int max[32] = {0};
        int max_g[32] = {0};
        for (int i = 0; i < 32; i++) {
          min[i] = 1024;
        }
        while(SW2);//スイッチ離れるまで待つ
        tone(buzzer,2000,100);
        TIMER timer1;
        timer1.reset();
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Scanning Green");
        display.display();
        while(!SW2){  //緑色読み取り
          if(timer1.get()>500){ //ブザー鳴らす
            tone(buzzer,2000,100);
            timer1.reset();
          }

          int line[4][8] = {0};
          for (int i = 0; i < 8; i++) {
            if (i >= 4) {
              digitalWrite(Pin_MPC, 1);
            } else {
              digitalWrite(Pin_MPC, 0);
            }
            if (i % 4 >= 2) {
              digitalWrite(Pin_MPB, 1);
            } else {
              digitalWrite(Pin_MPB, 0);
            }
            if (i % 2 > 0) {
              digitalWrite(Pin_MPA, 1);
            } else {
              digitalWrite(Pin_MPA, 0);
            }
            delayMicroseconds(500);
            line[0][i] = analogRead(A0);
            line[1][i] = analogRead(A1);
            line[2][i] = analogRead(A2);
            line[3][i] = analogRead(A3);
            delayMicroseconds(100);

            if (line[0][i] > max_g[i]) max_g[i] = line[0][i];
            if (line[1][i] > max_g[i + 8]) max_g[i + 8] = line[1][i];
            if (line[2][i] > max_g[i + 16]) max_g[i + 16] = line[2][i];
            if (line[3][i] > max_g[i + 24]) max_g[i + 24] = line[3][i];
            if (line[0][i] < min[i]) min[i] = line[0][i];
            if (line[1][i] < min[i + 8]) min[i + 8] = line[1][i];
            if (line[2][i] < min[i + 16]) min[i + 16] = line[2][i];
            if (line[3][i] < min[i + 24]) min[i + 24] = line[3][i];
          }
        }
        while(SW2);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Scanning White");
        display.display();
        while (!SW2) {
          tone(buzzer, 2000);
          int line[4][8] = {0};
          for (int i = 0; i < 8; i++) {
            if (i >= 4) {
              digitalWrite(Pin_MPC, 1);
            } else {
              digitalWrite(Pin_MPC, 0);
            }
            if (i % 4 >= 2) {
              digitalWrite(Pin_MPB, 1);
            } else {
              digitalWrite(Pin_MPB, 0);
            }
            if (i % 2 > 0) {
              digitalWrite(Pin_MPA, 1);
            } else {
              digitalWrite(Pin_MPA, 0);
            }
            delayMicroseconds(500);
            line[0][i] = analogRead(A0);
            line[1][i] = analogRead(A1);
            line[2][i] = analogRead(A2);
            line[3][i] = analogRead(A3);
            delayMicroseconds(100);

            if (line[0][i] > max[i]) max[i] = line[0][i];
            if (line[1][i] > max[i + 8]) max[i + 8] = line[1][i];
            if (line[2][i] > max[i + 16]) max[i + 16] = line[2][i];
            if (line[3][i] > max[i + 24]) max[i + 24] = line[3][i];
            if (line[0][i] < min[i]) min[i] = line[0][i];
            if (line[1][i] < min[i + 8]) min[i + 8] = line[1][i];
            if (line[2][i] < min[i + 16]) min[i + 16] = line[2][i];
            if (line[3][i] < min[i + 24]) min[i + 24] = line[3][i];
          }
        }
        if (SW2) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.print("EEPROM writeing...");
          display.display();
          tone(buzzer, 2500, 180);
          delay(200);
          tone(buzzer, 3000, 100);
          for (int i = 0; i < 32; i++) {
            //EEPROM.write(i, (max[i] + min[i]) / 8);
            EEPROM.write(i,(((max[i]-max_g[i])*0.1 + max_g[i]) /4));
          }
          for (int i = 0; i < 32; i++) {
            line_th[i] = EEPROM.read(i) * 4;
          }
        }
      }
    } else if (mode == 4) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("PING");

      for (int i = 0; i < 4; i++) {
        ping.get(i);
        display.println(ping.value[i]);
      }
      display.display();

    } else if (mode == 5) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("CAMERA");

      //display.println("BLUE");

      //openmv.set_color(OPENMV_BLUE);
      //display.println(openmv.get_goal());
      
      openmv.getorangeball();
      display.println(openmv.orangeX);
      display.println(openmv.orangeY);
      display.println(openmv.orangedir);

      
      display.display();

      if(SW2){
        kicker(1);
        delay(100);
        kicker(0);
        delay(200);
        
      }

    }  // end if
  }    // end while
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(30, 20);
  display.println("Singularity");
  display.display();
  }
}


