#include <Arduino.h>


//Signal Detection Variables
volatile unsigned long riseTime = 0;
volatile unsigned long prevTime = 0;
volatile unsigned long sampledDelays[10];
volatile bool trigger = 0;
volatile int count = 0;
volatile bool checkDelay = 0;
unsigned long detectedBitRate = 0;
volatile bool calibrating = 0;

//Timing Verification Variables
unsigned long prevTimeStamp;
unsigned long driftCorrection;


//Main Variables
bool detectedSignal = 0;
bool validatedSignal = 0;
bool recievingMode = 0;

//Message Storage
int incomingPacket[12];
char decodedMessage;


//Rise Time Detection Callback Function
void rise(){
  if(calibrating){
    prevTime = riseTime;
    riseTime = micros();
    sampledDelays[count % 10] = riseTime - prevTime;
    count++;
  }
  
}

//Function that takes sampled bit delays and averages them
void calibrateDelay(){
  count = 0;
  calibrating = 1;
  while(count < 10){
    //Busy Wait
  }
  calibrating = 0;

  unsigned long sum = 0;
  for(int i = 1; i < 10; i++){
    sum += sampledDelays[i];
  }
  
  detectedBitRate = sum / 9;
  detectedSignal = 1;

}

//Initializes pin 2 for interrupts
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2,INPUT_PULLUP);
  prevTime = micros();
  
}

void hammingCodeExtract(){
  //Make function that deconstructs the hamming code into data and counts bit error
}

//Simple cleanup function
void resetSamples(){
  for(int i = 0; i < 10; i++){
    sampledDelays[i] = 0;
  }
}

//Use for getting exact start of signal tranmsission
void awaitTriggerSignal(){
  trigger = 1;
  detachInterrupt(digitalPinToInterrupt(2));
}

//Verifies the calculated bit delay is usable
bool verifySignal(){
  trigger = 0;
  bool bitRead;
  
  attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,RISING);
  while(!trigger){}
  

  delayMicroseconds(detectedBitRate);
  for(int i = 0; i < 10; i++){
    prevTimeStamp = micros();
    bitRead = digitalRead(2);
    Serial.print(bitRead);
    if(!bitRead){
      
      return(false);
    }
    delayMicroseconds(detectedBitRate - (micros() - prevTimeStamp));
  }
  
  return(true);
}

//Get raw 12 bit packet 
void readMessage(){
  trigger = 0;
  attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,RISING);
  while(!trigger){}
  
  delayMicroseconds(detectedBitRate);
  
  for(uint16_t i = 0; i < 9; i++){
    prevTimeStamp = micros();
    incomingPacket[i] = digitalRead(2);
    delayMicroseconds(detectedBitRate - (micros() - prevTimeStamp));
  }
}




void loop() {
  if(!detectedSignal){
    attachInterrupt(digitalPinToInterrupt(2),rise,RISING);
    resetSamples();
    calibrateDelay();
    detachInterrupt(digitalPinToInterrupt(2));
    Serial.print("Detected Bit Rate: ");
    Serial.println(detectedBitRate);
    delay(500);
  }

  if(validatedSignal == 0){
    validatedSignal = verifySignal();
    if(validatedSignal){
      Serial.println("Signal Verified");
      recievingMode = 1;
    }
    else{
      Serial.println("Verify Failed. Trying again"); 
      detectedSignal = 0;
    }
    delay(500);
  }

  if(recievingMode){
    Serial.print("Entering reading mode in: \n3...");
    delay(1000);
    Serial.print("2...");
    delay(1000);
    Serial.println("1...");
    delay(1000);
    readMessage();
    for(int i = 0; i < 9; i++){
      Serial.print(incomingPacket[i]);
    }
    Serial.println("");
    validatedSignal = 0;
  }

  //delayMicroseconds(detectedBitRate);
  
}