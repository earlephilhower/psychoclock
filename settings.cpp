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

#include <Arduino.h>
#include <EEPROM.h>
#include "settings.h"
#include "password.h"
#include "log.h"

static byte CalcSettingsChecksum();

Settings settings;


void StartSettings()
{
  EEPROM.begin(sizeof(settings)+2);
}

void StopSettings()
{
  EEPROM.commit();
  EEPROM.end();
}

bool LoadSettings(bool reset)
{
  bool ok = false;

  StartSettings();
  
  // Try and read from "EEPROM", if that fails use defaults
  byte *p = (byte *)&settings;
  for (unsigned int i=0; i<sizeof(settings); i++) {
    byte b = EEPROM.read(i);
    *(p++) = b;
  }
  byte chk = EEPROM.read(sizeof(settings));
  byte notChk = EEPROM.read(sizeof(settings)+1);

  byte calcChk = CalcSettingsChecksum();
  byte notCalcChk = ~calcChk;

  if ((chk != calcChk) || (notChk != notCalcChk) ||(settings.version != SETTINGSVERSION) || (reset)) {
    LogPrintf("Setting checksum mismatch, generating default settings\n");
    memset(&settings, 0, sizeof(settings));
    settings.version = SETTINGSVERSION;
    settings.isSetup = false;
    settings.ssid[0] = 0;
    settings.psk[0] = 0;
    strcpy_P(settings.hostname, PSTR("psychoplug"));
    settings.useDHCP = true;
    memset(settings.ip, 0, 4);
    memset(settings.dns, 0, 4);
    memset(settings.gateway, 0, 4);
    memset(settings.netmask, 0, 4);
    memset(settings.logsvr, 0, 4);
    strcpy_P(settings.ntp, PSTR("us.pool.ntp.org"));
    strcpy_P(settings.uiUser, PSTR("admin"));
    strcpy_P(settings.timezone, PSTR("America/Los_Angeles"));
    settings.use12hr = true;
    settings.usedmy = false;
    settings.alarmEnabled = false;
    HashPassword("admin", settings.uiSalt, settings.uiPassEnc);
    ok = false;
    LogPrintf("Unable to restore settings from EEPROM\n");
  } else {
    LogPrintf("Settings restored from EEPROM\n");
    ok = true;
  }

  StopSettings();

  return ok;
}

void SaveSettings()
{
  LogPrintf("Saving Settings\n");

  StartSettings();
  
  byte *p = (byte *)&settings;
  for (unsigned int i=0; i<sizeof(settings); i++) EEPROM.write(i, *(p++));
  byte ck = CalcSettingsChecksum();
  EEPROM.write(sizeof(settings), ck);
  EEPROM.write(sizeof(settings)+1, ~ck);
  EEPROM.commit();

  StopSettings();
}

static byte CalcSettingsChecksum()
{
  byte *p = (byte*)&settings;
  byte c = 0xef;
  for (unsigned int j=0; j<sizeof(settings); j++) c ^= *(p++);
  return c;
}

