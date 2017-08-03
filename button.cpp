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

#include "button.h"

Button::Button( int pin )
{
  pinMode( pin, INPUT_PULLUP );
  this->pin = pin;
  ignoring = false;
  checkTime = 0;
  prevButton = NONE;
  debounceButton = false;
  lastEvent = NONE;
}

bool Button::Raw()
{
  return !digitalRead(pin);
}

// Check for and return a button press or release
Button::Event Button::CheckEvent()
{
  Event event = NONE;
  bool curButton = Raw();
  
  if (!ignoring) {
    if (curButton != prevButton) {
      ignoring = true;
      checkTime = micros() + 5000;
    }
    prevButton = curButton;
  } else {
    if (curButton != prevButton) {
      // Noise, reset the clock
      ignoring = false;
    } else if (micros() > checkTime) {
      if (curButton != debounceButton) {
        lastEvent = event = (debounceButton) ? RELEASE : PRESS;
        debounceButton = curButton;
      }
      ignoring = false;
    }
  }
  return event;
}

bool Button::Current()
{
  if (lastEvent == PRESS) return true;
  else return false;
}

