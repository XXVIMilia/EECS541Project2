#include <Arduino.h>
#include <perams.h>
#include <stdio.h>
#include <TimerOne.h>

float upTime,downTime;
int packet[8];  //[char -> bits]
volatile bool output[15]; //[trigger(0),hammingcode(1-13),parity(14)]
int ham[12];
uint8_t data;

volatile int counter;
volatile int sequence;


volatile bool sending;
void sendMessage(){
  if(sending){
    if((sequence % 10) == 0){
    digitalWrite(dataPin,output[counter%14]);
    counter++;
  }
  else{
    digitalWrite(dataPin, 0);
  }
  sequence++;
  }
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
  int i, p_n = 0, c_l, j, k;
  i = 0;

  while (8 > (int)(1 << i) - (i + 1)) {
    p_n++;
    i++;
  }
  c_l = p_n + 8;

  j = k = 0;
  
  for(int i = 0; i < 12; i++){
    if(i == (int)(1 << k) - 1){

      ham[i] = -1;
      k++;
    }
    else{
      ham[i] = packet[j]; 
      j++;
    }
  }

  Serial.print("Redundant locations: ");
  for(int i = 0; i < 12; i++) {
      Serial.print(ham[11-i],DEC);
  }
    Serial.print('\n');

 

  // Traverse and update the
  // hamming code
  for (int i = 0; i < p_n; i++) {

    // Find current position
    int position = (int)(1 << i);

    // Find value at current position
    int value = ham_calc(position, c_l);

    // Update the code
    ham[position - 1] = value;
  
  }

  // Print the Hamming Code
  Serial.print("The generated Code Word is: ");
    for(int i = 0; i < 12; i++) {
        Serial.print(ham[11-i]);
        output[i+1] = ham[11-i];
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
bool parity;
int ones = 0;
int bitFlip_i = -1;
void loop() {
  Serial.flush();
  Serial.println("Send a command: 'p' = send pilot signal, 'c' = set character to deliver, 'm' = send message,'e' = set bit error");
  while(Serial.available() < 1){}
  command = Serial.read();

  if(command == 'p'){
    Serial.println("Sending pilot signal");
      for(int i = 0; i < 15; i++){
        output[i] = 1;
    }
    counter = 0;
    sequence = 0;
    Timer1.attachInterrupt(sendMessage,45);
    sending = 1;
    delay(1000);
    sending = 0;
    
    Serial.println("Done sending pilot signal");

  }
  else if(command == 'c'){
    Serial.println("Type your desired character");
    while(!Serial.available()){}
    Serial.readBytesUntil('\n',&data,1);
    Serial.print("Character Read in: ");
    Serial.print((char)data);
    Serial.print(", 0b");
    Serial.println(data,BIN);
    setOutput(data);
    ready_to_send = 1;
    hammingCodeGenerate();
    output[0] = 1;

    
    ones = 0;
    for(int i = 1; i < 13 ; i++){
      if(output[i]){
        ones++;
      }
    }

    if(ones % 2){
      parity = 0;
    }
    else{
      parity = 1;
    }

    output[13] = parity;
    Serial.print("Generated Output: ");
    for(int i = 0; i < 14 ; i++){
      Serial.print(output[i]);
    }
    Serial.print("\n");
    
  }
  else if(command == 'm'){
    if(bitFlip_i != -1){
      Serial.print("adding error to index: ");
      Serial.println(bitFlip_i);
      output[13 - bitFlip_i] = !(bool)output[13 - bitFlip_i];
    }
    if(ready_to_send){
     
      counter = 0;
      sequence = 0;

      sending = 1;
      while(counter < 15){}
      sending = 0;
      digitalWrite(dataPin,0);
    }
    else{
      Serial.println("Set character first");
    }
    
  }
  else if(command == 'e'){
    Serial.println("Type your desired bit flip location (hex) (0-13)");
    while(!Serial.available()){}
    bitFlip_i = Serial.read();
    bitFlip_i -= 48;
    if(bitFlip_i > 9){
      bitFlip_i -= 39;
    }
    if(bitFlip_i >=0 && bitFlip_i < 14){
      Serial.print("Index to flip: ");
      Serial.println(bitFlip_i);
    }
    else{
      Serial.println("Invalid index, no error set");
      bitFlip_i = -1;
    }
    

  }
  else{
    Serial.println("Unknown character");
  }
  delay(500);
}