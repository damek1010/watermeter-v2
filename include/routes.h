#ifndef ROUTES_H
#define ROUTES_H

#include "Arduino.h"
#include <ESP8266WebServer.h>
#include <SD.h>
#include "Constants.h"
#include "TimeService.h"

ESP8266WebServer server(80);

bool ACCESS_POINT_SAVED_RESTART_NOW = false;

String access_point_saved_ssid, access_point_saved_password;

char buff[MAX_PART_SIZE + 1];
int partSize = 0;

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

    access_point_saved_ssid = server.arg("ssid");
    access_point_saved_password = server.arg("password");

    Serial.println(access_point_saved_ssid + " " + access_point_saved_password);

    // server.sendHeader("Location", "/save");
    displaySave();
    delay(3000);
    ACCESS_POINT_SAVED_RESTART_NOW = true;
    delay(1000);
}

void handleMeasurements()
{
    String content;
    String filename = String(getCurrentDate());
    filename.concat(".csv");

    String startDate = server.arg(0);
    String endDate = server.arg(1);

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

#endif