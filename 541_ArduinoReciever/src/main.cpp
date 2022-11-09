#include <Arduino.h>
#include <TimerOne.h>


//Signal Detection Variables
volatile unsigned long fallTime = 0;
volatile unsigned long prevTime = 0;
volatile unsigned long sampledDelays[20];
volatile bool trigger = 0;
volatile int count = 0;
volatile bool checkDelay = 0;
unsigned long detectedBitRate = 0;
volatile bool calibrating = 0;
volatile bool hit;

//Timing Verification Variables
unsigned long prevTimeStamp;
unsigned long driftCorrection;


//Main Variables
bool detectedSignal = 0;
bool validatedSignal = 0;
bool recievingMode = 0;

//Message reading
volatile int incomingPacket[13];
volatile bool reading = 0;
volatile int readCount = 0;
char decodedMessage;


//User input
char command;

//fall Time Detection Callback Function
void fall(){
  if(calibrating){
    prevTime = fallTime;
    fallTime = micros();
    sampledDelays[count % 20] = fallTime - prevTime;
    count++;
  }
  
}

//Function that takes sampled bit delays and averages them
void calibrateDelay(){
  count = 0;
  calibrating = 1;
  attachInterrupt(digitalPinToInterrupt(2),fall,FALLING);
  while(count < 20){
    //Busy Wait
  }
  calibrating = 0;
  detachInterrupt(digitalPinToInterrupt(2));

  unsigned long sum = 0;
  for(int i = 2; i < 20; i++){
    Serial.println(sampledDelays[i]);
    sum += sampledDelays[i];
  }
  
  detectedBitRate = sum / 18;

}


void readData(){
    digitalWrite(3, LOW);
    if(reading){
      if(hit){
      incomingPacket[readCount % 13] = 1;
    }
    else{
      incomingPacket[readCount % 13] = 0;
    }
    
    readCount++;
    // if(readCount == 12){
    //     Timer1.detachInterrupt();
    // }
    hit = 0;
    }
    
    
}

//Initializes pin 2 for interrupts



//Simple cleanup function
void resetArrays(){
  for(int i = 0; i < 20; i++){
    sampledDelays[i] = 0;
  }
  for(int i = 0; i < 13; i++){
    incomingPacket[i] = 0;
  }
}


//Use for getting exact start of signal tranmsission. Used for detecting falling edge of bits
void awaitTriggerSignal(){
  if(!trigger){
    trigger = 1;
    Timer1.start();
    hit = 0;
  }
  else{
     hit = 1;
  }  
}



//Verifies the calculated bit delay is usable
bool verifySignal(){
  trigger = 0;
  readCount = 0;

  //Timer1.attachInterrupt(readData,detectedBitRate);  
  attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,FALLING);
  while(!trigger){}
  reading = 1;

  
  while(readCount < 13){
    //delayMicroseconds(100);
    digitalWrite(3, HIGH);
  }
  reading = 0;
  detachInterrupt(digitalPinToInterrupt(2));

  digitalWrite(3, HIGH);



  for(int i = 0; i < 13; i++){
    if(!incomingPacket[i]){ 
      return(false);
    }
  }
  
  return(true);
}

//Get raw 12 bit packet 
void readMessage(){
  trigger = 0;
  readCount = 0;

  //Timer1.attachInterrupt(readData,detectedBitRate); 
  attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,FALLING);
  while(!trigger){}
  reading = 1;
  
  while(readCount < 13){
    digitalWrite(3, HIGH);
  }
  //Timer1.detachInterrupt();
  reading = 0;
  detachInterrupt(digitalPinToInterrupt(2));
  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  digitalWrite(3, HIGH);
  Timer1.initialize();
  
}



void loop() {
  Serial.println("Send a command: 'c' = calibrate, 'r' = read");
  while(!Serial.available()){}
  command = Serial.read(); 

  if(command == 'c'){
    Serial.println("Attempting to detect signal");
    resetArrays();
    calibrateDelay();
    Serial.print("Detected Bit Rate: ");
    Serial.println(detectedBitRate);
    Timer1.attachInterrupt(readData,detectedBitRate); 


    //Validating
    Serial.println("Attempting to validate signal");
    validatedSignal = verifySignal();
    if(validatedSignal){
      Serial.println("Signal Verified");
      recievingMode = 1;
    }
    else{
      Serial.println("Verify Failed. Trying again"); 
      detectedSignal = 0;
      recievingMode = 0;
    }

    Serial.println("Packet detected");
    for(int i = 0; i < 13; i++){
      Serial.print(incomingPacket[i]);
    }
    Serial.print('\n');
  }
  else if(command == 'r'){
      recievingMode = 1;//DEBUG ONLY!!!!!!!!!!!!!!
      if(recievingMode){
      resetArrays();
      Serial.print("Entering reading mode in: \n3...");
      delay(1000);
      Serial.print("2...");
      delay(1000);
      Serial.println("1...");
      delay(1000);
      readMessage();
      for(int i = 0; i < 13; i++){
        Serial.print(incomingPacket[i]);
      }
      Serial.println("");
    }
    else{
      Serial.println("Signal not calibrated yet");
    }

  }






  
}