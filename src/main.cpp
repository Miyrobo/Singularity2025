#include <Arduino.h>
#include <EEPROM.h>
#include <UI.h>
#include <SPI.h>
#include <Wire.h>
#include <move.h>
#include <device.h>
#include <sensors.h>
#include <game.h>


#define MAX_Speed 100

int WirelessControl=0;


MOTOR motor;
BALL ball;
BNO gyro;
ULTRASONIC ping;
MOVE move;
CAMERA openmv;
PID pid;
LINE line;
PUSHSWITCH sw1(Pin_S1);
PUSHSWITCH sw2(Pin_S2);
PUSHSWITCH sw3(Pin_S3);


TIMER timer[20];
TIMER linetim;
TIMER pingset;

//構造体にロボットの状態をまとめる
Sensors sensors{ball, gyro, ping, openmv, line};
Actuator act{motor, move, pid};

TIMER timfps;
int fps=0;

bool kick = 0; //キッカー状態

int hold_th; //ホールドセンサ 閾値

void sensormonitor();
int mode = 0;  // センサーモニターモード


void setup() {
  //ピン設定
  pins_init(); 

  tone(buzzer, 2714, 120);
  openmv.begin();
  Display_Singularityinit();
  analogReadAveraging(5);

  Serial.begin(9600);
  ESP32_UART.begin(115200);

  motor.stop();
  motor.pwm_out();
  gyro.setup();
  
  //EEPROM.write(34, 100);
  hold_th = EEPROM.read(34);

  timer[0].reset();
  if(SW1)dinogame();
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

bool WirelessStop=false;

//#define kadodassyutu//2025/6/22
TIMER timer_lift; //持ち上げ検出タイマ

//========================================================================================================================
void loop() {
  
  move.speed = MAX_Speed;
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

  gyro.get();
//ライン読み取り
  line.get_value();
  line.check(sensors);


  if (pingset.get() > 200 && 0) { //超音波を200ms間隔で読み取り
    ping.get(0);
    ping.get(1);
    ping.get(2);
    pingset.reset();
  }

  ball.get(); //ボール位置取得

  if (ball.isExist) { //ボール見えた
    if(move.kickdir == 0){
      move.carryball(ball.dir + gyro.dir,ball.distance);
      move.dir=move.dir-gyro.dir;
    }else{
      move.carryball(ball.dir,ball.distance);
    }

    if (move.dir > 180)
      move.dir -= 360;
    else if (move.dir < -180)
      move.dir += 360;

    timer[13].reset();
  } else {
    move.dir = 1000;
  }
  

  if(timer[13].get()>100 && 0){ //ボールが見えずに0.1秒経過 定位置復帰
    int pingdiff = ping.value[0]-ping.value[1]; //右に行くほど+
    move.speed = 80;
    if(ping.value[2]>70){
      if(pingdiff > 20){
        move.dir = -135;
      }else if(pingdiff < -20){
        move.dir = 135;
      }else{
        move.dir=180;
      }
    }else{
      if(pingdiff > 20){
        move.dir = -90;
      }else if(pingdiff < -20){
        move.dir = 90;
      }else{
        move.dir=1000;
      }
    }
    
  }

  line.avoid(sensors,act); //ライン回避

  
  if(abs(ball.dir)<15 && line.goalchance_count<0)line.goalchance_count++;
  //line.goalchance_count=0; //機能無効化
  

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
      motor.cal_power(move.dir, move.speed, pid.run((gyro.dir - kickdir)/6));
    } else if (ping.value[1] > 85 && ping.value[0] < 60) {
      if(ping.value[2]>100 ||1) //敵陣
        kickdir = 30;
      else
        kickdir = 10;
      motor.cal_power(move.dir, move.speed, pid.run((gyro.dir - kickdir)/4));
    } else
      motor.cal_power(move.dir, move.speed, pid.run(gyro.dir/4));
  } else {
    motor.cal_power(move.dir, move.speed, pid.run(gyro.dir));
  }
  move.kickdir=kickdir;



  if (!TS) {
    motor.stop();
  }


  if (timer[7].get() > 100 && !kick && timer[9].get() > 600 && abs(ball.dir) <15 &&
      abs(gyro.dir - kickdir) < 10 && !WirelessStop) {  //キックする条件
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
  if(abs(gyro.ypr[2]) > 6 || abs(gyro.ypr[1])>10){
    digitalWrite(13, LOW);
    timer_lift.reset(); //持ち上げ検知タイマーリセット
  }else{
    digitalWrite(13, HIGH);
  }
  if (timer_lift.get()<100) {  //持ち上げで停止
    motor.stop();
    line.isHalfout = false; //押し出し復帰処理
  } else {
    
  }

  //ESP32使用 無線デバッグ機能
  if(WirelessStop)motor.stop();
  int c=Serial5.read();

  if(c=='1'){
    WirelessControl=0;
    WirelessStop=false;
    tone(buzzer, 800, 80);  // 高めの音で素早く鳴らす
    delay(90);              // 音の終了を待ってすぐスタート
  }else if(c=='2'){
    WirelessStop=true;
    WirelessControl=0;
    motor.stop();
    motor.pwm_out();
    tone(buzzer, 800, 100); delay(120);
    tone(buzzer, 600, 100); delay(120);
    tone(buzzer, 400, 100); delay(120);
  }else if(c=='f'){
    if(WirelessControl==1)WirelessControl=0;
    else WirelessControl=1;
  }else if(c=='r'){
    if(WirelessControl==2)WirelessControl=0;
    else WirelessControl=2;
  }else if(c=='l'){
    if(WirelessControl==4)WirelessControl=0;
    else WirelessControl=4;
  }else if(c=='b'){
    if(WirelessControl==3)WirelessControl=0;
    else WirelessControl=3;
  }else if(c=='c'){
    if(WirelessControl==5)WirelessControl=0;
    else WirelessControl=5;
  }else if(c=='u'){
    if(WirelessControl==6)WirelessControl=0;
    else WirelessControl=6;
  }
  switch (WirelessControl)
  {
  case 1:
    motor.cal_power(0,60); break;
  case 2:
    motor.cal_power(90,60); break;
  case 3:
    motor.cal_power(180,60); break;
  case 4:
    motor.cal_power(-90,60); break;
  case 5:
    motor.cal_power(0,0,20); break;
  case 6:
    motor.cal_power(0,0,-20); break;
  default:
    break;
  }
  ESP32_UART.println(ball.dir);
  //無線デバッグ機能 ここまで

  motor.pwm_out(); //モーター出力

}

