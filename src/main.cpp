#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "Constants.h"
#include "TimeService.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS3231.h>
#include "LcdLogger.h"
#include "Measurement.h"
#include "Config.h"
#include "Network.h"

String ssid, password;

String measurement_period_as_string, counter_value_as_string, date_refresh;

unsigned long last_time_of_save = millis();

unsigned long time_millis;

static unsigned long last_interrupt_time = 0;

// long initialization_time_start, initialization_time;

bool sd_initialization_result;

byte error, address;

unsigned long interrupt_time;

void ICACHE_RAM_ATTR handleInterrupt()
{
  interrupt_time = millis();
  // If interrupts come faster than 100ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 100)
  {
    ++pulseCounter;
  }
  last_interrupt_time = interrupt_time;
}

// struct Wifi_Network
// {
//   String ssid;
//   String password;
// };

// Wifi_Network network;

File csv, network_file, measurementperiod_file, countervalue_file, pulses_per_liter_f;

void savingRoutine();

bool sdInitialize();

bool interruptsInitialize();

// bool wifi_initialize();

// bool access_point_initialize();

// void save_network_informations(String ssid, String password);
// void get_network_informations();

void (*resetFunc)(void) = 0;

void wireSetup();

void error_loop();

void save_measurement_period();
// void get_measurement_period();

// void getCounterValue();
void save_counter_value();

// void get_pulses_per_liter();
void save_pulses_per_liter();

void setup()
{
  wireSetup();

  lcdSetup();

  if (!sdInitialize())
  {
    lcdWrite("SD Card Error!", "Check and reset");
    error_loop();
  }

  loadConfig();

  loadCounterValue();

  interruptsInitialize();

  networkInit();
}

void loop()
{
  server.handleClient();

  time_millis = millis();

  if (time_millis - last_time_of_save > SAVE_PERIOD)
  {
    savingRoutine();
    delay(0);
    if (WiFi.status() == WL_CONNECTED)
    {
      lcdWrite("NW " + network.ssid.substring(0, 13), WiFi.localIP().toString() + "/");
    }
  }
  delay(0);

  if (NETWORK_CHANGED_RESTART_NOW)
  {
    saveNetworkInformation(saved_ssid, saved_password);
    resetFunc();
  }

  if (AP_NETWORK_CHANGED_RESTART_NOW)
  {
    saveNetworkInformation(saved_ssid, saved_password);
    save_pulses_per_liter();
    resetFunc();
  }

  delay(0);

  if (MEASUREMENT_PERIOD_CHANGED)
  {
    save_measurement_period();
  }

  delay(0);

  if (PULSES_PER_LITER_CHANGED)
  {
    save_pulses_per_liter();
  }
}

void savingRoutine()
{
  delay(0);
  last_time_of_save = time_millis;

  saveMeasurement(pulseCounter, pulseCounter - lastMeasurement);

  save_counter_value();
}

bool sdInitialize()
{
  if (!SD.begin(PIN_CS))
  {
    return false;
  }
  return true;
}

bool interruptsInitialize()
{
  pinMode(PIN_PULSE_COUNTER, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_PULSE_COUNTER), handleInterrupt, FALLING);

  return true;
}

// bool wifiInitialize()
// {
//   get_network_informations();

//   if (network.ssid.isEmpty())
//   {
//     return false;
//   }

//   WiFi.mode(WIFI_STA);
//   WiFi.disconnect();
//   delay(200);

//   WiFi.begin(network.ssid, network.password);

//   initialization_time_start = millis();

//   bool change = false;

//   while (WiFi.status() != WL_CONNECTED)
//   {
//     if (change)
//     {
//       lcdWrite("Connecting to", network.ssid.substring(0, 16));
//       change = false;
//     }
//     else
//     {
//       lcdWrite("Connecting to", network.ssid.substring(0, 10) + "******");
//       change = true;
//     }
//     delay(1000);
//     Serial.print(".");

//     initialization_time = millis();

//     if (initialization_time - initialization_time_start > MAX_WIFI_INTIALIZE_TIME)
//     {
//       lcdWrite("NW con. failed!", "Turning on AP");
//       delay(2000);
//       return false;
//     }
//   }
//   lcdWrite("NW Connection", "successfull!");
//   delay(2000);

//   lcdWrite("NW " + network.ssid.substring(0, 13), WiFi.localIP().toString() + "/");

//   return true;
// }

// bool access_point_initialize()
// {

//   WiFi.mode(WIFI_AP);
//   WiFi.disconnect();
//   delay(200);

//   WiFi.softAPConfig(localIp, gateway, subnet);
//   WiFi.softAP(ACCESS_POINT_NETWORK_NAME);

//   lcdWrite("AP " + ACCESS_POINT_NETWORK_NAME, ACCESS_POINT_IP_STRING);

//   return true;
// }

// void save_network_informations(String ssid, String password)
// {

//   SD.remove(NETWORKFILE);

//   network_file = SD.open(NETWORKFILE, FILE_WRITE);

//   network_file.print(ssid);
//   network_file.print("\n");
//   network_file.print(password);
//   network_file.print("\n");

//   network_file.close();

//   lcdWrite("Saving & resetin", ssid.substring(0, 16));
//   delay(2000);
// }

// void get_network_informations()
// {
//   if (!SD.exists(NETWORKFILE))
//   {
//     network.ssid = "";
//     return;
//   }

//   network_file = SD.open(NETWORKFILE, FILE_READ);

//   if (!network_file)
//   {
//     network.ssid = "";
//     return;
//   }

//   ssid = network_file.readStringUntil('\n');
//   password = network_file.readStringUntil('\n');

//   network.ssid = ssid;
//   network.password = password;
// }

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

void error_loop()
{
  while (1)
  {
    delay(100);
  }
}

void save_measurement_period()
{
  SD.remove(MEASUREMENT_PERIOD_FILE);
  delay(10);
  measurementperiod_file = SD.open(MEASUREMENT_PERIOD_FILE, FILE_WRITE);
  delay(0);

  measurementperiod_file.print(SAVE_PERIOD);
  measurementperiod_file.print("\n");

  measurementperiod_file.close();
}

void save_counter_value()
{

  SD.remove(COUNTER_VALUE_FILE);
  delay(0);
  countervalue_file = SD.open(COUNTER_VALUE_FILE, FILE_WRITE);
  delay(0);

  countervalue_file.print(pulseCounter / PULSES_PER_LITER);
  countervalue_file.print("\n");

  countervalue_file.close();
}

void save_pulses_per_liter()
{

  SD.remove(PULSES_PER_LITER_FILE);
  delay(0);
  pulses_per_liter_f = SD.open(PULSES_PER_LITER_FILE, FILE_WRITE);
  delay(0);

  pulses_per_liter_f.print(PULSES_PER_LITER);
  pulses_per_liter_f.print("\n");

  pulses_per_liter_f.close();
}