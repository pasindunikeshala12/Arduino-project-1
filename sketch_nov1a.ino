#include <Servo.h>
#include <SoftwareSerial.h>

Servo servoMotor;

// Pin define
const int flamePins[] = {A1, A2, A3, A4, A5};  
const int smokePin = A0;
const int relayPin = 2;
const int servoPin = 9;
const int ledPin = 7;
const int buzzerpin =8;
// Thresholds
const int flameThreshold = 600;
const int smokeThreshold = 250;

// Servo angles
const int servoPositions[] = {160, 130, 100, 70, 40};

void flashLED(int times, int delayTime);
void scanArea();
void extinguishFire(int sensorIndex);

SoftwareSerial sim800L(10, 11); 


void setup() {
  Serial.begin(9600);
  Serial.println("Fire Fighting System Initialized");
   sim800L.begin(9600);
   delay(100);
   sim800L.println("AT");
   delay(1000);
//test the sim 800l
  while (sim800L.available()) {
    Serial.write(sim800L.read());
  }

  sim800L.println("AT+CSQ");  // Signal strength
  delay(2000);
  while (sim800L.available()) {
    Serial.write(sim800L.read());
  }

  sim800L.println("AT+CREG?"); // Network registration
  delay(2000);
  while (sim800L.available()) {
    Serial.write(sim800L.read());
  }

  sim800L.println("AT+CMGF=1"); // Set text mode
  delay(2000);
  while (sim800L.available()) {
    Serial.write(sim800L.read());
  }

  Serial.println("Ready...");


  // Pin setup
  for (int i = 0; i < 5; i++) 
  pinMode(flamePins[i], INPUT);
  pinMode(smokePin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerpin,OUTPUT);
  servoMotor.attach(servoPin);
  servoMotor.write(90); 
}



void loop() {
  int flameValues[5];
  int smokeValue = analogRead(smokePin);
  bool smokeDetected = false;
   bool flameDetected = false;
  int flameIndex = -1;
if ( smokeValue > smokeThreshold) {
      smokeDetected = true;
  if(smokeDetected){
    for (int i = 0; i < 5; i++) {
    flameValues[i] = analogRead(flamePins[i]);
     
      if (flameValues[i] > flameThreshold ){
      flameDetected = true;
      flameIndex = i;
      }
    }
  }  
}else if(~smokeDetected){
  for (int i = 0; i < 5; i++) {
    flameValues[i] = analogRead(flamePins[i]);
     
      if (flameValues[i] > flameThreshold ){
      flameDetected = true;
      flameIndex = i;
      }
    }
}
  
  Serial.print("Flame: ");
  for (int i = 0; i < 5; i++) {
    Serial.print(flameValues[i]);
    Serial.print(" ");
  }
  Serial.print("| Smoke: ");
  Serial.println(smokeValue);

  if (flameDetected && !smokeDetected) {
    Serial.print(" Fire detected by flame sensor ");
    Serial.println(flameIndex + 1);
   
    flashLED(30, 100);      
    scanArea();             
    extinguishFire(flameIndex);
    sendSMS("ALERT: FLAME detected! Fire-fighting system activated.");
    makeCall("+0760580066"); 
    sendGPRSData();
    }
    else if(flameDetected && smokeDetected){
    Serial.print(" Fire detected by flame&smoke sensor ");
    Serial.println(flameIndex + 1);

    flashLED(30, 100);      
    scanArea();             
    extinguishFire(flameIndex);
    sendSMS("ALERT: FIRE detected! Fire-fighting system activated.");
    makeCall("+0760580066"); 
     sendGPRSData();
    }
    else if(!flameDetected && smokeDetected){
    Serial.print(" Fire detected by smoke sensor ");
    Serial.println(flameIndex + 1);

    flashLED(30, 100);      
    scanArea();             
    sendSMS("ALERT: SMOKE detected! Fire-fighting system activated.");
    makeCall("+0760580066"); 
    sendGPRSData();
    }
  else {
    digitalWrite(relayPin, LOW);
    digitalWrite(ledPin, LOW);
  }

  delay(2000);
}





//  led and buzzer alarm
void flashLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(delayTime);
    digitalWrite(buzzerpin, HIGH);
    delay(delayTime);
    digitalWrite(ledPin, LOW);
    delay(delayTime);
    digitalWrite(buzzerpin, LOW);
    delay(delayTime);
  }
}

// servo scan area
void scanArea() {
  for (int pos = 50; pos <= 150; pos += 2) {
    servoMotor.write(pos);
    delay(100);
  }
}

// water moter
void extinguishFire(int sensorIndex) {
  int targetPos = servoPositions[sensorIndex];
  servoMotor.write(targetPos);

  digitalWrite(relayPin, HIGH);
  delay(8000);  
  digitalWrite(relayPin, LOW);

  servoMotor.write(90); 
}

void sendSMS(String message) {
  sim800L.println("AT+CMGF=1"); // Set SMS text mode
  delay(500);
  sim800L.println("AT+CMGS=\"+94760580066\""); 
  delay(500);
  sim800L.print(message);
  delay(500);
  sim800L.write(26); // Ctrl+Z to send SMS
  delay(2000);
  Serial.println("SMS sent!");
}

void makeCall(String number) {
  sim800L.print("ATD");
  sim800L.print(number);
  sim800L.println(";");
  delay(5000); 
  sim800L.println("ATH"); 
  delay(500);
  Serial.println("Call made and ended.");
}
// Send GPRS HTTP request
void sendGPRSData() {
  Serial.println(" Sending GPRS data...");
  sim800L.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(1000);
  sim800L.println("AT+SAPBR=3,1,\"APN\",\"yourAPN\"");  // <-- Change to your SIM's APN
  delay(2000);
  sim800L.println("AT+SAPBR=1,1");
  delay(3000);
  sim800L.println("AT+HTTPINIT");
  delay(1000);
  sim800L.println("AT+HTTPPARA=\"CID\",1");
  delay(1000);
  sim800L.println("AT+HTTPPARA=\"URL\",\"http://yourserver.com/upload?fire=1\"");
  delay(1000);
  sim800L.println("AT+HTTPACTION=0");
  delay(6000);
  sim800L.println("AT+HTTPTERM");
  delay(1000);
  sim800L.println("AT+SAPBR=0,1");
  delay(1000);
  Serial.println("GPRS data sent.");
}
