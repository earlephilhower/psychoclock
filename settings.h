/*
  PsychoPlug
  ESP8266 based remote outlet with standalone timer and MQTT integration
  
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

#ifndef _settings_h
#define _settings_h

#include <Arduino.h>
#include "password.h"

#define SETTINGSVERSION (1)

typedef struct {
  byte version;
  bool isSetup;
  
  char ssid[32];
  char psk[32];
  char hostname[32];
  bool useDHCP;
  byte ip[4];
  byte dns[4];
  byte gateway[4];
  byte netmask[4];
  byte logsvr[4];
  char ntp[48];
  bool use12hr;
  bool usedmy;
  char timezone[32];

  // Web Interface
  char uiUser[32];
  char uiPassEnc[PASSENCLEN];
  char uiSalt[SALTLEN];

  // Alarm info
  bool alarmEnabled;
  int alarmHour;
  int alarmMinute;
  char alarmSound[32];
} Settings;
extern Settings settings;

void StartSettings();
bool LoadSettings(bool reset);
void SaveSettings();
void StopSettings();

#endif

