#ifndef RTC_h
#define RTC_h

#include <Arduino.h>
#include <Wire.h>
#include "DataTypes.h"

boolean getDate(struct Date *);
void setDate(struct Date);
//void printDate(struct Date);
boolean timePassed(struct Date, struct Date, int);
boolean earlierDate(struct Date, struct Date);
struct Date addMinutes(struct Date , int);
int daysInMonth(int , int);
uint8_t decToBcd(uint8_t);
uint8_t bcdToDec(uint8_t);

#endif
