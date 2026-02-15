#include <DHT.h>

// PIN DEFINITIONS 
#define DHTPIN A0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Light sensor
const int PHOTON = A4;
const int LED = A3;
const float VCC = 4.98;
const float R_DIV = 3000;
const float DARK_MIN = 10000;

// Ultrasonic sensor
const int trigPin = A1;
const int echoPin = A2;
const int redLED = 1;
const int buz = A5;

// Display pins (7-segment)
int segA = 5, segB = 13, segC = 10, segD = 8, segE = 7;
int segF = 4, segG = 11, segPt = 9;
int d1 = 6, d2 = 3, d3 = 2, d4 = 12;

// Multiplex timing (1ms per digit -> ~4ms full cycle)
const long digitDelayUs = 1000;

// Variables
float temperature = 0;
long duration = 0;
int distance = 0;
bool objectDetected = false;
unsigned long previousMillis = 0;
bool buzzerState = LOW;
const int beepDuration = 10;


// Variables for logging

unsigned long previousLogMillis = 0;
const unsigned long logInterval = 60000;  // 1 minute
unsigned long minutesSinceStart = 0;

void setup() {
  Serial.begin(9600);

  // Startup message + delay for PuTTY
  Serial.println("Logging started...");
  delay(5000);

  dht.begin();

  // Light sensor pins
  pinMode(PHOTON, INPUT);
  pinMode(LED, OUTPUT);

  // Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buz, OUTPUT);

  // Display pins - ALL OUTPUT
  pinMode(13, OUTPUT); pinMode(12, OUTPUT); pinMode(11, OUTPUT);
  pinMode(10, OUTPUT); pinMode(9, OUTPUT); pinMode(8, OUTPUT);
  pinMode(7, OUTPUT); pinMode(6, OUTPUT); pinMode(5, OUTPUT);
  pinMode(4, OUTPUT); pinMode(3, OUTPUT); pinMode(2, OUTPUT);

  delay(2000);
  Serial.println("MULTI-SENSOR DASHBOARD READY!");
  Serial.println("Time | Light | Temp Display ");
}

void loop() {
  unsigned long currentMillis = millis();

  // LIGHT SENSOR
  int lightADC = analogRead(PHOTON);
  float lightV = lightADC * VCC / 1023.0;
  float lightR = R_DIV * (VCC / lightV - 1.0);

  if (lightR >= DARK_MIN) {
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }

  // DHT11 TEMPERATURE
  static unsigned long lastTempRead = 0;
  if (currentMillis - lastTempRead >= 500) {
    temperature = dht.readTemperature();
    if (isnan(temperature)) {
      temperature = 25.0;
    }
    lastTempRead = currentMillis;
  }

  // ULTRASONIC DISTANCE
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) {
    distance = 0;
  } else {
    distance = (duration * 0.034) / 2;
  }

  // Object detection
  if (distance > 0 && distance < 150) {
    digitalWrite(redLED, HIGH);
    objectDetected = true;
  } else {
    digitalWrite(redLED, LOW);
    objectDetected = false;
  }

  // BUZZER LOGIC
  handleBuzzer(currentMillis);

  // DISPLAY TEMPERATURE
  displayTemperature(temperature);

  // LOG EVERY 1 MINUTE (TEMP + LIGHT ONLY)
  if (currentMillis - previousLogMillis >= logInterval) {
    minutesSinceStart = currentMillis / 60000;
    
    Serial.print("MIN:");
    Serial.print(minutesSinceStart);
    Serial.print(" | Light:");
    Serial.print(lightR);
    Serial.print("Ohm | Temp:");
    Serial.print(temperature);
    Serial.println("C");
    
    previousLogMillis = currentMillis;
  }
}

// BUZZER CONTROL 
void handleBuzzer(unsigned long currentMillis) {
  int pauseInterval = 0;

  if (objectDetected) {
    if (distance <= 5)           pauseInterval = 1;
    else if (distance <= 10)     pauseInterval = 100;
    else if (distance <= 15)     pauseInterval = 300;
    else if (distance <= 30)     pauseInterval = 700;
    else if (distance <= 50)     pauseInterval = 1200;
  }

  if (pauseInterval == 1) {
    digitalWrite(buz, HIGH);
  } else if (pauseInterval == 0) {
    digitalWrite(buz, LOW);
    buzzerState = LOW;
  } else {
    if (buzzerState == HIGH) {
      if (currentMillis - previousMillis >= (unsigned long)beepDuration) {
        digitalWrite(buz, LOW);
        buzzerState = LOW;
        previousMillis = currentMillis;
      }
    } else {
      if (currentMillis - previousMillis >= (unsigned long)pauseInterval) {
        digitalWrite(buz, HIGH);
        buzzerState = HIGH;
        previousMillis = currentMillis;
      }
    }
  }
}

