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
#include <Hash.h>  //sha1 exported here
#include "password.h"


// Set the settings.uiPassEnc to the raw password and callthis to make a new salt and run encryption against it
// Output overwrites the uiPassEnc variable
void HashPassword(const char *pass, char *uiSalt, char *uiPassEnc)
{
  memset(uiSalt, 0, SALTLEN); // Clear salt to start
  memset(uiPassEnc, 0, PASSENCLEN); // Clear salt to start
  if (pass[0]==0) return; // No password
  for (unsigned int i=0; i<SALTLEN; i++)
    uiSalt[i] = RANDOM_REG32 & 0xff;

  // Now catenate the hash and raw password to temp storage
  char raw[128];
  memset(raw, 0, sizeof(raw));
  memcpy(raw, uiSalt, SALTLEN);
  strncpy(raw+SALTLEN, pass, 64);
  int len = strnlen(pass, 63)+1;
  sha1((uint8_t*)raw, SALTLEN+len, (uint8_t*)uiPassEnc);
  memset(raw, 0, sizeof(raw)); // Get rid of plaintext temp copy 
}

bool VerifyPassword(char *pass, const char *uiSalt, const char *uiPassEnc)
{
  // Now catenate the hash and raw password to temp storage
  char raw[128];
  memset(raw, 0, sizeof(raw));
  memcpy(raw, uiSalt, SALTLEN);
  strncpy(raw+SALTLEN, pass, 64);
  int len = strnlen(pass, 63)+1;
  while (*pass) { *(pass++) = 0; } // clear out sent-in 
  char dest[PASSENCLEN];
  sha1((uint8_t*)raw, SALTLEN+len, (uint8_t*)dest);
  memset(raw, 0, sizeof(raw));
  if (memcmp(uiPassEnc, dest, sizeof(dest))) return false;
  return true;
}

