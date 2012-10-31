// pin layout
const int knockSensor = A0;         // the piezo is connected to analog pin 0
const int led = 13;                 // built in LED at pin 13
const int red = 6;                  // RGB LED, must be PWM pin
const int green = 5;                // RGB LED, must be PWM pin
const int blue = 3;                 // RGB LED, must be PWM pin

// constants
const int knockHighThreshold = 100; // the analog reading must go above this value to trigger a knock
const int knockLowThreshold = 3;    // no additional knocks will be triggered until the readings fall below this value
const int knockTimeout = 1000;      // stop listening for additional knocks after 1 second without a knock
const int maxKnocks = 20;           // maximum number of knocks in pattern

const int secretKnockIndividualThreshold = 25; // the maximum amount that an individual knock timing (normalized) can be off before rejection
const int secretKnockAverageThreshold = 15;    // the maximum average knock variation from secret knock before rejection

// variables
int sensorReading;
int knockValue;
int knockTime;
int lastKnock;
boolean knocking;
boolean knocked;

int knockIndex = 0;
int knockTimings[maxKnocks];

int r = 0, g = 0, b = 0;

int secretKnock[maxKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // "Shave and a Hair Cut, two bits."

void setup()
{
  // initialize serial Port
  Serial.begin(9600);
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  knocked = false;
  knocking = false;
  lastKnock = millis();
}

void processKnocks()
{
  knocking = false;
  int numKnocks = knockIndex + 1;
  knockIndex = 0;
  
  // TODO: process knock timings to determine pattern
  if(numKnocks >= 2)
  {
    // get the length of the secret knock
    int secretKnockLength = 0;
    for(int i = 0; i < maxKnocks; i++) if(secretKnock[i] > 0) secretKnockLength++; else break;
    
    // first, check whether we have the right number of knocks
    boolean stillValid = true;
    if(secretKnockLength != numKnocks - 1) stillValid = false;
    
    if(stillValid)
    {
      // get the maximum separation between knocks, for normalization
      int maxInterval = 0;
      for(int i = 0; i < numKnocks; i++) if(knockTimings[i] > maxInterval) maxInterval = knockTimings[i];
      
      int totalDiff = 0;
      for(int i = 0; i < numKnocks; i++)
      {
        knockTimings[i] = map(knockTimings[i], 0, maxInterval, 0, 100);
        int diff = abs(knockTimings[i] - secretKnock[i]);
        if(diff > secretKnockIndividualThreshold) stillValid = false;
        totalDiff += diff;
      }
      if(totalDiff / secretKnockLength > secretKnockAverageThreshold) stillValid = false;
    }
    
    if(stillValid)
    {
      Serial.println("Secret Knock Detected");
      fadeToColor(0,255,0);
    }
    else
    {
      Serial.println("Valid Knock Pattern");
      fadeToColor(255,0,0);
    }
  }
  else
  {
    Serial.println("Not Enough Knocks");
    fadeToColor(0,0,255);
  }
}

void fadeToColor(int r2, int g2, int b2)
{
  int ri = r, gi = g, bi = b;
  float p;
  for(int i = 0; i < 50; i++)
  {
    p = (float)i / 50.0f;
    ri = r + (int)((r2 - r) * p);
    gi = g + (int)((g2 - g) * p);
    bi = b + (int)((b2 - b) * p);
    analogWrite(red, 255 - ri);
    analogWrite(green, 255 - gi);
    analogWrite(blue, 255 - bi);
    
    delay(10);
  }
  r = r2;
  g = g2;
  b = b2;  
}

void loop()
{
  sensorReading = analogRead(knockSensor);
  
  if(knocked)
  {
    if(sensorReading < knockLowThreshold)
    {
      knocked = false;
      digitalWrite(led, LOW);
    }
  }
  else
  {
    if(sensorReading > knockHighThreshold)
    {
      if(sensorReading > knockValue)
      {
        knockValue = sensorReading;
        knockTime = millis();
      }
      else
      {
        knocked = true;
        digitalWrite(led, HIGH);
        Serial.print("knock (");
        Serial.print(knockValue);
        Serial.println(")");
        
        if(knocking)
        {
          int elapsed = knockTime - lastKnock;
          knockTimings[knockIndex] = elapsed;
          knockIndex++;
          if(knockIndex >= maxKnocks) processKnocks();
        }
        else
        {
          knocking = true;
        }
        
        lastKnock = knockTime;
        knockValue = 0;
        delay(10); // need a small delay or else it triggers multiple times
      }
    }
    else
    {
      if(knocking && millis() - lastKnock >= knockTimeout) processKnocks();
    }
  }
}

