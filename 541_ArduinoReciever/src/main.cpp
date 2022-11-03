#include <Arduino.h>
#include <TimerOne.h>


//Signal Detection Variables
volatile unsigned long riseTime = 0;
volatile unsigned long prevTime = 0;
volatile unsigned long sampledDelays[20];
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

//Message reading
volatile int incomingPacket[12];
volatile bool reading = 0;
volatile int readCount = 0;
char decodedMessage;


//User input
char command;

//Rise Time Detection Callback Function
void rise(){
  if(calibrating){
    prevTime = riseTime;
    riseTime = micros();
    sampledDelays[count % 20] = riseTime - prevTime;
    count++;
  }
  
}

//Function that takes sampled bit delays and averages them
void calibrateDelay(){
  count = 0;
  calibrating = 1;
  attachInterrupt(digitalPinToInterrupt(2),rise,FALLING);
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
    incomingPacket[readCount % 12] = !digitalRead(2);
    readCount++;
    if(readCount == 12){
        Timer1.detachInterrupt();
    }
    
}

//Initializes pin 2 for interrupts



//Simple cleanup function
void resetSamples(){
  for(int i = 0; i < 20; i++){
    sampledDelays[i] = 0;
  }
}

//Use for getting exact start of signal tranmsission
void awaitTriggerSignal(){
  trigger = 1;
  Timer1.start();
  detachInterrupt(digitalPinToInterrupt(2));
}



//Verifies the calculated bit delay is usable
bool verifySignal(){
  trigger = 0;
  readCount = 0;

  Timer1.attachInterrupt(readData,detectedBitRate);  
  attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,FALLING);
  while(!trigger){}

  
  while(readCount < 12){
    //delayMicroseconds(100);
    digitalWrite(3, HIGH);
  }

  digitalWrite(3, HIGH);



  for(int i = 0; i < 12; i++){
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


  attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,FALLING);
  while(!trigger){}
  

  Timer1.attachInterrupt(readData,detectedBitRate);
  while(readCount < 12){}
  Timer1.detachInterrupt();
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
    resetSamples();
    calibrateDelay();
    Serial.print("Detected Bit Rate: ");
    Serial.println(detectedBitRate);

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
    for(int i = 0; i < 12; i++){
      Serial.print(incomingPacket[i]);
    }
    Serial.print('\n');
  }
  else if(command == 'r'){
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
    else{
      Serial.println("Signal not calibrated yet");
    }

  }






  
}