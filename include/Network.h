#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SD.h>
#include "routes.h"
#include "LcdLogger.h"
#include "Measurement.h"

struct Wifi_Network
{
    String ssid;
    String password;
};

Wifi_Network network;

bool ACCESS_POINT_WORING = false;

long initializationTimeStart, initializationTime;

void loadNetworkInformation();
void saveNetworkInformation(String ssid, String password);
bool wifiInit();
bool accessPointInit();

void networkInit()
{
    if (wifiInit())
    {
        initRtc();

        server.on("/", handleRoot);
        server.on("/measurements", handleMeasurements);
        server.on("/style.css", []() {
            sendFile("/web/style.css", "text/css");
        });
        server.onNotFound(handleNotFound);
        server.on("/settings", handleSettings);

        server.on("/settings.html", handleSettings);
        server.on("/savenetworksettings", handleSaveNetworkSettings);
        server.on("/savemeasurmentsettings", handleSaveMeasurementSettings);
        server.on("/saveinputsettings", handleSaveInputSettings);
        server.on("/index.html", handleRoot);

        server.on("/measurements/whole", [] {
            server.send(200, "text/plain", String(pulseCounter / PULSES_PER_LITER));
        });

        server.on("/measurements/day", handleDay);
        server.on("/measurements/month", handleMonth);
        server.on("/measurements/year", handleYear);

        server.on("/measurements/details", handleDetails);

        server.begin();
    }
    else
    {
        accessPointInit();
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

bool wifiInit()
{
    loadNetworkInformation();

    if (network.ssid.isEmpty())
    {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(200);

    WiFi.begin(network.ssid, network.password);

    initializationTimeStart = millis();

    bool change = false;

    while (WiFi.status() != WL_CONNECTED)
    {
        if (change)
        {
            lcdWrite("Connecting to", network.ssid.substring(0, 16));
            change = false;
        }
        else
        {
            lcdWrite("Connecting to", network.ssid.substring(0, 10) + "******");
            change = true;
        }
        delay(1000);
        Serial.print(".");

        initializationTime = millis();

        if (initializationTime - initializationTimeStart > MAX_WIFI_INTIALIZE_TIME)
        {
            lcdWrite("NW con. failed!", "Turning on AP");
            delay(2000);
            return false;
        }
    }
    lcdWrite("NW Connection", "successfull!");
    delay(2000);

    lcdWrite("NW " + network.ssid.substring(0, 13), WiFi.localIP().toString() + "/");

    return true;
}

bool accessPointInit()
{

    WiFi.mode(WIFI_AP);
    WiFi.disconnect();
    delay(200);

    WiFi.softAPConfig(localIp, gateway, subnet);
    WiFi.softAP(ACCESS_POINT_NETWORK_NAME);

    lcdWrite("AP " + ACCESS_POINT_NETWORK_NAME, ACCESS_POINT_IP_STRING);

    return true;
}

void loadNetworkInformation()
{
    if (!SD.exists(NETWORKFILE))
    {
        network.ssid = "";
        return;
    }

    File networkFile = SD.open(NETWORKFILE, FILE_READ);

    if (!networkFile)
    {
        network.ssid = "";
        return;
    }

    String ssid = networkFile.readStringUntil('\n');
    String password = networkFile.readStringUntil('\n');

    network.ssid = ssid;
    network.password = password;

    networkFile.close();
}

void saveNetworkInformation(String ssid, String password)
{
    SD.remove(NETWORKFILE);

    File networkFile = SD.open(NETWORKFILE, FILE_WRITE);

    networkFile.print(ssid);
    networkFile.print("\n");
    networkFile.print(password);
    networkFile.print("\n");

    networkFile.close();

    lcdWrite("Saving & resetin", ssid.substring(0, 16));
    delay(2000);

    networkFile.close();
}