// 7-SEGMENT DISPLAY FUNCTIONS 
void displayTemperature(float temp) {
  int d1_val = tens(temp);
  int d2_val = ones(temp);
  int d3_val = tenths(temp);
  int d4_val = 10; // 'C'

  // Digit 1: tens
  selectDigit(1);
  sendDigit(d1_val);
  digitalWrite(segPt, LOW);
  delayMicroseconds(digitDelayUs);

  // Digit 2: ones + decimal point
  selectDigit(2);
  sendDigit(d2_val);
  digitalWrite(segPt, HIGH);
  delayMicroseconds(digitDelayUs);

  // Digit 3: tenths
  selectDigit(3);
  sendDigit(d3_val);
  digitalWrite(segPt, LOW);
  delayMicroseconds(digitDelayUs);

  // Digit 4: "C"
  selectDigit(4);
  sendDigit(d4_val);
  digitalWrite(segPt, LOW);
  delayMicroseconds(digitDelayUs);
}

void selectDigit(int d) {
  digitalWrite(d1, HIGH);
  digitalWrite(d2, HIGH);
  digitalWrite(d3, HIGH);
  digitalWrite(d4, HIGH);
  switch(d) {
    case 1: digitalWrite(d1, LOW); break;
    case 2: digitalWrite(d2, LOW); break;
    case 3: digitalWrite(d3, LOW); break;
    case 4: digitalWrite(d4, LOW); break;
  }
}

void sendDigit(int x) {
  switch(x) {
    case 0: zero(); break;
    case 1: one(); break;
    case 2: two(); break;
    case 3: three(); break;
    case 4: four(); break;
    case 5: five(); break;
    case 6: six(); break;
    case 7: seven(); break;
    case 8: eight(); break;
    case 9: nine(); break;
    default: cee(); break;
  }
}

// Digit extraction functions
int tens(float x)   { if(x<0) x=0; return (int)(x/10); }
int ones(float x)   { if(x<0) x=0; return (int)(x - tens(x)*10); }
int tenths(float x) { if(x<0) x=0; return (int)((x - tens(x)*10 - ones(x))*10); }

// 7-segment patterns 
void zero() {
  digitalWrite(segA, HIGH); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH); digitalWrite(segE, HIGH); digitalWrite(segF, HIGH);
  digitalWrite(segG, LOW); digitalWrite(segPt, LOW);
}
void one() {
  digitalWrite(segA, LOW); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, LOW); digitalWrite(segE, LOW); digitalWrite(segF, LOW);
  digitalWrite(segG, LOW); digitalWrite(segPt, LOW);
}
void two() {
  digitalWrite(segA, HIGH); digitalWrite(segB, HIGH); digitalWrite(segC, LOW);
  digitalWrite(segD, HIGH); digitalWrite(segE, HIGH); digitalWrite(segF, LOW);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void three() {
  digitalWrite(segA, HIGH); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH); digitalWrite(segE, LOW); digitalWrite(segF, LOW);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void four() {
  digitalWrite(segA, LOW); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, LOW); digitalWrite(segE, LOW); digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void five() {
  digitalWrite(segA, HIGH); digitalWrite(segB, LOW); digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH); digitalWrite(segE, LOW); digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void six() {
  digitalWrite(segA, HIGH); digitalWrite(segB, LOW); digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH); digitalWrite(segE, HIGH); digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void seven() {
  digitalWrite(segA, HIGH); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, LOW); digitalWrite(segE, LOW); digitalWrite(segF, LOW);
  digitalWrite(segG, LOW); digitalWrite(segPt, LOW);
}
void eight() {
  digitalWrite(segA, HIGH); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH); digitalWrite(segE, HIGH); digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void nine() {
  digitalWrite(segA, HIGH); digitalWrite(segB, HIGH); digitalWrite(segC, HIGH);
  digitalWrite(segD, HIGH); digitalWrite(segE, LOW); digitalWrite(segF, HIGH);
  digitalWrite(segG, HIGH); digitalWrite(segPt, LOW);
}
void cee() {
  digitalWrite(segA, HIGH); digitalWrite(segB, LOW); digitalWrite(segC, LOW);
  digitalWrite(segD, HIGH); digitalWrite(segE, HIGH); digitalWrite(segF, HIGH);
  digitalWrite(segG, LOW); digitalWrite(segPt, LOW);
}
