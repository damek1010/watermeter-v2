#ifndef ROUTES_H
#define ROUTES_H

#include "Arduino.h"
#include <ESP8266WebServer.h>
#include <SD.h>
#include "Constants.h"
#include "TimeService.h"

ESP8266WebServer server(80);

bool NETWORK_CHANGED_RESTART_NOW = false;
bool MEASUREMENT_PERIOD_CHANGED = false;

unsigned long SAVE_PERIOD = DEFAULT_SAVE_PERIOD * SAVE_PERIOD_MULTIPLIER;

String saved_ssid, saved_password;

char buff[MAX_PART_SIZE + 1];
int partSize = 0;

char number_buff[20];

void sendFile(String filename, String mimetype = "text/html")
{
    File file = SD.open(filename);
    char *last = buff;
    partSize ^= partSize;

    server.setContentLength(file.size());
    server.send(200, mimetype, "");
    while (file.available())
    {
        while (file.available() && ++partSize <= MAX_PART_SIZE)
        {
            *(last++) = file.read();
        }
        *(last) = '\0';
        server.sendContent(buff);
        partSize ^= partSize;
        last = buff;
    }

    file.close();
}

void handleRoot()
{
    sendFile("/web/index.html");
}

void handleAPSettings()
{
    sendFile("/web/apsettings.html");
}

void displaySave()
{
    sendFile("/web/apsave.html");
}

void handleSaveAPSettings()
{

    saved_ssid = server.arg("ssid");
    saved_password = server.arg("password");

    //Serial.println(access_point_saved_ssid + " " + access_point_saved_password);

    // server.sendHeader("Location", "/save");
    displaySave();
    delay(3000);
    NETWORK_CHANGED_RESTART_NOW = true;
    delay(1000);
}

void handleSaveNetworkSettings()
{
    saved_ssid = server.arg("ssid");
    saved_password = server.arg("password");
    displaySave();

    NETWORK_CHANGED_RESTART_NOW = true;
}

void handleSaveMeasurementSettings()
{
    int period = server.arg("interval").toInt();

    if (period < 1 || period > 60)
    {
        SAVE_PERIOD = DEFAULT_SAVE_PERIOD * SAVE_PERIOD_MULTIPLIER;
    }
    else
    {
        SAVE_PERIOD = SAVE_PERIOD_MULTIPLIER * period;
    }
    MEASUREMENT_PERIOD_CHANGED = true;
    delay(100);
    sendFile("/web/changesaved.html");
}

void handleMeasurements()
{
    String content;
    String filename = "/data/" + String(getCurrentDate());
    filename.concat(".csv");

    String startDate = server.arg(0);
    String endDate = server.arg(1);

    sendFile(filename, "text/plain");
}

void handleSettings()
{
    sendFile("/web/settings.html");
}

void handleJS(String path)
{
    String fullpath = "/web" + path;
    //Serial.println("HHAHAHAHAHAHAHAHAHAHAHAHAHA");
    sendFile(fullpath, "application/javascript");
}

void handleCss(String path)
{

    String fullpath = "/web" + path;
    //Serial.println("HHAHAHAHAHAHAHAHAHAHAHAHAHA");
    sendFile(fullpath, "text/css");
}