//-----------------------------------------------------------------------------------------------------------------------
//メインループここまで


#define MODE_MAX 5

int lp = 0;
void sensormonitor() {
  display.setTextSize(2);
  if(!TS){
  while (!TS) {
    if(Serial5.available()){
    Serial.write(Serial5.read());
    //tone(buzzer,1000,1);
    }
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

    if (sw1.pushed()) {
      tone(buzzer, 1077, 100);
      mode++;
      if (mode > MODE_MAX) {
        mode = 0;
      }

    }
    if (sw3.pushed()) {
      tone(buzzer, 2000, 100);
      gyro.reset();
    }

    if (mode == 0) {  // ボールモニター
      ball.get();
      //Serial5.print("Singularity ball=");
      Serial5.println(ball.dir);
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
        if (line.value[i][2] > line._th[i * 8 + 2]) {
          line_m[i][0] = 1;
        }
        if (line.value[i][3] > line._th[i * 8 + 3]) {
          line_m[i][1] = 1;
        }
        if (line.value[i][4] > line._th[i * 8 + 4]) {
          line_m[i][2] = 1;
        }
        if (line.value[i][6] > line._th[i * 8 + 6]) { //5ではなく6
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
        if (line.value[i][1] > line._th[i * 8 +1]) {
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
        if (line.value[i][0] > line._th[i * 8 + 0]) {
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

        if (line.value[i][5] > line._th[i * 8 + 5]) { //6ではなく5
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 3,
                           32 - sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 1,
                           32 - sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
        if (line.value[i][7] > line._th[i * 8 + 7]) {
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
        line.value_angel[(i*3 + 11)%12] = line.value[i][7]>line._th[i*8+7]?1:0;
        line.value_angel[i*3 + 0]       = line.value[i][6]>line._th[i*8+6]?1:0;
        line.value_angel[i*3 + 1]       = line.value[i][5]>line._th[i*8+5]?1:0;
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
      display.print(line._th[l1*8+l2]);
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

      display.print(line.c_str());
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
            line._th[i] = EEPROM.read(i) * 4;
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
      display.print(openmv.orangedir);
      display.print("  ");
      display.print(openmv.orangedistance);

      
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


