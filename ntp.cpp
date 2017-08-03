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
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#include "ntp.h"
//#include "settings.h"
//#include "log.h"

static WiFiUDP ntpUDP;
static time_t GetNTPTime();
static void SendNTPPacket(IPAddress &address, byte *packetBuffer);

static short syncInterval = 600;

void StartNTP()
{
  // Enable NTP timekeeping
  ntpUDP.begin(8675); // 309
  setSyncProvider(GetNTPTime);
  setSyncInterval(syncInterval);
}

void StopNTP()
{
  ntpUDP.stop();
}


/*-------- NTP code ----------*/
/* Taken from the ESP8266 WebClient NTP sample */
#define NTP_PACKET_SIZE (48) // NTP time is in the first 48 bytes of message

static time_t GetNTPTime()
{
  IPAddress ntpServerIP; // NTP server's ip address
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

  while (ntpUDP.parsePacket() > 0) ; // discard any previously received packets
  // get a random server from the pool
  WiFi.hostByName("us.pool.ntp.org", ntpServerIP);
  SendNTPPacket(ntpServerIP, packetBuffer);
  int timeout = 1500; // Avoid issue of millis() rollover
  while (timeout--) {
    int size = ntpUDP.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer

#ifdef DEBUG_NTP
      for (int i=0; i<6; i++) {
        LogPrintf("NTP %02x: ", i*8);
        for (int j=0; j<8; j++) LogPrintf("%02x ", packetBuffer[i*8+j]);
        LogPrintf("\n");
      }
#endif

      unsigned char stratum = packetBuffer[1];
      if (stratum==0) {
        if (!memcmp_P(packetBuffer+12, PSTR("RATE"), 4)) {
//          LogPrintf("NTP RATE Kiss of Death, setting syncInterval to %d\n", syncInterval);
          syncInterval += 10;
          setSyncInterval(syncInterval);
        } else if (!memcmp_P(packetBuffer+12, PSTR("RSTR"), 4) || !memcmp_P(packetBuffer+12, PSTR("DENY"), 4)) {
//          LogPrintf("NTP DENY/RSTR Kiss of Death, disabling NTP requests from this IP\n");
          // TBD - should we clear it?  No way to notify user easily... settings.ntp[0] = 0;
        } else {
//          LogPrintf("NTP Kiss of Death, Unknown code: %c%c%c%c\n", packetBuffer[12], packetBuffer[13], packetBuffer[14], packetBuffer[15]);
        }
        return 0; // Some error occurred, can't get the time
      }

      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL /*70 years*/;
    } else
      delay(1);
  }
  return 0; // return 0 if unable to get the time
}


// send an NTP request to the time server at the given address
static void SendNTPPacket(IPAddress &address, byte *packetBuffer)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  ntpUDP.beginPacket(address, 123); //NTP requests are to port 123
  ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
  ntpUDP.endPacket();
}





