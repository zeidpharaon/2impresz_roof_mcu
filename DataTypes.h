#ifndef DataTypes_h
#define DataTypes_h

struct Date{
  byte second;
  byte minute;
  byte hour;
  byte day;
  byte month;
  byte year;
};

struct SweepData{
  byte channelNumber;
  struct Date date;
  float temperature;
  float humidity;
  float intensity;
  int sweepSize;
  float* voltages;
  float* currents;
};

struct SweepInfo{
  byte channelNumber;
  boolean on = false;
  float startVoltage;
  float endVoltage;
  float stepVoltage;
  int timeBetweenSweeps;//unit ??
};

struct ChannelInfo{
  struct SweepInfo sweepInfo;
  struct Date timeOfLastSweep;
};
#endif
