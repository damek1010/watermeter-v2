#ifndef CONSTANTS_H
#define CONSTANTS_H

//other constants
const String NETWORKFILE = "network_informations.txt";
const String MEASUREMENT_PERIOD_FILE = "measurement_interval.txt";\
const String COUNTER_VALUE_FILE = "counter_value.txt";


const String ACCESS_POINT_NETWORK_NAME = "WatermeterAP";
const String ACCESS_POINT_IP_STRING = "192.168.1.1/";

const IPAddress localIp(192, 168, 1, 1);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

const long SAVE_PERIOD_MULTIPLIER = 1000*60;

const long MAX_WIFI_INTIALIZE_TIME = 30000;

const int lcdColumns = 16;
const int lcdRows = 2;

const int DEFAULT_SAVE_PERIOD = 10;


#define PIN_CLK 14
#define PIN_MISO 12
#define PIN_MOSI 13
#define PIN_CS 15

#define MAX_PART_SIZE 512


//CLOCK
#define PIN_DAT D4
#define PIN_RST D3

//LCD

#define PIN_SDA D2
#define PIN_SCL D1

//MEASURING_WATER
#define PIN_PULSE_COUNTER D0




#endif