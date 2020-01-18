#ifndef LCD_LOGGER_H
#define LCD_LOGGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Constants.h"

LiquidCrystal_I2C* lcd;

void lcdWrite(String firstLine, String secondLine = "");

void lcdSetup()
{
  lcd = new LiquidCrystal_I2C(0x27, 16, 2);

  lcd->init(PIN_SDA, PIN_SCL);
  lcd->backlight();

  lcd->setCursor(0, 0);
  lcdWrite("Turning on...");
  delay(1500);
}

void lcdWrite(String first_line, String second_line)
{
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(first_line);
  lcd->setCursor(0, 1);
  lcd->print(second_line);
}

#endif