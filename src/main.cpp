#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "Constants.h"
#include "routes.h"
#include "TimeService.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//const char *ssid = "";
//const char *password = "";

const String NETWORKFILE = "network_informations.txt";

const String ACCESS_POINT_NETWORK_NAME = "WatermeterAP";

const String ACCESS_POINT_IP_STRING = "192.168.1.1/";

IPAddress localIp(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

String ssid, password;

bool ACCESS_POINT_WORING = false;

uint32_t utime = 0;

uint32_t lastMeasurement = 0;

long last_time_of_save = millis();

long SAVE_PERIOD = 10 * 1000;

long time_millis;

uint32_t pulseCounter = 0;

static unsigned long last_interrupt_time = 0;

const long MAX_WIFI_INTIALIZE_TIME = 20000;
long initialization_time_start, initialization_time;

bool sd_initialization_result;

int lcdColumns = 16;
int lcdRows = 2;
byte error, address;

LiquidCrystal_I2C lcd(0, 0, 0);

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

struct Measurement
{
  uint32_t value;
  uint32_t delta;
  String time;
};

struct Wifi_Network
{
  String ssid;
  String password;
};

Wifi_Network network;

File csv, network_file;

void saving_routine();

bool system_initialize();

bool sd_initialize();

bool interrupts_initialize();

bool wifi_initialize();

bool access_point_initialize();

void save_network_informations(String ssid, String password);

void get_network_informations();

void (*resetFunc)(void) = 0;

void wireSetup();

void lcdSetup();

void write_on_lcd(String first_line, String second_line = "");

void error_loop();

void setup()
{
  system_initialize();

  wireSetup();

  lcdSetup();

  if (!sd_initialize())
  {
    write_on_lcd("SD Card Error!", "Check and reset");
    error_loop();
  }

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
    server.on("/", handleAPSettings);
    server.on("/apsettings.html", handleAPSettings);
    server.on("/save", handleSaveAPSettings);
    server.on("/style.css", []() {
      sendFile("/web/style.css", "text/css");
    });
    server.onNotFound(handleNotFound);
    server.begin();
    ACCESS_POINT_WORING = true;
  }
}

void loop()
{
  server.handleClient();

  if (ACCESS_POINT_WORING)
  {
    while (1)
    {
      server.handleClient();
      delay(100);
      if (ACCESS_POINT_SAVED_RESTART_NOW)
      {
        save_network_informations(access_point_saved_ssid, access_point_saved_password);
        resetFunc();
      }
    }
  }

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
  buf.concat(",");
  buf.concat(measurement.time);
  buf.concat(",");
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
}

bool system_initialize()
{

  Serial.begin(115200);
  delay(10);

  //system_update_cpu_freq(SYS_CPU_160MHZ);

  return true;
}

bool sd_initialize()
{

  //Serial.print("Initializing SD card...");

  if (!SD.begin(PIN_CS))
  {
    //Serial.println("initialization failed!");
    return false;
  }
  //Serial.println("initialization done.");
  return true;
}

bool interrupts_initialize()
{

  pinMode(PIN_PULSE_COUNTER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_PULSE_COUNTER), handleInterrupt, FALLING);

  return true;
}

bool wifi_initialize()
{

  get_network_informations();

  if (network.ssid.isEmpty())
  {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);

  WiFi.begin(network.ssid, network.password);

  initialization_time_start = millis();

  bool change = false;

  while (WiFi.status() != WL_CONNECTED)
  {
    if (change)
    {
      write_on_lcd("Connecting to", network.ssid.substring(0, 16));
      change = false;
    }
    else
    {
      write_on_lcd("Connecting to", network.ssid.substring(0, 10) + "******");
      change = true;
    }
    delay(1000);
    Serial.print(".");

    initialization_time = millis();

    if (initialization_time - initialization_time_start > MAX_WIFI_INTIALIZE_TIME)
    {
      //Serial.println("Wifi initialization failed! Creating access point");
      write_on_lcd("NW con. failed!", "Turning on AP");
      delay(2000);
      return false;
    }
  }
  write_on_lcd("NW Connection","successfull!");
  delay(2000);

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
  write_on_lcd("NW " + network.ssid.substring(0, 13), WiFi.localIP().toString() + "/");

  return true;
}

bool access_point_initialize()
{

  WiFi.mode(WIFI_AP);
  WiFi.disconnect();
  delay(200);

  WiFi.softAPConfig(localIp, gateway, subnet);
  WiFi.softAP(ACCESS_POINT_NETWORK_NAME);

  //Serial.println("Acces Point is working");
  //Serial.print("Network ");
  //Serial.println(ACCESS_POINT_NETWORK_NAME);
  //Serial.println("Watermeter IP: " + localIp.toString());
  write_on_lcd("AP " + ACCESS_POINT_NETWORK_NAME, ACCESS_POINT_IP_STRING);

  return true;
}

void save_network_informations(String ssid, String password)
{

  SD.remove(NETWORKFILE);

  network_file = SD.open(NETWORKFILE, FILE_WRITE);

  network_file.print(ssid);
  network_file.print("\n");
  network_file.print(password);
  network_file.print("\n");

  network_file.close();

  write_on_lcd("Saving & resetin", ssid.substring(0, 16));
  delay(2000);
}

void get_network_informations()
{
  Serial.println(1);

  if (!SD.exists(NETWORKFILE))
  {
    network.ssid = "";
    return;
  }
  Serial.println(2);

  network_file = SD.open(NETWORKFILE, FILE_READ);
  Serial.println(3);

  ssid = network_file.readStringUntil('\n');
  password = network_file.readStringUntil('\n');
  Serial.println(4);

  network.ssid = ssid;
  network.password = password;
  Serial.println(6);
}

void wireSetup()
{

  Wire.begin(PIN_SDA, PIN_SCL);

  for (address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      break;
    }
  }
}

void lcdSetup()
{

  lcd = LiquidCrystal_I2C(address, lcdColumns, lcdRows);

  lcd.init(PIN_SDA, PIN_SCL);
  lcd.backlight();

  lcd.setCursor(0, 0);
  write_on_lcd("Turning on...");
  delay(1500);
}

//each string must be at most 16 characters (including whitespaces)!
void write_on_lcd(String first_line, String second_line)
{
  lcd.clear();
  delay(0);
  lcd.setCursor(0, 0);
  delay(0);
  lcd.print(first_line);
  delay(0);
  lcd.setCursor(0, 1);
  delay(0);
  lcd.print(second_line);
  delay(0);
}

void error_loop()
{
  while (1)
  {
    delay(100);
  }
}