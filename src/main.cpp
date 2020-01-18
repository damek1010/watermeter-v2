#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "Constants.h"
#include "TimeService.h"
#include <Wire.h>
#include "LcdLogger.h"
#include "Measurement.h"
#include "Config.h"
#include "Network.h"

unsigned long last_time_of_save = millis();

unsigned long time_millis;

static unsigned long last_interrupt_time = 0;

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

void savingRoutine();

bool sdInitialize();

bool interruptsInitialize();

void (*resetFunc)(void) = 0;

void wireSetup();

void errorLoop();

void setup()
{
  wireSetup();

  lcdSetup();

  if (!sdInitialize())
  {
    lcdWrite("SD Card Error!", "Check and reset");
    errorLoop();
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
    savePulsesPerLiter();
    resetFunc();
  }

  delay(0);

  if (MEASUREMENT_PERIOD_CHANGED)
  {
    saveMeasurementPeriod();
  }

  delay(0);

  if (PULSES_PER_LITER_CHANGED)
  {
    savePulsesPerLiter();
  }
}

void savingRoutine()
{
  delay(0);
  last_time_of_save = time_millis;

  saveMeasurement(pulseCounter, pulseCounter - lastMeasurement);

  saveCounterValue();
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

void errorLoop()
{
  while (1)
  {
    delay(100);
  }
}