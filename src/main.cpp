#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "Constants.h"
#include <SD.h>

const char *ssid = "DESKTO";
const char *password = "23X5q)23";

uint32_t utime = 0;

uint32_t lastMeasurement = 0;

short dayNumber = 0;

bool end_of_work = false;

long last_time_of_save = millis();

long SAVE_PERIOD = 10000;

long time_millis;

uint32_t pulseCounter = 0;

static unsigned long last_interrupt_time = 0;

void ICACHE_RAM_ATTR handleInterrupt()
{
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 100ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 100)
  {
    ++pulseCounter;
    //Serial.println(pulseCounter);

    //Serial.println("writing to file");
  }
  last_interrupt_time = interrupt_time;
}

void ICACHE_RAM_ATTR stopProgram()
{
  //Serial.println("Disabling Watermeter");
  end_of_work = true;
}

struct Measurement
{
  uint32_t value;
  uint32_t delta;
  String time;
};

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

File csv;

ESP8266WebServer server(80);

String getCurrentDate();

void saveMeasurement(uint32_t value, uint32_t delta);

void handleRoot();

void handleMeasurements();

void handleSettings();

void handleNotFound();

void sendFile(String filename, String mimetype = "text/html");

void saving_routine();

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

  pinMode(PIN_PULSE_COUNTER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_PULSE_COUNTER), handleInterrupt, FALLING);

  pinMode(PIN_TURN_OFF, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_TURN_OFF), stopProgram, FALLING);

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

  time_millis = millis();

  if (time_millis - last_time_of_save > SAVE_PERIOD)
  {
    saving_routine();
  }

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

void saveMeasurement(uint32_t value, uint32_t delta)
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
  measurement.delta = delta;

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
  buf.concat(", ");
  buf.concat(measurement.delta);

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

void saving_routine()
{
  delay(0);
  last_time_of_save = time_millis;

  //if day chnged, create new file
  // if (ts.getDayNumber() != dayNumber)
  // {
  //   // dayNumber = TimeService::getInstance().getDayNumber();
  // }

  saveMeasurement(pulseCounter, pulseCounter - lastMeasurement);

  utime += 3;
  Serial.println("4");

  lastMeasurement = pulseCounter;

  if (end_of_work)
  {
    noInterrupts();

    csv.close();
    while (1)
    {
      delay(1000);
    }
  }
}