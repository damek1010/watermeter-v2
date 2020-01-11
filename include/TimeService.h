#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

String getCurrentDate()
{
  time_t rawtime = timeClient.getEpochTime();
  struct tm ts = *localtime(&rawtime);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &ts);
  //Serial.println(buf);

  return buf;
}

#endif