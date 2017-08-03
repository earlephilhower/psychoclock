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

#ifndef _log_h
#define _log_h

// Start and stop logging infrastructure
void StartLog();
void StopLog();

// Write string to the log (serial or UDP)
extern void Log(const char *str);

// Ease-of-use to send formatted output
#define LogPrintf(fmt, ...) { char buff[256]; snprintf_P(buff, sizeof(buff), PSTR(fmt), ## __VA_ARGS__); Log(buff); }

#endif
