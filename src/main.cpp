#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "Constants.h"
#include <SD.h>

const char *ssid = "912B";
const char *password = "splot123";

struct Measurement
{
  int value;
  String time;
};

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

File csv;

String getCurrentDate()
{
  time_t rawtime = timeClient.getEpochTime();
  struct tm ts = *localtime(&rawtime);
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &ts);
  Serial.println(buf);

  return buf;
}

void saveMeasurement(int value)
{
  /*
  *
  * Wczytywanie czasu
  * 
  */

  timeClient.update();

  /*
  *
  * Tworzenie pomiaru
  * 
  */
  Measurement measurement;
  measurement.time = timeClient.getFormattedTime();
  measurement.value = value;

  /*
  *
  * Zapisywanie pomiaru
  * 
  */
  Serial.print("Saving measurement: {\n\tvalue:");
  Serial.print(measurement.value);
  Serial.print(",\n\ttime: ");
  Serial.print(measurement.time);
  Serial.print("\n}");

  String filename = String(getCurrentDate());
  filename.concat(".csv");
  csv = SD.open(filename, FILE_WRITE);

  String buf = String(measurement.value);
  buf.concat(", ");
  buf.concat(measurement.time);

  csv.println(buf);
  csv.close();
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  Serial.print("Initializing SD card...");

  if (!SD.begin(PIN_CS))
  {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
}

void loop()
{
  saveMeasurement(10);
  delay(3000);
}