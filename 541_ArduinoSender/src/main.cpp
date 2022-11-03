#include <Arduino.h>
#include <perams.h>
#include <math.h>
#include <stdio.h>
#include <TimerOne.h>

float upTime,downTime;
int packet[8];
int output[12];

volatile int counter;
volatile int sequence;


void sendMessage(){
  if((sequence % 10) == 0){
    digitalWrite(dataPin,packet[counter%8]);
    counter++;
  }
  else{
    digitalWrite(dataPin, 0);
  }
  sequence++;
}


void setup() {
  // put your setup code here, to run once:
  pinMode(dataPin,OUTPUT);
  upTime = bitPeriod * 0.1;
  downTime = bitPeriod - upTime;
  Serial.begin(9600);
  for(int i = 0; i < 8; i++){
    packet[i] = 1;
  }
  counter = 0;
  sequence = 0;
  Timer1.initialize();
  Timer1.attachInterrupt(sendMessage,45);

  
}







void hammingCodeGenerate(){
  //Implement the hamming code
}



void loop() {
}