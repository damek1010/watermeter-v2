#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
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

ESP8266WebServer server(80);

String getCurrentDate();

void saveMeasurement(int value);

void handleRoot();

void handleMeasurements();

void handleSettings();

void handleNotFound();

void sendFile(String filename, String mimetype = "text/html");

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

  server.on("/", handleRoot);                     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/measurements", handleMeasurements); // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/style.css", []() {
    sendFile("/web/style.css", "text/css");
  });
  server.on("/js/Chart.bundle.min.js", []() {
    sendFile("/web/js/Chart.bundle.min.js", "application/javascript");
  });
  server.onNotFound(handleNotFound); // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.on("/settings.html", handleSettings);
  server.on("/index.html", handleRoot);
  server.begin();
}

void loop()
{
  server.handleClient();

  // saveMeasurement(10);
  // delay(3000);
}

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

void handleRoot()
{
  sendFile("/web/index.html");
}

void handleMeasurements()
{
  String content;
  String filename = String(getCurrentDate());
  filename.concat(".csv");

  sendFile(filename, "text/plain");
}

void handleSettings()
{
  sendFile("/web/settings.html");
}

void handleNotFound()
{
  server.send(404, "text/html", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void sendFile(String filename, String mimetype)
{
  File file = SD.open(filename);
  String content = "";
  server.setContentLength(file.size());
  server.send(200, mimetype, "");
  int partSize = 0;
  while (file.available())
  {
    if (partSize < MAX_PART_SIZE)
    {
      content.concat((char)file.read());
      partSize++;
    }
    else
    {
      server.sendContent(content);
      partSize = 0;
      content.clear();
    }
  };
  if (partSize > 0)
  {
    server.sendContent(content);
  }

  file.close();
}