#include <Arduino.h>
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>           //included with Arduino IDE (www.arduino.cc)
#include <SD.h>
#include <Wire.h>
#include "SHTSensor.h"
#include "Wireless.h"
#include "RTC.h"
#include "DataTypes.h"
#include "SDFIFO.h"

#define connectionLED 7
#define SDfail 8
#define button 3
#define NODEID ROOF
#define irradiancePin 0
#define sreg 32 //"chip select for shoft registers used to control the relays"
#define SELECTALL 99  //relays
#define UNSELECTALL 100 //relays

unsigned long lastMillisMeasure = 0;
unsigned long lastMillisSend = 0;

unsigned long interval = 1000;
/*unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 200;    // the debounce time; increase if the output flickers
boolean measure = false;*/

RFM69 radio;
SHTSensor sht;
SPISettings settingsA(14000000, MSBFIRST, SPI_MODE0);
//struct ChannelInfo testChannel;
struct ChannelInfo channels[16];

void measureChannel(struct ChannelInfo);
float getIrradience();
//float measureCurrent(float);
void initializeSweep();
boolean measureCurrent(float, float*);
boolean sendSweepData(String s);
boolean receiveData();
void updateChannel(struct SweepInfo);
boolean connect();
void intializeRelays();
void chooseModule(int);
//void getDate(struct Date* );
void printDate(struct Date*);
//void readSD(String);
//String readNext(File*);
void read();
void send();