void handleNotFound()
{
    String path = server.uri();
    //Serial.println(path);
    int index = path.indexOf("/js");
    //Serial.println("HHAHAHAHAHAHAHAHAHAHAHAHAHA");

    if (index >= 0)
    {
        handleJS(path);
        return;
    }

    index = path.indexOf("/css");
    if (index >= 0)
    {
        handleCss(path);
        return;
    }

    server.send(404, "text/html", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

////////getting data from sd card

uint32_t get_delta_from_record(String line)
{
    //HH:MM:SS,
    unsigned start = 9, end = 0;
    for (unsigned int i = start; i < line.length(); ++i)
    {
        delay(0);
        if (line.charAt(i) == ',')
        {
            end = i;
            break;
        }
    }
    delay(0);
    (line.substring(start, end)).toCharArray(number_buff, 20);
    delay(0);

    return strtoul(number_buff, NULL, 10);
}

String get_whole_day_data(String day)
{
    String filename = "/data/" + day;
    filename.concat(".csv");
    delay(0);
    File day_file = SD.open(filename);
    delay(0);
    if (!day_file)
    {
        return day + ",0";
    }
    uint32_t result = 0;
    while (day_file.available())
    {
        delay(0);
        filename = day_file.readStringUntil('\n');
        delay(0);
        if (filename.length() > 9)
        {
            result += get_delta_from_record(filename);
        }
    }
    delay(0);
    return String(day + "," + String(result));
}

uint32_t get_whole_day_data_uint32(String day)
{
    String filename = "/data/" + day;
    filename.concat(".csv");
    delay(0);
    File day_file = SD.open(filename);
    delay(0);
    if (!day_file)
    {
        return 0;
    }
    uint32_t result = 0;
    while (day_file.available())
    {
        delay(0);
        filename = day_file.readStringUntil('\n');
        delay(0);
        if (filename.length() > 9)
        {
            result += get_delta_from_record(filename);
        }
    }
    delay(0);
    return result;
}

String make_hour_from_int(int hour)
{
    if (hour < 10)
    {
        return String("0" + String(hour));
    }
    else
    {
        return String(hour);
    }
}

String get_hourly_day_data(String day)
{
    String result = "";

    File file = SD.open("/data/" + day + ".csv");
    if (!file)
    {
        for (int i = 0; i <= 23; ++i)
        {
            result.concat(make_hour_from_int(i));
            result.concat(",0\n");
        }
        return result;
    }

    int hour = 0;
    uint32_t hour_delta = 0;
    String hour_string = "00";
    String line;
    String hour_from_line;

    while (file.available())
    {
        delay(0);
        line = file.readStringUntil('\n');
        Serial.println(line);
        hour_from_line = line.substring(0, 2);
        Serial.println(hour_from_line + " " + hour_string);
        if (!hour_from_line.equals(hour_string))
        {
            Serial.println(make_hour_from_int(hour + 1));
            if (!hour_from_line.equals(make_hour_from_int(hour + 1)))
            {
                while (!hour_string.equals(hour_from_line))
                {
                    delay(0);
                    result.concat(hour_string);
                    result.concat(",");
                    result.concat(hour_delta);
                    result.concat("\n");
                    ++hour;
                    hour_delta = 0;
                    hour_string = make_hour_from_int(hour);
                    Serial.println(hour_string);
                    delay(250);
                }
            }
            else
            {
                delay(0);
                result.concat(hour_string);
                result.concat(",");
                result.concat(hour_delta);
                result.concat("\n");
                hour_string = hour_from_line;
                hour_delta = 0;
                ++hour;
            }
        }
        delay(0);
        hour_delta += get_delta_from_record(line);
    }

    delay(0);
    result.concat(hour_string);
    result.concat(",");
    result.concat(hour_delta);
    result.concat("\n");
    hour_string = hour_from_line;
    hour_delta = 0;
    ++hour;

    while (hour < 24)
    {
        delay(0);
        hour_string = make_hour_from_int(hour);

        result.concat(hour_string);
        result.concat(",");
        result.concat(0);
        result.concat("\n");
        ++hour;
    }

    return result;
}

//month like 01, 02 ...
String get_month_data_by_days(String year, String month)
{
    String beginning_date = year + "-" + month + "-";
    String file;
    String result = "";
    delay(0);
    for (int i = 1; i <= 31; ++i)
    {
        delay(0);
        if (i < 10)
        {
            file = beginning_date + "0" + i;
        }
        else
        {
            file = beginning_date + i;
        }
        delay(0);
        result.concat(get_whole_day_data(file));
        delay(0);
        result.concat("\n");
        delay(0);
    }
    return result;
}

uint32_t get_whole_month_uint32(String year, String month)
{
    String beginning_date = year + "-" + month + "-";
    String file;
    uint32_t result = 0;
    delay(0);
    for (int i = 1; i <= 31; ++i)
    {
        delay(0);
        if (i < 10)
        {
            file = beginning_date + "0" + i;
        }
        else
        {
            file = beginning_date + i;
        }
        delay(0);
        result += get_whole_day_data_uint32(file);
        delay(0);
    }
    return result;
}

void handleDay()
{
    String day = server.arg(0);
    delay(0);
    server.send(200, "text/plain", String(get_whole_day_data_uint32(day)));
}

void handleMonth()
{
    String year = server.arg(0);
    String month = server.arg(1);
    delay(0);
    server.send(200, "text/plain", String(get_whole_month_uint32(year, month)));
}

void handleDayHourly()
{
    String day = server.arg(0);
    delay(0);
    server.send(200, "text/plain", get_hourly_day_data(day));
}

void handleMonthDaily()
{
    String year = server.arg(0);
    String month = server.arg(1);
    delay(0);
    server.send(200, "text/plain", get_month_data_by_days(year, month));
}

#endif