#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
 
const char* ssid     = "912B";
const char* password = "splot123";

struct Measurement {
  int value;
  String time;
};

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

void saveMeasurement(int value) {
  timeClient.update();
  Measurement measurement;
  measurement.time = timeClient.getFormattedTime();
  measurement.value = value;
  Serial.print("Saving measurement: {\n\tvalue:");// +  +  + "\n}"
  Serial.print(measurement.value);
  Serial.print(",\n\ttime: ");
  Serial.print(measurement.time);
  Serial.print("\n}");
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
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