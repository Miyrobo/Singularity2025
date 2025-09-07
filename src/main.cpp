#include <Arduino.h>
#include <EEPROM.h>
#include <device.h>
#include <game.h>
#include <instance.h>

#define MAX_Speed 100

//#define PIDmonitor

int WirelessControl=0;

bool radicon=false;
int joyX = 0;
int joyY = 0;
int lastJoyX = 0;
int lastJoyY = 0;

unsigned long lastReceiveTime = 0; // 最終受信時刻
const unsigned long timeout = 250;  // 250msで停止

TIMER timfps;
int fps=0;

bool kick = 0; //キッカー状態

int hold_th; //ホールドセンサ 閾値

void sensormonitor();
void sensormonitor_old();
int mode = 0;  // センサーモニターモード

void Wireless_debug(); //無線デバッグ

int mapJoystick(int val);

bool keepermode_now=1;
int balldir_r;

//------------------------
// PID用変数
float Kp = 0.0;
float Ki = 0.0;
float Kd = 0.0;
// 受信バッファ
String inputString = "";

void parsePID(String str);

void readPIDSerial() {
  while (ESP32_UART.available()) {
    tone(buzzer,2000,10);
    char inChar = (char)ESP32_UART.read();
    if (inChar == '\n' || inChar == '\r') {
      inputString.trim();
       // Vコマンド判定
      if (inputString.equals("V")) {
        motor.cal_power(0,0,50); // 振動処理
        motor.pwm_out();
        delay(100);
      } else
        parsePID(inputString);
      inputString = "";
    } else {
      inputString += inChar;
    }
  }
}

// --- 関数: 受信文字列を解析して PID 変数に反映 ---
void parsePID(String str) {
  Kp=pid.Kp;Kd=pid.Kd;Ki=pid.Ki;
  if (str.length() < 2) return; // 最低限の長さチェック

  char type = str.charAt(0);           // P/I/D
  float val = str.substring(1).toFloat(); // 数値部分

  switch (type) {
    case 'P': pid.Kp = val; break;
    case 'I': pid.Ki = val; break;
    case 'D': pid.Kd = val; break;
    default: break; // 不明文字は無視
  }

  // デバッグ表示
  // Serial.print("Updated PID: ");
  // Serial.print("Kp="); Serial.print(Kp);
  // Serial.print(" Ki="); Serial.print(Ki);
  // Serial.print(" Kd="); Serial.println(Kd);
}

void setup() {
  pins_init(); //ピン設定
  ledring.begin();
  ledring.clear();
    ledring.show();
  
  mysetting.load(); //設定読み込み

  tone(buzzer, 2714, 120);
  
  //delay(100);
  if(mysetting.neopixel!=NeoPixel_OFF){
    ledring.rainbow();
    ledring.update();
  }else{
    ledring.clear();
    ledring.show();
  }
  
  openmv.setup();
  Display_Singularityinit();
  analogReadAveraging(5);

  Serial.begin(9600);
  ESP32_UART.begin(115200);

  motor.stop();
  motor.pwm_out();
  gyro.setup();

  

  
  //EEPROM.write(34, 85); //ホールドセンサ閾値
  hold_th = EEPROM.read(34);

  timer[0].reset();
  if(SW1){
    tone(buzzer,2000,100); //恐竜ゲーム起動
    dinogame();
  }
  
  Startup_sound(); //起動音
  
  while (SW1 || SW2 || SW3)
    ;
}

bool WirelessStop=false;

TIMER timer_lift; //持ち上げ検出タイマ

