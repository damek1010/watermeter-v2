#include <Arduino.h>
#include <SD.h>
#include "Constants.h"
#include "TimeService.h"
#include "Config.h"

struct Measurement
{
    uint32_t value;
    uint32_t delta;
    String time;
};

uint32_t lastMeasurement = 0;
uint32_t pulseCounter = 0;

void saveMeasurement(uint32_t value, uint32_t delta)
{
    /*
  *
  * Tworzenie pomiaru
  * 
  */

    Serial.println(delta);
    Serial.println(PULSES_PER_LITER);
    Serial.println(lastMeasurement);

    uint32_t validDelta = delta / PULSES_PER_LITER;

    uint32_t validValue = value / PULSES_PER_LITER;

    Serial.println(validDelta);

    Measurement measurement;
    measurement.time = getCurrentTime();
    measurement.value = validValue;
    measurement.delta = validDelta;

    /*
  *
  * Zapisywanie pomiaru
  * 
  */

    String filename = "/data/" + String(getCurrentDate());
    filename.concat(".csv");
    File csv = SD.open(filename, FILE_WRITE);

    //CHANGED!
    //now time,delta,value
    String buf = String(measurement.time);
    buf.concat(",");
    buf.concat(measurement.delta);
    buf.concat(",");
    buf.concat(measurement.value);

    csv.println(buf);
    csv.close();

    if (validDelta != 0)
    {
        lastMeasurement += validDelta * PULSES_PER_LITER;
    }
}

void loadCounterValue()
{
    if (!SD.exists(COUNTER_VALUE_FILE))
    {
        return;
    }

    File countervalueFile = SD.open(COUNTER_VALUE_FILE, FILE_READ);

    if (!countervalueFile)
    {
        return;
    }

    String counterValueAsString = countervalueFile.readStringUntil('\n');
    char counterValueBuff[20];

    counterValueAsString.toCharArray(counterValueBuff, 20);

    pulseCounter = strtoul(counterValueBuff, NULL, 10);

    if (pulseCounter == ULONG_MAX)
    {
        pulseCounter = 0;
    }

    pulseCounter *= PULSES_PER_LITER;

    lastMeasurement = pulseCounter;
}