#ifndef ROUTES_H
#define ROUTES_H

#include "Arduino.h"
#include <ESP8266WebServer.h>
#include <SD.h>
#include "Constants.h"
#include "TimeService.h"

ESP8266WebServer server(80);

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
        while (file.available() && ++partSize <= MAX_PART_SIZE )
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