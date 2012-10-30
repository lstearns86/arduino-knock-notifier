// constants
const int knockSensor = A0;         // the piezo is connected to analog pin 0
const int led = 13;                 // built in LED at pin 13
const int knockHighThreshold = 100; // the analog reading must go above this value to trigger a knock
const int knockLowThreshold = 3;    // no additional knocks will be triggered until the readings fall below this value
const int knockTimeout = 1000;      // stop listening for additional knocks after 1 second without a knock
const int maxKnocks = 20;           // maximum number of knocks in pattern

// variables
int sensorReading;
int knockValue;
int knockTime;
int lastKnock;
boolean knocking;
boolean knocked;

int knockIndex = 0;
int knockTimings[maxKnocks];

void setup()
{
  // initialize serial Port
  Serial.begin(9600);
  
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
    Serial.println("Valid Knock Pattern");
  }
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

