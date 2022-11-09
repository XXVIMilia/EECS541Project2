#include <Arduino.h>
#include <perams.h>
#include <math.h>
#include <stdio.h>
#include <TimerOne.h>

float upTime,downTime;
int packet[8];  //[char -> bits]
int output[14]; //[trigger(0),hammingcode(1-12),parity(13)]
int ham[12];
uint8_t data;

volatile int counter;
volatile int sequence;


void sendMessage(){
  if((sequence % 9) == 0){
    digitalWrite(dataPin,output[counter%14]);
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
  
  counter = 0;
  sequence = 0;
  Timer1.initialize();
  output[0] = 1;

  
}


// Function to calculate bit for
// ith position
int ham_calc(int position, int code_length){
  int count = 0, i, j;
  i = position - 1;

  // Traverse to store Hamming Code
  while (i < code_length) {
    for (j = i; j < i + position; j++) {
      // If current bit is 1
      if (ham[j] == 1)
          count++;
    }

    // Update i
    i = i + 2 * position;
  }

  if (count % 2 == 0){
    return 0;
  } 
  else{
    return 1;
  }
}




void hammingCodeGenerate(){
  //Implement the hamming code
  int k,j = 0;
  
  for(int i = 0; i < 12; i++){
    if(i == ((int)pow(2, k) - 1)){
      ham[i] = -1;
      k++;
    }
    else{
      ham[i] = packet[j]; 
      j++;
    }
  }

  // Traverse and update the
  // hamming code
  for (int i = 0; i < 4; i++) {

    // Find current position
    int position = (int)pow(2, i);

    // Find value at current position
    int value = ham_calc(position, 12);

    // Update the code
    ham[position - 1] = value;
  
  }

  // Print the Hamming Code
  Serial.print("The generated Code Word is: ");
    for(int i = 0; i < 12; i++) {
        Serial.print(ham[11-i]);
  }
  Serial.print('\n');

  }



void setOutput(uint8_t input){
  Serial.print("Transcribing to packet: ");
  for(int i = 0; i < 8; i++){
    packet[i] = (input >> i) & (0x01);
  }
  for(int i = 0; i < 8; i++){
    Serial.print(packet[7-i]);
  }
  Serial.print("\n");


}

char command;
bool ready_to_send = 0;

void loop() {
  Serial.flush();
  Serial.println("Send a command: 'p' = send pilot signal, 'c' = set character to deliver, \n'm' = send message,'e' = set bit error");
  while(Serial.available() < 1){}
  command = Serial.read();

  if(command == 'p'){
    Serial.println("Sending pilot signal");
      for(int i = 0; i < 14; i++){
        output[i] = 1;
    }
    counter = 0;
    sequence = 0;
    Timer1.attachInterrupt(sendMessage,50);
    delay(10000);
    Timer1.detachInterrupt();
    Serial.println("Done sending pilot signal");

  }
  else if(command == 'c'){
    Serial.println("Type your desired character");
    while(!Serial.available()){}
    Serial.readBytesUntil('\n',&data,1);
    Serial.print("Character Read in: ");
    Serial.println(data,BIN);
    setOutput(data);
    ready_to_send = 1;
    hammingCodeGenerate();

    
  }
  else if(command == 'm'){
    if(ready_to_send){
      counter = 0;
      Timer1.attachInterrupt(sendMessage,45);
      while(counter < 15){}
      Timer1.detachInterrupt();
      Serial.println("Transmission complete");
    }
    else{
      Serial.println("Set character first");
    }
    
  }
  else{
    Serial.println("Unknown character");
  }
  delay(500);
}