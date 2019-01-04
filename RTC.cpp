#include "RTC.h"
#include <Wire.h>
#include "DataTypes.h"
boolean getDate(struct Date* d){
  //Serial.println("in get date function");
  //struct Date ret;
  Wire.begin();

  Wire.beginTransmission(0x68);
  Wire.write(0x00); //address of second register;
  Wire.endTransmission();
  //Serial.println(Wire.requestFrom(0x68, 7));
  if(Wire.requestFrom(0x68, 7) != 7) return false;
  d->second = bcdToDec(Wire.read());
  d->minute = bcdToDec(Wire.read());
  d->hour = bcdToDec(Wire.read());
  //now.hours = bcdToDec(Wire.read() & 0b10111111);
  bcdToDec(Wire.read());
  d->day = bcdToDec(Wire.read());
  d->month = bcdToDec(Wire.read());
  d->year = bcdToDec(Wire.read());
  //Serial.println("leaving get date function");
  return true;
}

void setDate(struct Date d){
  Wire.beginTransmission(0x68);
  Wire.write(0x00); //address of second register;
  Wire.write(decToBcd(d.second));
  Wire.write(decToBcd(d.minute));
  Wire.write(decToBcd(d.hour));
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(d.day));
  Wire.write(decToBcd(d.month));
  Wire.write(decToBcd(d.year));
  Wire.endTransmission();
}

/*void printDate(struct Date p){
  //Serial.println("printin date");
  Serial.println(String(p.hour) + ':' + String(p.minute) + ':' + String(p.second) + '\t' + String(p.day) + '/' + String(p.month) + "/20" + String(p.year));
}*/

boolean timePassed(struct Date d1, struct Date d2, int m){
  //returns true if time between d1 and d2 is more than m minutes
  d1 = addMinutes(d1, m);
  if(earlierDate(d1, d2)){
    return true;
  }else{
    return false;
  }
}

boolean earlierDate(struct Date d1, struct Date d2){
  //returns true if first parapeter is an earlier date than the second
  if(d1.year < d2.year){
    return true;
  }else if(d1.year > d2.year){
    return false;
  }else if(d1.month < d2.month){//same year so compare month
    return true;
  }else if(d1.month > d2.month){
    return false;
  }else if(d1.day < d2.day){
    return true;
  }else if(d1.day > d2.day){
    return false;
  }else if(d1.hour < d2.hour){
    return true;
  }else if(d1.hour > d2.hour){
    return false;
  }else if(d1.minute < d2.minute){
    return true;
  }else if(d1.minute > d2.minute){
    return false;
  }else if(d1.second < d2.second){
    return true;
  }else if(d1.second > d2.second){
    return false;
  }else{
    return false;
  }
}

struct Date addMinutes(struct Date d, int m){
  struct Date ret;
  ret.second = d.second;
  ret.minute = (d.minute + m)%60;
  int minuteOverFlow = (d.minute + m)/60;
  ret.hour = (d.hour + minuteOverFlow)%24;
  int hourOverFlow = (d.hour + minuteOverFlow)/24;
  ret.day = ((d.day + hourOverFlow - 1)%daysInMonth(d.month, d.year)) + 1;
  int dayOverFlow = (d.day + hourOverFlow - 1)/daysInMonth(d.month, d.year);
  ret.month = ((d.month + dayOverFlow - 1)%12) + 1;
  int monthOverFlow = (d.month + dayOverFlow - 1)/(12);
  ret.year = d.year + monthOverFlow;

  return ret;
}

int daysInMonth(int m, int y){
  byte monthsLength[]= {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  //byte monthsLengthLeap[31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
  if(y % 4 == 0){
    //leap
    if(m == 2) return 29;
    else return monthsLength[m-1];
  }else{
    //not leap
    return monthsLength[m-1];
  }
}

uint8_t decToBcd(uint8_t dec){
  uint8_t bcd;
  bcd = ((dec / 10) << 4) | (dec % 10);
  return bcd;
}

uint8_t bcdToDec(uint8_t bcd){
  uint8_t dec;
  dec = 10*(bcd >> 4) + (bcd & 0x0F);
  return dec;
}
