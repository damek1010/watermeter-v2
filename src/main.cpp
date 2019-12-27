#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "Constants.h"
#include "routes.h"
#include "TimeService.h"

const char *ssid = "";
const char *password = "";

uint32_t utime = 0;

uint32_t lastMeasurement = 0;

short dayNumber = 0;

bool end_of_work = false;

long last_time_of_save = millis();

long SAVE_PERIOD = 10000;

long time_millis;

uint32_t pulseCounter = 0;

static unsigned long last_interrupt_time = 0;

const long MAX_WIFI_INTIALIZE_TIME = 10000;
long initialization_time_start, initialization_time;

bool sd_initialization_result; 


void ICACHE_RAM_ATTR handleInterrupt()
{
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 100ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 100)
  {
    ++pulseCounter;
    //Serial.println(pulseCounter);

    Serial.println("writing to file");
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

File csv;

void saving_routine();

bool system_initialize();

bool sd_initialize();

bool interrupts_initialize();

bool wifi_initialize();

bool access_point_initialize();

void setup()
{

  system_initialize();

//TODO uzyc to do wyswietlenia komunikatu na stronach, ze z sd jest co nie tak
  sd_initialization_result = sd_initialize();

  interrupts_initialize();

  if (wifi_initialize())
  {
    timeClient.begin();

    server.on("/", handleRoot);
    server.on("/measurements", handleMeasurements);
    server.on("/style.css", []() {
      sendFile("/web/style.css", "text/css");
    });
    server.on("/js/Chart.bundle.min.js", []() {
      sendFile("/web/js/Chart.bundle.min.js", "application/javascript");
    });
    server.onNotFound(handleNotFound);
    server.on("/settings.html", handleSettings);
    server.on("/index.html", handleRoot);
    server.begin();
  }
  else
  {
    access_point_initialize();
  }
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

bool system_initialize()
{

  Serial.begin(115200);
  delay(10);

  system_update_cpu_freq(SYS_CPU_160MHZ);

  return true;
}

bool sd_initialize()
{

  Serial.print("Initializing SD card...");

  if (!SD.begin(PIN_CS))
  {
    Serial.println("initialization failed!");
    return false;
  }
  Serial.println("initialization done.");
  return true;
}

bool interrupts_initialize()
{

  pinMode(PIN_PULSE_COUNTER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_PULSE_COUNTER), handleInterrupt, FALLING);

  pinMode(PIN_TURN_OFF, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_TURN_OFF), stopProgram, FALLING);

  return true;
}

bool wifi_initialize()
{

  WiFi.begin(ssid, password);

  initialization_time_start = millis();

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    initialization_time = millis();

    if (initialization_time - initialization_time_start > MAX_WIFI_INTIALIZE_TIME)
    {
      Serial.println("Wifi initialization failed! Creating access point");
      return false;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  return true;
}

bool access_point_initialize(){


  return true;
}