//========================================================================================================================
//ここからサッカーメインプログラム
void loop() {
  openmv.update();

  move.speed = mysetting.movespeed;
  fps++;
  if(timfps.get()>1000){ //1秒間の処理速度表示
    display.clearDisplay();
    display.setRotation(0);
    display.setCursor(40,25);
    display.setTextSize(4);
    display.print(fps);
    // display.setTextSize(2);
    // display.setCursor(10,10);
    // display.print("P : ");
    // display.println(pid.Kp);
    // display.print("I : ");
    // display.println(pid.Ki);
    // display.print("D : ");
    // display.println(pid.Kd);
    display.display();
    fps=0;
    timfps.reset();
  }
  motor.brake=false;
  sensormonitor();
  motor.brake=true;

  gyro.get();
  // gyro.getvel();
// Serial.print(gyro.ax);
// Serial.print(",");
// Serial.print(gyro.ay);
// Serial.print(",");
// Serial.print(gyro.vx);
// Serial.print(",");
// Serial.println(gyro.vy);
  //ライン読み取り
  line.get_value(); 
  line.check(sensors);


  if (pingset.get() > 200) { //超音波を200ms間隔で読み取り
    ping.get(0);
    ping.get(1);
    ping.get(2);
    pingset.reset();
  }

  if(mysetting.balltype == BALL_IS_IR){ //ボールモード
    ball.get(); //ボール位置取得
  }else{
     ColorPos Orange = openmv.getOrange();

      ball.dir=Orange.dir;
      ball.isExist=Orange.found;

      if(Orange.distance < 45){
        ball.distance=100;
      }else{
        ball.distance=5000;
      }
  }
  

  if (ball.isExist) { //ボール見えた
    move.carryball(sensors);
    timer[13].reset();
  } else {
    move.dir = 1000;
  }
  

  if(timer[13].get()>100 && mysetting.useuss){ //ボールが見えずに0.1秒経過 定位置復帰
    int pingdiff = ping.value[0]-ping.value[1]; //右に行くほど+
    move.speed = 80;
    if(ping.value[2]>90){
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

  move.avoid_line(sensors); //ライン回避

  //ペナルティエリア抜け出し
  // if(move.dir==1000) move.speed=0;
  // int x=sin(radians(move.dir))*move.speed;
  // if(ball.dir > 50 && ball.dir < 130 && abs(x) < 10 && move.speed < 50 && ball.distance>2500){
  //   move.dir=0;
  //   move.speed=80;
  // }else if(ball.dir < -50 && ball.dir > -130 && abs(x) < 10 && move.speed < 50 && ball.distance>2500){
  //   move.dir=0;
  //   move.speed=80;
  // }
  
  if(abs(ball.dir)<15 && line.goalchance_count<0)line.goalchance_count++;
  //line.goalchance_count=0; //機能無効化
  

  if (analogRead(Pin_ballcatch) < hold_th) {
    timer[11].reset(); //ボール捕捉中
  }else{
    timer[7].reset();
  }
  if(move.speed>MAX_Speed) //スピードを制限
    move.speed=MAX_Speed;
  int kickdir = 0;
  //kickdir=move.toGoalPING(sensors);
  ColorPos Yellow= openmv.getYellow();
  if(mysetting.usecamera && Yellow.found){ //カメラでゴールを向く
    
    if (timer[11].get()<200 && ball.isExist && abs(ball.dir)< 100 && !line.depth[0]) {  // ボール捕捉時
      if(Yellow.found){
        timer[14].reset();
        kickdir=gyro.dir+Yellow.dir;
        kickdir=constrain(kickdir,-50,50); //±50の範囲に
      }
    }
  }else{
    if(timer[11].get()<200 && ball.isExist && abs(ball.dir)< 100 && !line.depth[0])
      kickdir=move.toGoalPING(sensors);
  }
  
  move.kickdir=kickdir;

  if(move.speed>mysetting.movespeed)move.speed=mysetting.movespeed;

  int PID=pid.run(gyro.dir - kickdir, gyro.dir);//pid.run(gyro.dir - kickdir,gyro.vel);
  if(abs(gyro.dir)<5 || (abs(gyro.dir)<abs(kickdir) && gyro.dir*kickdir>=0))
    if(timer[14].get()<750)PID=PID/2.5;
  motor.cal_power(move.dir, move.speed, PID);

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
  if(abs(gyro.ypr[2]) > 15 || abs(gyro.ypr[1])>15){
    digitalWrite(13, LOW);
    timer_lift.reset(); //持ち上げ検知タイマーリセット
  }else{
    timer[15].reset();
    digitalWrite(13, HIGH);
  }
  if (timer_lift.get()<100) {  //持ち上げられた
    motor.stop();
    line.isHalfout = false; //押し出し復帰処理
  } else {
    
  }

  //------------------------------------------

  //キーパー
  if(keepermode_now){
  Yellow= openmv.getYellow();
  if(!line.isHalfout && line.Num_angel<1){ //ペナルティエリア外
    if(Yellow.found && abs(Yellow.dir)>90){
      move.dir=Yellow.dir;
    }else{
      int pingdiff = ping.value[0]-ping.value[1]; //右に行くほど+
      move.speed = 80;
      if(pingdiff > 20){
        move.dir = -135;
      }else if(pingdiff < -20){
        move.dir = 135;
      }else{
        move.dir=180;
      }
    }
  }else{ //
    int xx=0,yy=0;
    //yy フィールドに戻る強さ
    if(line.Num_angel<1){ //完全に押し出された  
      move.dir=line.dir;
      move.speed=80;
    }else {
      if(line.lmax >= 9){
        if(line.isHalfout) yy=50;
        else yy=-50;
      }else if(line.lmax >= 6){
        if(line.isHalfout) yy=20;
        else yy=-20;
      }
      if(ball.isExist){
        xx=sin(radians(ball.dir - line.dir))*150;
        if(line.dir>75 && ball.dir>0)xx=0;
        else if(line.dir<-75 && ball.dir<0)xx=0;
      }
      move.dir=atan2(xx,yy)*57.3 + line.dir;
      move.speed=sqrt(xx*xx + yy*yy);
    }
      
  }
  PID=pid.run(gyro.dir - kickdir, gyro.dir);//pid.run(gyro.dir - kickdir,gyro.vel);
  if(abs(gyro.dir)<5 || (abs(gyro.dir)<abs(kickdir) && gyro.dir*kickdir>=0))
  if(timer[14].get()<750)PID=PID/2.5;
  motor.cal_power(move.dir, move.speed, PID);
  if(abs(balldir_r-ball.dir)>10){
    timer[16].reset();
    balldir_r=ball.dir;
  }
  if(ball.isExist && timer[16].get()>2000 && ball.distance<3400 && abs(ball.dir)<40){
    keepermode_now=0;
    timer[16].reset();
  }
  }else{ //keepermode_now==0
    if(timer[16].get()>1000){
      keepermode_now=1;
      timer[16].reset();
    }
  }
  
  //------------------------------------------
  



  //Wireless_debug();  //ESP32使用 無線デバッグ機能

  if(radicon){
  String xStr, yStr;

  if (ESP32_UART.available()) {
    // データ受信
    xStr = ESP32_UART.readStringUntil(',');
    yStr = ESP32_UART.readStringUntil('\n');

    lastJoyX = mapJoystick(xStr.toInt());
    lastJoyY = mapJoystick(yStr.toInt());

    lastReceiveTime = millis(); // 受信時刻更新
  }

  // 50ms以内に信号がなければ停止
  if (millis() - lastReceiveTime > timeout) {
    joyX = 0;
    joyY = 0;
  } else {
    joyX = lastJoyX;
    joyY = lastJoyY;
  }

  // 移動量計算
  move.dir = degrees(atan2(joyY, joyX));
  move.speed = sqrt(joyX * joyX + joyY * joyY);

  motor.cal_power(move.dir, move.speed,pid.run(gyro.dir));
  }

#ifdef PIDmonitor
  if(timer[17].get()>5){
    // ESP32_UART.print(gyro.dir);
    // ESP32_UART.print(",");
    // ESP32_UART.print(PID);
    // ESP32_UART.print(",");
    // ESP32_UART.println(pid.get_v());
    ESP32_UART.print(pid.P);
    ESP32_UART.print(",");
    ESP32_UART.print(pid.I);
    ESP32_UART.print(",");
    ESP32_UART.println(pid.D);
    timer[17].reset();
  }
    // Serial.print(pid.P);
    // Serial.print(",");
    // Serial.print(pid.I);
    // Serial.print(",");
    // Serial.println(pid.D);
  readPIDSerial();
#endif

  motor.pwm_out(); //モーター出力

  ledring.clear();
  if(mysetting.neopixel==NeoPixel_ON && ball.isExist){
    ledring.setLedDirection(ball.dir,3,CRGB(255,0,0));
  }
  if(timer_lift.get()<100 && mysetting.neopixel==NeoPixel_ON){
      if(millis()%500<250){
        ledring.setColorAll(CRGB::Red);
      }else{
        ledring.clear();
      }
  }
  if(mysetting.neopixel==NeoPixel_ON)
    for(int i=0;i<4;i++)
      if(line.depth[i]){
        ledring.setLedDirection(i*90,3,CRGB::White);
      }

  if(mysetting.neopixel==NeoPixel_ON){
    ColorPos Yellow= openmv.getYellow();
    if(Yellow.found)
      ledring.setLedDirection(Yellow.dir,1,CRGB::Green);
  }
  if(mysetting.neopixel==NeoPixel_OFF){
    ledring.clear();
  }
  ledring.show();

}

//-----------------------------------------------------------------------------------------------------------------------
//メインループここまで



  //ESP32使用 無線デバッグ機能
void Wireless_debug(){
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
}

int mapJoystick(int val) {
  const int center = 2000;   // 中心値
  const int dead = 200;      // デッドゾーン ±100

  if (val >= center - dead && val <= center + dead) {
    return 0; // デッドゾーン内はゼロ
  }
  
  int result;
  if (val < center - dead) {
    // 中心以下: -100〜0
    result = map(val, center - dead, 0, 0, -100);
  } else {
    // 中心以上: 0〜100
    result = map(val, center + dead, 4095, 0, 100); // 12bit ADC想定
  }
  // 100を超えないように制限
  if (result > 100) result = 100;
  if (result < -100) result = -100;

  return result;
}

#define MODE_MAX 5

void sensormonitor(){
  if(!TS){
    //センサモニタ開始処理
    int mode=0;
    int setting_menu=0;
    while(!TS){
      
      motor.brake=false;
      motor.stop();
      motor.pwm_out();
      kicker(0);
      
      int cx,cy;
      if(mode==0){ //待機モード
        ledring.show();
        ledring.clear();
        if(sw2.pushed() && mode==0){
          tone(buzzer,2000,100);
          sensormonitor_old();
        }
        display.clearDisplay();
        display.setTextSize(2);
        display.setRotation(1); //縦表示

        //ボールモニタ
        if(mysetting.balltype==BALL_IS_IR){
          ball.get();
          display.setTextSize(1);
          display.setCursor(50,5);
          display.print("IR");
        }else{
          openmv.update();
          ColorPos orange = openmv.getOrange();
          ball.dir=orange.dir;
          ball.isExist = orange.found;
          display.setTextSize(1);
          display.setCursor(50,5);
          display.print("Or");
        }
        cx=20,cy=40;
        display.drawCircle(cx,cy,15,SSD1306_WHITE);
        display.drawLine(cx-17,cy,cx+17,cy,SSD1306_WHITE);
        display.drawLine(cx,cy-17,cx,cy+17,SSD1306_WHITE);
        if(ball.isExist){
          display.drawLine(cx,cy,cx + sin(radians(ball.dir))*17,cy - cos(radians(ball.dir))*17,SSD1306_WHITE);
          display.fillCircle(cx + sin(radians(ball.dir))*17,cy - cos(radians(ball.dir))*17,3,SSD1306_WHITE);
        }

        //方位モニタ
        gyro.get();
        cx=45,cy=70;
        display.drawCircle(cx,cy,15,SSD1306_WHITE);
        display.drawLine(cx,cy,cx + sin(radians(-gyro.dir))*17,cy - cos(radians(-gyro.dir))*17,SSD1306_WHITE);

        //キャリブレーション状態チェック
        gyro.updateCalibration();
        display.setCursor(35,115);
        if(gyro.cal_gyro!=3)display.print("G");
        display.setCursor(50,115);
        if(gyro.cal_mag!=3)display.print("M");

        display.display();

        if (sw3.pushed()) {
          tone(buzzer, 1500, 100);
          gyro.reset();
        }

        if (sw1.pushed()) {
          tone(buzzer, 2000, 100);
          mode = 1; //設定モード移行
          setting_menu = 0;
        }
      }else if(mode==1){ //設定モード
        if(mysetting.neopixel!=NeoPixel_OFF){
          ledring.update();
        }
        display.clearDisplay();
        display.setTextSize(1);
        display.setRotation(1); //縦表示

        display.setCursor(0,5);
        display.print("setting");

        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,40);
        display.print("Ball:");
        // if(setting_menu==0){
        //   display.setTextColor(SSD1306_BLACK,SSD1306_WHITE);
        // }
        if(mysetting.balltype == BALL_IS_IR) display.print("IR");
        else display.print("ORNG");
        display.setTextColor(SSD1306_WHITE);
        if(setting_menu==0){
          display.print("<");
          if(sw2.pushed()){
            tone(buzzer,2500,100);
            mysetting.balltype = !mysetting.balltype;
          }
        }
        
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,60);
        display.print("Speed:");
        // if(setting_menu==1){
        //   display.setTextColor(SSD1306_BLACK,SSD1306_WHITE);
        // }
        display.print(mysetting.movespeed);
        display.print("%");
        display.setTextColor(SSD1306_WHITE);
        if(setting_menu==1){
          display.print("<");
          if(sw2.pushed()){
            tone(buzzer,2500,100);
            mysetting.movespeed = mysetting.movespeed+10;
            if(mysetting.movespeed>100)mysetting.movespeed=50;
          }
        }

        display.setCursor(0,80);
        display.print("Cam:");
        // if(setting_menu==2){
        //   display.setTextColor(SSD1306_BLACK,SSD1306_WHITE);
        // }
        if(mysetting.usecamera) display.print("ON");
        else display.print("OFF");
        display.setTextColor(SSD1306_WHITE);
        if(setting_menu==2){
          display.print("<");
          if(sw2.pushed()){
            tone(buzzer,2500,100);
            mysetting.usecamera = !mysetting.usecamera;
          }
        }

        display.setCursor(0,100);
        display.print("Uss:");
        if(mysetting.useuss) display.print("ON");
        else display.print("OFF");
        display.setTextColor(SSD1306_WHITE);
        if(setting_menu==3){
          display.print("<");
          if(sw2.pushed()){
            tone(buzzer,2500,100);
            mysetting.useuss = !mysetting.useuss;
          }
        }

        display.setCursor(0,120);
        display.print("LED:");
        if(mysetting.neopixel==0) display.print("ON");
        else if(mysetting.neopixel==1)display.print("OFF");
        else display.print("Auto");
        display.setTextColor(SSD1306_WHITE);
        if(setting_menu==4){
          display.print("<");
          if(sw2.pushed()){
            tone(buzzer,2500,100);
            mysetting.neopixel ++;
            if(mysetting.neopixel>2)mysetting.neopixel=0;
          }
        }


        display.display();

        if(sw1.pushed()){
          tone(buzzer, 1700, 100);
          setting_menu++;
          if(setting_menu>=5)setting_menu=0;
        }
        if(sw3.pushed()){
          tone(buzzer, 2000, 100);
          mode = 0;
          mysetting.save();
        }
      }//mode分岐
    }
    //センサモニタ終了処理
  }
  
}

