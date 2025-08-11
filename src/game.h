#include "Arduino.h"
#include "UI.h"
#include "sensors.h"
#include "device.h"
#include "EEPROM.h"

//Googleの恐竜ゲームができます
//--------------------------------------
/////////////////////////////////////////////////////////////////////////////////////////////
//dinogame


#pragma once


const unsigned char bitmap_dino[] PROGMEM = {
  B00000000, B00000001, B11111111, B11000000,
  B00000000, B00000011, B11111111, B11000000,
  B00000000, B00000011, B10111111, B11100000,
  B00000000, B00000011, B10111111, B11100000,
  B00000000, B00000011, B11111111, B11100000,
  B00000000, B00000011, B11111111, B11100000,
  B00000000, B00000011, B11111111, B11100000,
  B00000000, B00000011, B11111011, B11100000,
  B00000000, B00000011, B11111000, B00000000,
  B00000000, B00000011, B11111111, B10000000,
  B00000000, B00000011, B11100000, B00000000,
  B10000000, B00000111, B11100000, B00000000,
  B10000000, B00011111, B11100000, B00000000,
  B11100000, B01111111, B11111100, B00000000,
  B11100000, B11111111, B11111100, B00000000,
  B11110001, B11111111, B11100100, B00000000,
  B11111111, B11111111, B11100000, B00000000,
  B11111111, B11111111, B11100000, B00000000,
  B11111111, B11111111, B11100000, B00000000,
  B01111111, B11111111, B11100000, B00000000,
  B00011111, B11111111, B11000000, B00000000,
  B00011111, B11111111, B11000000, B00000000,
  B00001111, B11111111, B10000000, B00000000,
  B00000011, B11111110, B00000000, B00000000,
  B00000001, B11100011, B10000000, B00000000,
  B00000001, B11100011, B10000000, B00000000,
  B00000001, B11000000, B00000000, B00000000,
  B00000001, B00000000, B00000000, B00000000,
  B00000001, B10000000, B00000000, B00000000,
  B00000001, B11000000, B00000000, B00000000
};

/**
   Made with Marlin Bitmap Converter
   https://marlinfw.org/tools/u8glib/converter.html

   This bitmap from the file 'dinomini2.png'
*/
/**
   Made with Marlin Bitmap Converter
   https://marlinfw.org/tools/u8glib/converter.html

   This bitmap from the file 'dinomini2.png'
*/
#pragma once

const unsigned char bitmap_dino2[] PROGMEM = {
  B00000000, B00000001, B11111111, B11000000,
  B00000000, B00000011, B11111111, B11000000,
  B00000000, B00000011, B10111111, B11100000,
  B00000000, B00000011, B10111111, B11100000,
  B00000000, B00000011, B11111111, B11100000,
  B00000000, B00000011, B11111111, B11100000,
  B00000000, B00000011, B11111111, B11100000,
  B00000000, B00000011, B11111011, B11100000,
  B00000000, B00000011, B11111000, B00000000,
  B00000000, B00000011, B11111111, B10000000,
  B00000000, B00000011, B11100000, B00000000,
  B10000000, B00000111, B11100000, B00000000,
  B10000000, B00011111, B11100000, B00000000,
  B11100000, B01111111, B11111100, B00000000,
  B11100000, B11111111, B11111100, B00000000,
  B11110001, B11111111, B11100100, B00000000,
  B11111111, B11111111, B11100000, B00000000,
  B11111111, B11111111, B11100000, B00000000,
  B11111111, B11111111, B11100000, B00000000,
  B01111111, B11111111, B11100000, B00000000,
  B00011111, B11111111, B11000000, B00000000,
  B00011111, B11111111, B11000000, B00000000,
  B00001111, B11111111, B00000000, B00000000,
  B00000011, B11111110, B00000000, B00000000,
  B00000001, B11001110, B00000000, B00000000,
  B00000001, B10000110, B00000000, B00000000,
  B00000000, B11100010, B00000000, B00000000,
  B00000000, B00000010, B00000000, B00000000,
  B00000000, B00000010, B00000000, B00000000,
  B00000000, B00000011, B00000000, B00000000
};




#pragma once

const unsigned char bitmap_sabo[] PROGMEM = {
  B00001100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B10000000,
  B11011100, B11000000,
  B11011100, B11000000,
  B11011100, B11000000,
  B11011100, B11000000,
  B11011100, B11000000,
  B11001100, B11000000,
  B11111111, B10000000,
  B01111111, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000,
  B00011100, B00000000
};


#define MAXSPEED 13
#define JUMPpower 7

double y;
int jump = 0;
int a = 0;

int oy[20];
int ox[20];
int n = 0;

int wx[10];
int wn = 0;

int co = 0;
int score;
int gamespeed = 7;
int hi;
int dinocos = 0;

void reset() {
  randomSeed(analogRead(A9));
  co = 0;
  n = 0;
  y = 34;
  wn = 0;
  a = 0;
  jump = 0;
  y = 34;
  score = 0;
  gamespeed = 6;
}

TIMER timer_dino;

