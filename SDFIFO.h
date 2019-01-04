#ifndef SDFIFO_h
#define SDFIFO_h

#include <Arduino.h>
#include <SPI.h>           //included with Arduino IDE (www.arduino.cc)
#include <SD.h>

#define FILE_NAME "FIFO.txt"

String SDReadNext(File);
boolean SDunsentData();
String SDEnqueue();
String SDDequeue();
void SDReadDecreasePosition();
void SDSetReadPos(String, int);
void SDSetWritePos(String, int);
int SDGetReadPos(String);
int SDGetWritePos(String);
#endif