int lp = 0;
void sensormonitor_old() {
  display.setTextSize(2);
  if(!TS){
  while (!TS) {
    ledring.show();
    ledring.clear();
    if(ESP32_UART.available()){
    Serial.write(ESP32_UART.read());
    //tone(buzzer,1000,1);
    }
    //digitalWrite(13, HIGH);
    gyro.get();
    if (abs(gyro.ypr[2]) > 6 || abs(gyro.ypr[1])>10 && mode!=1) {
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
      //ESP32_UART.print("Singularity ball=");
      ESP32_UART.println(ball.dir);
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
      display.println(analogRead(Pin_ballcatch));
      display.setCursor(60, 0);
      display.println((int)ball.distance);
      display.display();
      if(ball.isExist){
        ledring.setLedDirection(ball.dir,3,CRGB::Red);
      }
    } else if (mode == 1) {  // ラインモニター
      display.clearDisplay();
      line.get_value();

      for(int i=0;i<12;i++){
        if(line.s_angel[i])
          ledring.LEDs[i]=CRGB(0,255,0);
        else
          ledring.LEDs[i]=CRGB(0,0,0);
      }
      ledring.show();
      

      //十字部分 閾値判定
      int line_m[4][4] = {0};
      for (int i = 0; i < 4; i++) {
        if (line.s[i][2]) {
          line_m[i][0] = 1;
        }
        if (line.s[i][3]) {
          line_m[i][1] = 1;
        }
        if (line.s[i][4]) {
          line_m[i][2] = 1;
        }
        if (line.s[i][6]) { //5ではなく6
          line_m[i][3] = 1;
        }
      }
      //十字部分 描画
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          if (line_m[i][j] > 0) {
            display.fillRect(64 + cos(i * PI / 2) * (5 - j) * 6 - 3,
                             32 + sin(i * PI / 2) * (5 - j) * 6 - 3, 6, 6,
                             SSD1306_WHITE);
          } else {
            display.fillRect(64 + cos(i * PI / 2) * (5 - j) * 6 - 1,
                             32 + sin(i * PI / 2) * (5 - j) * 6 - 1, 2, 2,
                             SSD1306_WHITE);
          }
          if (lp == i * 4 + j) {
            display.drawCircle(64 + cos(i * PI / 2) * (5 - j) * 6,
                               32 + sin(i * PI / 2) * (5 - j) * 6, 7,
                               SSD1306_WHITE);
          }
        }

        //外側部分 描画
        if (line.s[i][1]) {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 - sin(i * PI / 2) * 6 - 3,
              32 + sin(i * PI / 2) * (5 - 0) * 6 + cos(i * PI / 2) * 6 - 3, 6,
              6, SSD1306_WHITE);
        } else {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 - sin(i * PI / 2) * 6 - 1,
              32 + sin(i * PI / 2) * (5 - 0) * 6 + cos(i * PI / 2) * 6 - 1, 2,
              2, SSD1306_WHITE);
        }

        //円部分 描画
        if (line.s[i][0]) {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 + sin(i * PI / 2) * 6 - 3,
              32 + sin(i * PI / 2) * (5 - 0) * 6 - cos(i * PI / 2) * 6 - 3, 6,
              6, SSD1306_WHITE);
        } else {
          display.fillRect(
              64 + cos(i * PI / 2) * (5 - 0) * 6 + sin(i * PI / 2) * 6 - 1,
              32 + sin(i * PI / 2) * (5 - 0) * 6 - cos(i * PI / 2) * 6 - 1, 2,
              2, SSD1306_WHITE);
        }

        if (line.s[i][7]) {
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 3,
                           32 + sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 + 1.0 / 6.0) * PI) * 12 - 1,
                           32 + sin((i / 2.0 + 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
        if (line.s[i][5]) {
          display.fillRect(64 + cos((i / 2.0 - 1.0 / 6.0) * PI) * 12 - 3,
                           32 + sin((i / 2.0 - 1.0 / 6) * PI) * 12 - 3, 6, 6,
                           SSD1306_WHITE);
        } else {
          display.fillRect(64 + cos((i / 2.0 - 1.0 / 6.0) * PI) * 12 - 1,
                           32 + sin((i / 2.0 - 1.0 / 6) * PI) * 12 - 1, 2, 2,
                           SSD1306_WHITE);
        }
      }      
      
      
      //シリアル出力
      for(int i=0;i<31;i++){
        Serial.print(line.value32[i]);
        Serial.print(",");
      }
      Serial.println(line.value32[31]);

      for (int i = 0; i < 12; i++) {
      ESP32_UART.print(line.value_angel[i]);
      if (i < 12) ESP32_UART.print(",");  // 最後の要素にはカンマを付けない
      }
      for(int i=0;i<12;i++){
        ESP32_UART.print(line.s_angel[i]);
        if(i < 11) ESP32_UART.print(",");
      }
        
      ESP32_UART.println();  // 改行で1パケット終了

      display.setTextSize(1);
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
      ledring.setLedDirection(-gyro.dir,3,CRGB::Purple);
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
          if(timer1.get()%500<250){
            ledring.setColorAll(CRGB(0,255,0));
          }else{
            ledring.setColorAll(CRGB(0,0,0));
          }
          ledring.show();

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
            line[0][i] = analogRead(Pin_Line1);
            line[1][i] = analogRead(Pin_Line2);
            line[2][i] = analogRead(Pin_Line3);
            line[3][i] = analogRead(Pin_Line4);
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
          if(timer1.get()%500<250){
            ledring.setColorAll(CRGB(255,255,255));
          }else{
            ledring.setColorAll(CRGB(0,0,0));
          }
          ledring.show();
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
      openmv.update();
      // ColorPos Yellow = openmv.getYellow();
      // display.println(Yellow.x);
      // display.println(Yellow.y);
      // display.print(Yellow.dir);
      ColorPos Orange = openmv.getOrange();
      display.println(Orange.x);
      display.println(Orange.y);
      display.print(Orange.dir);
      display.print("  ");
      display.print(Orange.distance);
      //display.print(openmv.orangedistance);

      
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