void setup() {
  // put your setup code here, to run once:
  Wire.begin();

  //relays Intialization
  intializeRelays();
  chooseModule(1);

  pinMode(connectionLED, OUTPUT);
  pinMode(SDfail, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(38400);
  Serial.println("Start");

  //SD Intialization
  if (!SD.begin(42)) {
    Serial.println("initialization failed!");
    digitalWrite(SDfail, HIGH);
    return;
  }
  digitalWrite(SDfail, LOW);
  Serial.println("initialization done.");

  //Temerature and humidity initialization
  if (sht.init()) {
      Serial.print("sht init(): success\n");
  } else {
      Serial.print("sht init(): failed\n");
  }

  //RF module Intialization
  if(!radio.initialize(FREQUENCY, NODEID, NETWORKID)) Serial.println("rf fail");
  radio.setHighPower();

  //Iradience Intialization
  analogReference(DEFAULT);
  pinMode(irradiancePin, INPUT);


  /*testChannel.sweepInfo.startVoltage = 0;
  testChannel.sweepInfo.endVoltage = 1;
  testChannel.sweepInfo.stepVoltage = 0.1;*/

  /*pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), send, FALLING);*/

  /*channels[4].sweepInfo.channelNumber = 4;
  channels[4].sweepInfo.timeBetweenSweeps = 1;
  channels[4].sweepInfo.on = true;
  channels[4].sweepInfo.startVoltage = -1;
  channels[4].sweepInfo.endVoltage = 3;
  channels[4].sweepInfo.stepVoltage = 0.1;*/

  //pinMode(2, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(2), read, FALLING);

}

void loop() {
    if((millis() - lastMillisSend) > interval){
      lastMillisSend = millis();
      Serial.println("checking send");
      if(SDunsentData()){
        Serial.println("unsent data");
        if(connect()){
          Serial.println("conncted");
          String f = SDDequeue();
          Serial.println(f);
          if(sendSweepData(f)){
            Serial.println("good");
            SD.remove(f);
            if(!SDunsentData()){
              SDSetReadPos(FILE_NAME, 0);
              SDSetWritePos(FILE_NAME, 0);
            }
          }else{
            Serial.println("bad");
            SDReadDecreasePosition();
          }
        }else{
          Serial.println("not conncted");
        }
      }
    }

    if((millis() - lastMillisMeasure) > interval){
      lastMillisMeasure = millis();
      struct Date now;
      getDate(&now);
      printDate(&now);
      for(int i = 0; i < 16; i++){
        if(channels[i].sweepInfo.on == true){
          if(timePassed(channels[i].timeOfLastSweep, now, channels[i].sweepInfo.timeBetweenSweeps)){
            channels[i].timeOfLastSweep = now;
            //printDate(&now);
            //Serial.println("time ");// + String(i));
            measureChannel(channels[i]);
          }
        }
      }
    }

    if(radio.receiveDone()){
        if(receiveData()){
          //Serial.println("received update");
          //Serial.println(channels[0].sweepInfo.on);
          //Serial.println(channels[0].sweepInfo.startVoltage);
          //Serial.println(channels[0].sweepInfo.endVoltage);
          //Serial.println(channels[0].sweepInfo.stepVoltage);
          //Serial.println(channels[0].sweepInfo.timeBetweenSweeps);

        }else{
          //Serial.println("receive fail");
        }
        //Serial.println("received something");
      }
}



/*void send() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    Serial.println("button pressed");
    measure = true;
  }
}*/

void measureChannel(struct ChannelInfo c){
  //String testName = "test.txt";
  String temp = SDEnqueue();
  //Serial.println(temp);
  if(SD.exists(temp)){
    SD.remove(temp);
  }
  File f = SD.open(temp, FILE_WRITE);
  f.println(c.sweepInfo.channelNumber);

  struct Date now;
  getDate(&now);
  f.println(now.year);
  f.println(now.month);
  f.println(now.day);
  f.println(now.hour);
  f.println(now.minute);
  f.println(now.second);

  float temperature;
  float humidity;
  float intensity;
  if (sht.readSample()) {
      /*Serial.print("SHT:\n");
      Serial.print("  RH: ");
      Serial.print(sht.getHumidity(), 2);
      Serial.print("\n");
      Serial.print("  T:  ");
      Serial.print(sht.getTemperature(), 2);
      Serial.print("\n");*/
      temperature = sht.getTemperature();
      humidity = sht.getHumidity();
  } else {
      //Serial.print("Error in readSample()\n");
      temperature = -1;
      humidity = -1;
  }
  intensity = getIrradience();

  //float temperature = 22.1;
  f.println(temperature);
  //float humidity = 103.6;
  f.println(humidity);
  //float intensity = 52.1;
  f.println(intensity);

  int sweepSize = (c.sweepInfo.endVoltage - c.sweepInfo.startVoltage) / (c.sweepInfo.stepVoltage);
  f.println(sweepSize);
  Serial.println(sweepSize);
  Serial.println("start measurment");
  initializeSweep();
  chooseModule(c.sweepInfo.channelNumber);
  for(float i = c.sweepInfo.startVoltage ; i <= c.sweepInfo.endVoltage; i += c.sweepInfo.stepVoltage){
    //Serial.println("measuring " + String(i));
    float current;
    measureCurrent(i, &current);
    //current = measureCurrent(i);
    f.println(i);
    f.println(current, 5);// change 5

  }
  chooseModule(UNSELECTALL);
  f.close();
}

boolean sendSweepData(String s) {
  Serial.println("trying to open file");
  if(!SD.exists(s)){
    return false;
  }
  File f = SD.open(s);
  Serial.println("opened file");
  //f.close();
  //Send an empty array to make sure the other device can receive data,
  //if ack is sent back then this device becomes in the TX_mode else transmission fails
/*  if (radio.sendWithRetry(LAB, 0, 0, 5, 40)) {
    Serial.println("connection succeded");
  } else {
    Serial.println("connection failed");
    return false;
  }*/

  byte dataArray[61];

  //prepare the first packet to be sent which is the "header"
  intToArray(dataArray, 0, 0);
  byteToArray(dataArray, 0, 2);
  byteToArray(dataArray, SDReadNext(f).toInt(), 3);
  byteToArray(dataArray, SDReadNext(f).toInt(), 4);
  byteToArray(dataArray, SDReadNext(f).toInt(), 5);
  byteToArray(dataArray, SDReadNext(f).toInt(), 6);
  byteToArray(dataArray, SDReadNext(f).toInt(), 7);
  byteToArray(dataArray, SDReadNext(f).toInt(), 8);
  SDReadNext(f); //second red but dont send ?
  floatToArray(dataArray, SDReadNext(f).toFloat(), 9);
  floatToArray(dataArray, SDReadNext(f).toFloat(), 13);
  floatToArray(dataArray, SDReadNext(f).toFloat(), 17);
  int sweepSize = SDReadNext(f).toInt();
  intToArray(dataArray, sweepSize, 21);

  //try to send the first packet
  if (radio.sendWithRetry(LAB, dataArray, 23, 5, 40)) {
    Serial.println("sent header succesfully");
  } else {
    Serial.println("send fail");
    return false;
  }


  //7 voltages and 7 currents could be sent in each packet
  //sweepSize/7.0 calculates how many packets are needed to transimit the entire data
  //ceil is used to round up
  //7.0 is used instead of 7 because ceil() expects a float as an input
  for (int p = 0; p < ceil(sweepSize / 7.0); p++)
  {
    //data for each packet is prepared
    intToArray(dataArray, p+1, 0); //packet number is prepared p+1 is used because the first packet was the "header"
    byteToArray(dataArray, 0, 2); //
    int i;
    for (i = 0; i < 7; i++)//7 is the number of voltages and cuurents sent per packet
    {
      // 3 is the number of bytes sent in the begining of the packet,
      //8 is the number of bytes of 2 floates
      floatToArray(dataArray, SDReadNext(f).toFloat(), 3 + i * 8);
      floatToArray(dataArray, SDReadNext(f).toFloat(), 3 + i * 8 + 4);

      //checks if this is the last packet that usually hs less data,
      //if it is then it breaks out of the loop,
      //and it changes the 3rd byte of the packet to inform the reiver that this is the last packet
      if(i + p*7 + 1 >= sweepSize)
      {
        byteToArray(dataArray, TRANSMISSION_DONE, 2);
        Serial.println(i);
        break;
      }
    }

    //it trys to send the packet
    if (radio.sendWithRetry(LAB, dataArray, 3 + (i+1)*8, 5, 40)) {
      //Serial.println("sent sweep data succesfully");
    } else {
      //Serial.println("send fail");
      return false;
    }
  }
  f.close();
  //Serial.println("finished transmission");
  return true;
}

boolean connect(){
  if (radio.sendWithRetry(LAB, 0, 0, 5, 40)) {
    Serial.println("connection succeded");
    //pinMode(connectionLED, HIGH);
    return true;
  } else {
    Serial.println("connection failed");
    //pinMode(connectionLED, LOW);
    return false;
  }
}

boolean receiveData(){
  if(radio.DATALEN == 0){
    //continue
    sendAcknoledge();
  }else{
    return false;
  }

  if(!waitNewData()) return false;
  byte info = arrayToByte(radio.DATA, 2);
  if(info == SWEEPDATA){
    struct SweepInfo rec;
    rec.channelNumber = arrayToByte(radio.DATA, 3);
    rec.startVoltage = arrayToFloat(radio.DATA, 4);
    rec.endVoltage = arrayToFloat(radio.DATA, 8);
    rec.stepVoltage = arrayToFloat(radio.DATA, 12);
    rec.timeBetweenSweeps = arrayToInt(radio.DATA, 16);
    byte temp = arrayToByte(radio.DATA, 18);
    Serial.print("!!!");
    Serial.println(temp);
    if(temp == 1){
      rec.on = true;
    }else{
      rec.on = false;
    }

    sendAcknoledge();
    /*channel.sweepInfo.timeBetweenSweeps = rec.timeBetweenSweeps;
    channel.sweepInfo.on = rec.on;
    channel.sweepInfo.startVoltage = rec.startVoltage;
    channel.sweepInfo.endVoltage = rec.endVoltage;
    channel.sweepInfo.stepVoltage = rec.stepVoltage;*/
    updateChannel(rec);
    //Serial.println("receive sweep info");
  }else{
    //Serial.println("wrong info");
    return false;

  }
  return true;

}

void updateChannel(struct SweepInfo u){
  channels[u.channelNumber].sweepInfo.channelNumber = u.channelNumber;
  channels[u.channelNumber].sweepInfo.startVoltage = u.startVoltage;
  channels[u.channelNumber].sweepInfo.endVoltage = u.endVoltage;
  channels[u.channelNumber].sweepInfo.stepVoltage = u.stepVoltage;
  channels[u.channelNumber].sweepInfo.timeBetweenSweeps = u.timeBetweenSweeps;
  channels[u.channelNumber].sweepInfo.on = u.on;
}

void initializeSweep() {
  Serial1.println("*RST\r");
  //delay(del);
  Serial1.println("SOUR:FUNC VOLT\r");
  //delay(del);
  Serial1.println(":SOUR:VOLT:MODE FIXED\r");
  //delay(del);
  Serial1.println(":SOUR:VOLT:RANG 20\r");
  //delay(del);
  Serial1.println(":SENS:CURR:PROT 10E-1\r");
  //delay(del);
  Serial1.println(":SENS:FUNC \"CURR\"\r");
  //delay(del);
  Serial1.println(":SENS:CURR:RANG 10E-3\r");
  //delay(del);
  Serial1.println(":FORM:ELEM CURR\r");
  //delay(del);
  Serial1.println(":OUTP ON\r");
  //delay(del);
}

boolean measureCurrent(float volt, float* cur){
  Serial1.println(":SOUR:VOLT:LEV " + String(volt) + "\r");
  Serial1.println(":READ?\r");
  //receiveDone = false;
  unsigned long start = millis();
  while (!Serial1.available() && millis() - start < 1000) {}
  if(millis() - start > 1000){
    return false;
  }else{
    char temp[20];
    Serial1.readBytesUntil('\r', temp, 20);
    *cur = String(temp).toFloat();
    return true;
  }
}

float getIrradience(){
  return float(analogRead(irradiancePin)) * 5/1023 * 1000;
}

void intializeRelays(){
  pinMode(sreg, OUTPUT);
  digitalWrite(sreg, LOW);
  SPI.begin();
  SPI.beginTransaction(settingsA);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  digitalWrite(sreg, HIGH);
  digitalWrite(sreg, LOW);
  SPI.endTransaction();
}

void chooseModule(int value){
  if (value < 16){
    int pos = 1;
    if (value < 8){
      pos = pos << value;
      SPI.beginTransaction(settingsA);
      SPI.transfer(pos);
      SPI.transfer(pos);
      SPI.transfer(0x00);
      SPI.transfer(0x00);
      digitalWrite(sreg, HIGH);
      //delay(1);
      digitalWrite(sreg, LOW);
      SPI.endTransaction();
      //Serial.print(pos);
    }else{
      pos = pos << (value-8);
      SPI.beginTransaction(settingsA);
      SPI.transfer(0x00);
      SPI.transfer(0x00);
      SPI.transfer(pos);
      SPI.transfer(pos);
      digitalWrite(sreg, HIGH);
      //delay(1);
      digitalWrite(sreg, LOW);
      SPI.endTransaction();
      //Serial.print(pos);
    }
  }

  if (value == SELECTALL){
    SPI.beginTransaction(settingsA);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    digitalWrite(sreg, HIGH);
    //delay(1);
    digitalWrite(sreg, LOW);
    SPI.endTransaction();
    //erial.print("ALL");
    //Serial.print(value);
  }

  if (value == UNSELECTALL){
    SPI.beginTransaction(settingsA);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    digitalWrite(sreg, HIGH);
    //delay(1);
    digitalWrite(sreg, LOW);
    SPI.endTransaction();
    //Serial.print("ALL");
    //Serial.print(value);
  }

  if (value > 16){
    //return('error');
  }
}

/*float measureCurrent(float v){
  //change to keithley code
  float c;
  c = v / 10000;
  return c;
}*/

void printDate(struct Date* p){
  Serial.println("printin date");
  Serial.println(String(p->hour) + ':' + String(p->minute) + ':' + String(p->second) + '\t' + String(p->day) + '/' + String(p->month) + "/20" + String(p->year));
}

/*void getDate(struct Date* d){
  d->second = 00;
  d->minute = 21;
  d->hour = 12;
  d->day = 3;
  d->month = 12;
  d->year = 18;
}*/