void gameover() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(5, 15);
  display.print("GAME OVER");
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(65, 0);
  display.print("score:");
  display.setCursor(100, 0);
  display.print(score);
  display.display();

  display.setCursor(0, 0);
  display.print("HI:");
  display.setCursor(20, 0);
  hi = EEPROM.read(100) * 256 + EEPROM.read(101);
  if (score > hi) {
    hi = score;
    EEPROM.write(100, int(score / 256));
    EEPROM.write(101, int(score % 256));
  }
  display.print(hi);
  display.display();
  tone(buzzer, 255, 50);
  delay(100);
  tone(buzzer, 255, 50);
  delay(50);
  while (!SW1 && !SW2 && !SW3);
  if (SW1) {
    while (SW1);
    //standby();
  }
  delay(200);
  if (!SW2 == 0 && !SW1 == 0) {
    timer_dino.reset();
    while (timer_dino.get() <= 3000 || !(!SW2 == 0 && !SW1 == 0)) {
      ;
    }
    if (timer_dino.get() > 3000) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(5, 15);
      display.print("PASSWARD");
      display.display();
      while (!SW2 == 0 || !SW1 == 0);
      delay(100);
      while (!SW2 && !SW1);
      tone(buzzer, 1600, 50);
      if (!SW2 == 0) {
        while (!SW2 == 0 || !SW1 == 0);
        delay(100);
        while (!SW2 == 1 && !SW1 == 1);
        tone(buzzer, 1600, 50);
        if (!SW2 == 0) {
          while (!SW2 == 0 || !SW1 == 0);
          delay(100);
          while (!SW2 == 1 && !SW1 == 1);
          tone(buzzer, 1600, 50);
          if (!SW1 == 0) {
            while (!SW2 == 0 || !SW1 == 0);
            delay(100);
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(5, 15);
            display.print("CLEAR DATA");
            display.setCursor(5, 40);
            display.print("NO");
            display.setCursor(85, 40);
            display.print("YES");
            display.display();
            while (!SW2 == 1 && !SW1 == 1);
            if (!SW2 == 0) {
              EEPROM.write(100, 0);
              EEPROM.write(101, 0);
              tone(buzzer, 2000, 100);
              display.clearDisplay();
              display.setTextSize(2);
              display.setTextColor(WHITE);
              display.setCursor(5, 15);
              display.print("CLEAR DATA");
              display.setCursor(20, 45);
              display.print("FINISH");
              display.display();
              delay(500);

            }
          }
        }
      }
    }
  }
  reset();
}


void dinogame()
{
  display.setRotation(0);
  reset();
  while (1) {
    int nn;
    int rr;
    if ((SW2  || SW3) && jump == 0) {
      a = -JUMPpower;
      jump = 1;
      tone(buzzer, 600, 50);
    }

    y += a;
    display.clearDisplay();
    if (dinocos % 4 < 2) {
      display.drawBitmap(0, y, bitmap_dino, 27, 30, WHITE);
    } else {
      display.drawBitmap(0, y, bitmap_dino2, 27, 30, WHITE);
    }
    if (y == 34)dinocos++;
    display.drawLine(0, 62, 128, 62, WHITE);

    int l = random(0, 3);
    if (l == 0) {
      ox[n] = 128;
      oy[n] = random(62, 66);
      n++;
    }
    if (score > 2000) {
      nn = 18;
    }
    else if (score > 1000) {
      nn = 22;
    } else {
      nn = 25;
    }
    if (score % 500 == 0 && gamespeed < MAXSPEED)
      gamespeed++;

    if (score > 2000) {
      rr = 40;
    } else if (score > 1500) {
      rr = 30;
    } else if (score > 500) {
      rr = 35;
    } else {
      rr = 40;
    }
    int r = random(0, rr - co);
    if (r == 0 && co > nn) {
      wx[wn] = 128;
      wn++;
      co = 0;
      r = random(0, 3);
      if (r == 0) {
        wx[wn] = 138;
        wn++;
      }
      r = random(0, 4);
      if (r == 0 && score > 250) {
        wx[wn] = 148;
        wn++;
      }
      r = random(0, 5);
      if (r == 0 && score > 750) {
        wx[wn] = 158;
        wn++;
      }
      r = random(0, 3);
      if (r == 0 && score > 2500) {
        wx[wn] = 168;
        wn++;
      }
    }


    co++;

    for (int i = 0; i < n; i++) {
      ox[i] -= gamespeed;
      display.fillRect(ox[i], oy[i], 3, 1, WHITE);

    }

    if (ox[0] <= 0 && n > 0) {
      for (int i = 1; i < n; i++) {
        ox[i - 1] = ox[i];
        oy[i - 1] = oy[i];
      }
      n--;
    }

    for (int i = 0; i < wn; i++) {
      //display.fillRect(wx[i],62-20,10,20,WHITE);
      wx[i] -= gamespeed;
      display.drawBitmap(wx[i], 62 - 20, bitmap_sabo, 10, 20, WHITE);

    }

    if (wx[0] <= -5 && wn > 0) {
      for (int i = 1; i < wn; i++) {
        wx[i - 1] = wx[i];
      }
      wn--;
    }


    if (wn != 0 && wx[0] < 20 && y > 15) {
      gameover();
    }
    if (wn > 1 && wx[1] < 20 && y > 15) {
      gameover();
    }
    if (wn > 2 && wx[2] < 20 && y > 15) {
      gameover();
    }
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(65, 0);
    display.print("score:");
    display.setCursor(100, 0);
    display.print(score);

    display.display();
    //delay(1);

    if (a == JUMPpower) {
      a = 0;
      jump = 0;
    }
    if (jump != 0) {
      a++;
    }

    score += 1;

    delay(10);
  }
}
