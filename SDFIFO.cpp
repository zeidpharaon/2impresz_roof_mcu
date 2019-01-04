#include "SDFIFO.h"

String SDReadNext(File f){
  String inputString = "";
  char c = f.read();
  while(c != '\n' && c != '\r'){
    inputString += c;
    c = f.read();
  }
  f.seek(f.position() + 1);
  return inputString;
}

boolean SDunsentData(){
  int w = SDGetWritePos(FILE_NAME);
  int r = SDGetReadPos(FILE_NAME);
  //uint32_t w = 1;
  //uint32_t r = 5;
  if(w > r){
    return true;
  }else{
    return false;
  }
}

String SDEnqueue(){
  int next = SDGetWritePos(FILE_NAME);
  SDSetWritePos(FILE_NAME, next+1);
  String temp = String(next) + ".txt";
  return temp;
}

String SDDequeue(){
  if(SDunsentData()){
    int next = SDGetReadPos(FILE_NAME);
    SDSetReadPos(FILE_NAME, next+1);
    return String(next) + ".txt";
  }else{
    return "";
  }
}

void SDReadDecreasePosition(){
  int next = SDGetReadPos(FILE_NAME);
  SDSetReadPos(FILE_NAME, next-1);
}

void SDSetReadPos(String n, int pos){
  int writePos = SDGetWritePos(n);
  if(SD.exists(n)) SD.remove(n);
  File f = SD.open(n, FILE_WRITE);
  f.println(pos);
  f.println(writePos);
  f.close();
}

void SDSetWritePos(String n, int pos){
  int readPos = SDGetReadPos(n);
  if(SD.exists(n)) SD.remove(n);
  File f = SD.open(n, FILE_WRITE);
  f.println(readPos);
  f.println(pos);
  f.close();
}

int SDGetReadPos(String n){
  File f = SD.open(n, FILE_READ);

  int ret =  SDReadNext(f).toInt();
  f.close();
  return ret;
}

int SDGetWritePos(String n){
  File f = SD.open(n, FILE_READ);
  SDReadNext(f);

  int ret =  SDReadNext(f).toInt();
  f.close();
  return ret;
}
