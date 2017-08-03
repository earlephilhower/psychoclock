/*
  PsychoClock
  ESP8266 based alarm clock with automatic time setting and music!
  
  Copyright (C) 2017  Earle F. Philhower, III

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <i2s.h>

#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <Fonts/tahoma5pt7b.h>

#include "button.h"
#include "ntp.h"
#include "timezone.h"
#include "webspiffs.h"
#include "AudioGeneratorMP3.h"
#include "AudioFileSourceSPIFFS.h"
#include "AudioOutputI2SDAC.h"

// Define SSID_NAME and SSID_PASS
#include "./wifi.h"

Max72xxPanel matrix = Max72xxPanel(D1, D2, D3, 4, 1);
#define base 6

void SetupDisplay()
{
  matrix.setRotation(0,3);
  matrix.setRotation(1,3);
  matrix.setRotation(2,3);
  matrix.setRotation(3,3);

  matrix.setPosition(0,3,0);
  matrix.setPosition(1,2,0);
  matrix.setPosition(2,1,0);
  matrix.setPosition(3,0,0);

  matrix.setIntensity(7); // Use a value between 0 and 15 for brightness
  matrix.setFont(&tahoma5pt7b);
}

void SetupNetwork()
{
  // Make sure ESP isn't doing any wifi operations.  Sometimes starts back up in AP mode, for example
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  
  WiFi.hostname("psycho2");
  
  byte zero[] = {0,0,0,0};
  WiFi.config(zero, zero, zero, zero);

  WiFi.begin(WIFI_NAME, WIFI_SSID);

  // Try forever
  int cnt = 0;
  pinMode(BUILTIN_LED, OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    matrix.fillScreen(LOW);
    matrix.setCursor(0,0+base);
    matrix.setTextWrap(false);
    char buff[32];
    sprintf(buff, "WiFi:%d", cnt%10);
    cnt++;
    matrix.print(buff);
    matrix.write();
    delay(100);
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    ESP.restart(); // Punt, maybe we're in a weird way.  Reboot and try it again
  }
  StartNTP();
  
  SetTZ("America/Los_Angeles");
  SetupWebSPIFFS();
  
}

bool onlySetup = false;

Button butSnooze(D7);
Button butHour(D5);
Button butMin(D6);

typedef enum {
  CURTIME,
  CURALARM,
  SETALARM,
  ALARM
} State;

State curState = CURTIME;

AudioGeneratorMP3 *mp3 = NULL;
AudioFileSourceSPIFFS *file = NULL;
AudioOutputI2SDAC *out = NULL;

void setup()
{
  Serial.begin(115200);
  Serial.flush();
  SetupDisplay();
  SetupNetwork();
  if (butSnooze.Raw() && butHour.Raw() && butMin.Raw()) {
    Serial.println("Entering setup only mode!");
    onlySetup = true;
    matrix.fillScreen(LOW);
    matrix.setCursor(0,0+base);
    matrix.setTextWrap(false);
    matrix.print("-Setup-");
    matrix.write();
  }

  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}


int alarmHour = 0;
int alarmMin = 0;
bool alarmOn = false;


void PrintTime(char *buff, int h, int m, int s, bool solidColon)
{
  bool pm = (h > 11);
  if (h==0) h = 12;
  if (h>12) h-= 12;
  sprintf(buff, "%d%c%02d%s", h, (solidColon ||(s&1))?':':' ',m, pm?"pm":"am");
}

void AnimateTime(bool invert)
{
  time_t t = LocalTime(now());
  int h = hour(t);
  int m = minute(t);
  int s = second(t);

  matrix.fillScreen(invert?HIGH:LOW);
  matrix.setCursor(0,0+base);
  matrix.setTextWrap(false);
  char buff[32];
  PrintTime(buff, h, m, s, false);
  matrix.print(buff);
  if (invert) {
    matrix.setTextColor(LOW);
  } else {
    matrix.setTextColor(HIGH);
  }
  if (alarmOn) matrix.drawPixel(31, 0, invert?LOW:HIGH);
  Serial.println(buff);
  matrix.write();
}

void AnimateAlarm(bool alarmEdit)
{
  matrix.fillScreen(LOW);
  matrix.setCursor(0,0+base);
  matrix.setTextWrap(false);
  char buff[32] = {0};
  if (!alarmOn) {
    sprintf(buff, "Off");
  } else {
    PrintTime(buff, alarmHour, alarmMin, 0, true);
  }
  if (alarmEdit && ((millis()%1000) < 100)) buff[0]= 0; // Make it blink whilst editing
  matrix.print(buff);
  Serial.println(buff);
  matrix.write();
}



void loop()
{
  static time_t lastTime;
  State nextState;

  static int lastHr = 25;
  static int lastMin = 61;
  
//  httpServer.handleClient();
  HandleWebSPIFFS();
  if (onlySetup) return;

  if (mp3) {
    if (!mp3->loop()) {
      mp3->stop();
      delete file;
      delete out;
      delete mp3;
      mp3 = NULL;
      file = NULL;
      out = NULL;
    }
  }

  nextState = curState;
  time_t t = LocalTime(now());
  switch (curState) {
    case CURTIME:
      if ( (lastMin!= minute(t)) || (lastHr != hour(t) ) ) {
        if (alarmOn) {
          if ( (alarmHour == hour(t)) && (alarmMin == minute(t)) ) {
            nextState = ALARM;
            file = new AudioFileSourceSPIFFS("/JamOnIt.mp3");
            out = new AudioOutputI2SDAC();
            mp3 = new AudioGeneratorMP3();
            mp3->begin(file, out);
          }
        }
      }
      lastHr = hour(t);
      lastMin = minute(t);
      AnimateTime(false);
      if (butSnooze.CheckEvent()==Button::PRESS) {
        lastTime = now();
        nextState = CURALARM;
      }
      break;
      
    case CURALARM:
      AnimateAlarm(false);
      if (butSnooze.CheckEvent()==Button::RELEASE) {
        nextState = CURTIME;
      } else if (now()-lastTime > 3) {
        nextState = SETALARM;
      }
      break;
      
    case SETALARM:
      AnimateAlarm(true);
      if (butHour.CheckEvent()==Button::PRESS) {
        if (!alarmOn) {
          alarmOn = true;
          alarmHour = 0;
        } else {
          alarmHour++;
          if (alarmHour==24) alarmOn = false;
        }
      }
      if (butMin.CheckEvent()==Button::PRESS) {
        alarmMin = (alarmMin+1) % 60;
      }
      if (butSnooze.CheckEvent()==Button::PRESS) {
        nextState = CURTIME;
      }
      break;
      
    case ALARM:
      AnimateTime(true);
      time_t t = LocalTime(now());
      if ( (butSnooze.CheckEvent() == Button::PRESS) || (hour(t)!=alarmHour) || (minute(t)!=alarmMin)) {
        nextState = CURTIME;
        mp3->stop();
        delete file;
        delete out;
        delete mp3;
        mp3 = NULL;
        file = NULL;
        out = NULL;
      }
  }

  curState = nextState;
}

