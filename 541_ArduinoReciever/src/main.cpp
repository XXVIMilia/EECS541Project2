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
volatile int incomingPacket[14];
volatile int readCount = 0;


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
  for(int i = 5; i < 20; i++){
    Serial.println(sampledDelays[i]);
    sum += sampledDelays[i];
  }
  
  detectedBitRate = sum / 15;

}


void readData(){
    
    if(trigger){
      //if(readCount > 0){//Ignore first bit in stream
        
        digitalWrite(7, LOW);
        if(hit){
          incomingPacket[readCount] = 1;
        }
        else{
          incomingPacket[readCount] = 0;
        }
        
      //}
      

      hit = 0;
      if(readCount == 14){
        readCount = 0;
        trigger = 0;
      }
      digitalWrite(7, HIGH);

      readCount++;
    }
    
    
    
}

//Simple cleanup function
void resetArrays(){
  for(int i = 0; i < 20; i++){
    sampledDelays[i] = 0;
  }
  for(int i = 0; i < 14; i++){
    incomingPacket[i] = 0;
  }
}

volatile int hitsCounted;
volatile bool awaitTrigger = 0;
//Use for getting exact start of signal tranmsission. Used for detecting falling edge of bits
void awaitTriggerSignal(){
  if(awaitTrigger){
    digitalWrite(3,LOW);
    if(!trigger){
      trigger = 1;
      // Timer1.restart();
      hit = 0;
      hitsCounted = 0;
      
    }
    else{
      hit = 1;
      hitsCounted++;
      
    }  

    digitalWrite(3, HIGH);

    }
  
  
}



//Verifies the calculated bit delay is usable
bool verifySignal(){
  trigger = 0;
  readCount = 0;
  hit = 0;

   
  awaitTrigger = 1;
  while(!trigger){}


  
  while(trigger){}
  awaitTrigger = 0;

  digitalWrite(3, HIGH);
  digitalWrite(7, HIGH);


  for(int i = 1; i < 14; i++){
    if(!incomingPacket[i]){ 
      return(false);
    }
  }
  
  return(true);
}

//Get raw 12 bit packet 
int readMessage(){
  trigger = 0;
  readCount = 0;
  hit = 0;
  hitsCounted = 0;


  awaitTrigger = 1;
  while(!trigger){}


  
  while(trigger){}
  awaitTrigger = 0;

  Serial.print("Hits counted: ");
  Serial.println(hitsCounted);
  return(hitsCounted);
  
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  pinMode(7,OUTPUT);
  digitalWrite(3, HIGH);
  digitalWrite(7, HIGH);
  Timer1.initialize();
  
}


bool ham_calc(int position, int code_length){
  int count = 0, i, j;
  i = position - 1;

  // Traverse to store Hamming Code
  while (i < code_length) {
    for (j = i; j < i + position; j++) {
      // If current bit is 1
      if (incomingPacket[j] == 1)
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


void decodeHam(){
  uint8_t error, decodedMessage;
  error = decodedMessage = 0;
  int ham[12];
  int hamCorrected[12];

  int ones = 0;
  ones = 0;
    for(int i = 0; i < 13 ; i++){
      if(incomingPacket[i]){
        ones++;
      }
    }
  bool parity;

  if(ones % 2){
      parity = 0;
    }
    else{
      parity = 1;
    }

 

  //Load data from packet into ham holder
  for (int i = 0; i < 12; i++) {
    ham[i] = incomingPacket[11-i];
  }

  // Traverse and update the
  // hamming code
  for (int i = 0; i < 4; i++) {

    // Find current position
    int position = (int)(1 << i);

    // Find value at current position
    int value = ham_calc(position, 12);

    // Update the code
    ham[position - 1] = value;
    hamCorrected[position - 1] = value;

    error |= (uint8_t)(ham[position -1] << i);
  
  }

  
  if((error > 0) && !parity){
    Serial.println("Two bit error detected");
    Serial.print("Error: ");
    Serial.println(error);
    Serial.print("Parity: ");
    Serial.println(parity);

  }
  else if((error > 0) && !parity){
    Serial.print("Error at: ");
    Serial.println(error);
    hamCorrected[error] = !hamCorrected[error];
  }
  else{
    Serial.println("No error detected");
  }


  int j,k;
  j = k = 0;
  int decodedPacket[8];
  int fixedDecodedPacket[8];
  //Extract incoming data from ham
  for(int i = 0; i < 12; i++){
    if(i == (int)(1 << k) - 1){
      k++;
    }
    else{
      decodedPacket[j] = ham[i]; 
      fixedDecodedPacket[j] = hamCorrected[i];
      j++;
    }
  }

  uint8_t data, correctedData;
  for(int i = 0; i < 8; i++){
    data |= (decodedPacket[i] << i);
    correctedData |= (fixedDecodedPacket[i] << i);
  }


  if((error > 0) && !parity){
    Serial.println("Unable to decode");
  }else if((error > 0) && parity){
    Serial.print("Uncorrected data: ");
    Serial.println((char)data);
    Serial.print("Corrected data: ");
    Serial.println((char)correctedData);
  }
  else{
    Serial.print("Data: ");
    Serial.println((char)data);
  }

}


bool timerActive = 0;
void loop() {
  resetArrays();
  Serial.println("Send a command: 'c' = calibrate, 'r' = read");
  while(!Serial.available()){}
  command = Serial.read(); 

  if(command == 'c'){
    Serial.println("Attempting to detect signal");
    calibrateDelay();
    attachInterrupt(digitalPinToInterrupt(2),awaitTriggerSignal,FALLING);
    Serial.print("Detected Bit Rate: ");
    Serial.println(detectedBitRate);
    Timer1.attachInterrupt(readData,detectedBitRate);
    Timer1.start();
    timerActive = 1;


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
    for(int i = 1; i < 14; i++){
      Serial.print(incomingPacket[i]);
    }
    Serial.print('\n');
    //do{}while(readMessage());//Trying to fix reading bug. Ugly solution
  }
  else if(command == 'r'){
    if(recievingMode){
      resetArrays();
      Serial.println("Entering reading mode:");
      readMessage();
      //do{}while(!readMessage());//Trying to fix reading bug. Ugly solution
      Serial.print("Recieved Packet: ");
      for(int i = 1; i < 14; i++){
        Serial.print(incomingPacket[i]);
      }
      Serial.println("");
      decodeHam();
     

    }
    else{
      Serial.println("Signal not calibrated yet");
    }

  }






  
}