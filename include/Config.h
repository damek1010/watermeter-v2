#ifndef CONFIG_H
#define CONFIG_H

#include <SD.h>
#include "Constants.h"

int PULSES_PER_LITER = 1;
unsigned long SAVE_PERIOD = DEFAULT_SAVE_PERIOD * SAVE_PERIOD_MULTIPLIER;

void loadPulsesPerLiter();
void loadMeasurementPeriod();

void loadConfig()
{
    loadPulsesPerLiter();
    loadMeasurementPeriod();
}

void loadPulsesPerLiter()
{
    if (!SD.exists(PULSES_PER_LITER_FILE))
    {
        PULSES_PER_LITER = 1;
        return;
    }

    File pulsesPerLiterFile = SD.open(PULSES_PER_LITER_FILE, FILE_READ);

    if (!pulsesPerLiterFile)
    {
        PULSES_PER_LITER = 1;
        return;
    }

    String counterValueAsString = pulsesPerLiterFile.readStringUntil('\n');
    char counterValueBuff[20];

    counterValueAsString.toCharArray(counterValueBuff, 20);

    PULSES_PER_LITER = strtoul(counterValueBuff, NULL, 10);

    if (PULSES_PER_LITER == 0)
    {
        PULSES_PER_LITER = 1;
    }
}

void savePulsesPerLiter()
{
  SD.remove(PULSES_PER_LITER_FILE);
  delay(0);
  File pulsesPerLiterFile = SD.open(PULSES_PER_LITER_FILE, FILE_WRITE);
  delay(0);

  pulsesPerLiterFile.print(PULSES_PER_LITER);
  pulsesPerLiterFile.print("\n");

  pulsesPerLiterFile.close();
}

void loadMeasurementPeriod()
{

  if (!SD.exists(MEASUREMENT_PERIOD_FILE))
  {
    return;
  }

  File measurementPeriodFile = SD.open(MEASUREMENT_PERIOD_FILE, FILE_READ);

  if (!measurementPeriodFile)
  {
    return;
  }

  String measurementPeriodAsString = measurementPeriodFile.readStringUntil('\n');

  int period = measurementPeriodAsString.toInt();
  if (period < 1 * SAVE_PERIOD_MULTIPLIER || period > 60 * SAVE_PERIOD_MULTIPLIER)
  {
    return;
  }
  else
  {
    SAVE_PERIOD = period;
  }
}

void saveMeasurementPeriod()
{
  SD.remove(MEASUREMENT_PERIOD_FILE);
  delay(10);
  File measurementPeriodFile = SD.open(MEASUREMENT_PERIOD_FILE, FILE_WRITE);
  delay(0);

  measurementPeriodFile.print(SAVE_PERIOD);
  measurementPeriodFile.print("\n");

  measurementPeriodFile.close();
}

#endif