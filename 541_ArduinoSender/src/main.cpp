#include <Arduino.h>
#include <perams.h>
#include <math.h>
#include <stdio.h>

float upTime,downTime;
int packet[8];
int output[12];



void setup() {
  // put your setup code here, to run once:
  pinMode(dataPin,OUTPUT);
  upTime = bitPeriod * dutyCycle;
  downTime = bitPeriod - upTime;
  Serial.begin(9600);
  for(int i = 0; i < 8; i++){
    packet[i] = 1;
  }
}


void sendMessage(){
  for(int i = 0; i < 11; i++ ){
    bool output = packet[i];
    digitalWrite(dataPin,output);
    delayMicroseconds(upTime);
    digitalWrite(dataPin,0);
    delayMicroseconds(downTime);
  }
}

void hammingCodeGenerate(){
  //Implement the hamming code
}



void loop() {
  sendMessage();
}