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


#ifndef _BUTTON_H
#define _BUTTON_H

#include <Arduino.h>

class Button {
  public:
    Button( int pin );
    
    typedef enum Event { NONE=0, PRESS, RELEASE } Event;
    
    bool Raw();          // Read the raw, undebounced state of any button
    Event CheckEvent();  // Return a single button event 
    bool Current();      // Current debounced state of any button

  private:
    uint8_t pin;
    bool ignoring;
    unsigned long checkTime;
    bool prevButton;
    bool debounceButton;
    uint8_t lastEvent;
};

#endif

