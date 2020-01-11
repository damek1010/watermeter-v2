#ifndef CONSTANTS_H
#define CONSTANTS_H

//other constants
const String NETWORKFILE = "network_informations.txt";
const String MEASUREMENT_PERIOD_FILE = "measurement_interval.txt";
const String COUNTER_VALUE_FILE = "counter_value.txt";
const String PULSES_PER_LITER_FILE = "pulses_per_liter.txt";

const String ACCESS_POINT_NETWORK_NAME = "WatermeterAP";

const String ACCESS_POINT_IP_STRING = "192.168.1.1/";

const IPAddress localIp(192, 168, 1, 1);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

const unsigned long SAVE_PERIOD_MULTIPLIER = 60000;

const long MAX_WIFI_INTIALIZE_TIME = 30000;

const int lcdColumns = 16;
const int lcdRows = 2;

const int DEFAULT_SAVE_PERIOD = 10;


//SD_CARD
#define PIN_MISO D6
#define PIN_MOSI D7
#define PIN_CS D8

#define MAX_PART_SIZE 512

#define PIN_CLK D5

//CLOCK
#define PIN_DAT D4
#define PIN_RST D0 

//LCD

#define PIN_SDA D2
#define PIN_SCL D1

//MEASURING_WATER
#define PIN_PULSE_COUNTER D3




#endif