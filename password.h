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

#ifndef _password_h
#define _password_h

#define PASSENCLEN (20)
#define SALTLEN (32)

// Set the settings.uiPassEnc to the raw password and callthis to make a new salt and run encryption against it
// Output overwrites the uiPassEnc variable
void HashPassword(const char *pass, char *uiSalt, char *uiPassEnc);

// Check that entered password matches the hash in settings
bool VerifyPassword(char *pass, const char *uiSalt, const char *uiPassEnc);

#endif

