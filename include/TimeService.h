#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))

RtcDS3231<TwoWire> Rtc(Wire);

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

String getFormattedDateTime(const char *format)
{
  time_t rawtime = Rtc.GetDateTime().Epoch32Time();
  struct tm ts = *localtime(&rawtime);
  char buf[20];
  strftime(buf, sizeof(buf), format, &ts);

  return buf;
}

String getCurrentDate()
{
  return getFormattedDateTime("%Y-%m-%d");
}

String getCurrentTime()
{
  return getFormattedDateTime("%H:%M:%S");
}

void initRtc()
{
  timeClient.begin();
  timeClient.update();

  Rtc.Begin();

  char dateBuf[20];
  char timeBuf[10];
  time_t rawtime = timeClient.getEpochTime();
  struct tm ts = *localtime(&rawtime);
  strftime(dateBuf, sizeof(dateBuf), "%b %d %Y", &ts);
  strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &ts);

  RtcDateTime datetime = RtcDateTime(dateBuf, timeBuf);

  if (!Rtc.IsDateTimeValid())
  {
    if (Rtc.LastError() != 0)
    {
      Serial.print("RTC communications error = ");
      Serial.println(Rtc.LastError());
    }
    else
    {
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(datetime);
    }
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running. Starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < datetime)
  {
    Rtc.SetDateTime(datetime);
  }

  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void printDateTime(const RtcDateTime &dateTime)
{
  char dateString[20];
  snprintf_P(dateString,
             countof(dateString),
             PSTR("%02u/%02u/%04u %02u:%02u:02u"),
             dateTime.Month(),
             dateTime.Day(),
             dateTime.Year(),
             dateTime.Hour(),
             dateTime.Minute(),
             dateTime.Second());

  Serial.println(dateString);
}

#endif