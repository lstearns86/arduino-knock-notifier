//includes
#include <SPI.h> // needed in Arduino 0019 or later
#include <WiFi.h>
#include <Twitter.h>
#include <String.h>

// pin layout
const int knockSensor = A0;         // the piezo is connected to analog pin 0
const int led = 13;                 // built in LED at pin 13
const int blue = 6;                  // RGB LED, must be PWM pin
const int green = 5;                // RGB LED, must be PWM pin
const int red = 3;                 // RGB LED, must be PWM pin
const int door = 8;

// constants
const int knockHighThreshold = 20; // the analog reading must go above this value to trigger a knock
const int knockLowThreshold = 3;    // no additional knocks will be triggered until the readings fall below this value
const int knockTimeout = 1000;      // stop listening for additional knocks after 1 second without a knock
const int maxKnocks = 20;           // maximum number of knocks in pattern

const int secretKnockIndividualThreshold = 25; // the maximum amount that an individual knock timing (normalized) can be off before rejection
const int secretKnockAverageThreshold = 15;    // the maximum average knock variation from secret knock before rejection
const int ledFadeDelay = 2000;  // the duration to display the rbg led color before fading out

char server[]="www.timeapi.org";
const char lf=10;
const char validKnockMsg[] = "Knock knock...someone's out there";
const char secretKnockMsg[] = "Secret knock detected";

// variables
char ssid[] = "CMSC838F";
char pass[] = "Hackerspace";
// Your Token to Tweet (get it from http://arduino-tweet.appspot.com/)
Twitter twitter("917007349-eDEI3aBmTcHyFmjpTqIBG7EA0ygx55bGgB7ps7ew");
WiFiClient client;
char inString[50]; // string for incoming serial data
int stringPos = 0; // string index counter
boolean startRead = false; // is reading?

int sensorReading;
int knockValue;
unsigned long knockTime;
unsigned long lastKnock;
boolean knocking;
boolean knocked;
unsigned long ledTime;

int knockIndex = 0;
unsigned long knockTimings[maxKnocks];

int r = 0, g = 0, b = 0;

int secretKnock[maxKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // "Shave and a Hair Cut, two bits."

boolean setupDone = false;

void setup()
{
  if(setupDone)
    return;
  // initialize serial Port
  Serial.begin(9600);
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(door, OUTPUT);
  digitalWrite(red, HIGH);
  digitalWrite(green, HIGH);
  digitalWrite(blue, HIGH);
  digitalWrite(door, LOW);
  
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  knocked = false;
  knocking = false;
  lastKnock = millis();
  fadeToColor(255, 0, 255);
  
  //connect to wifi
  Serial.println("Connecting to wifi...");
  WiFi.begin(ssid, pass);
  Serial.println("wifi connected...");
  //Serial.println("connecting to server...");
  //client.connect(server,80);
  //Serial.println("client connected to server");
  
  fadeToColor(0, 0, 0);
  ledTime = millis();
  
  setupDone = true;
}

void processKnocks()
{
  knocking = false;
  int numKnocks = knockIndex + 1;
  
  // reset values
  knockIndex = 0;
  lastKnock = millis();
  knockValue = 0;
  
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
      Serial.println(totalDiff / secretKnockLength);
      if(totalDiff / secretKnockLength > secretKnockAverageThreshold) stillValid = false;
    }
    
    if(stillValid)
    {
      Serial.println("Secret Knock Detected");
      digitalWrite(door, HIGH);
      fadeToColor(0,255,0);
      sendTweet(secretKnockMsg);
      delay(2000);
      fadeToColor(0,0,0);
      digitalWrite(door, LOW);
    }
    else
    {
      Serial.println("Valid Knock Pattern");
      fadeToColor(255,0,0);
      sendTweet(validKnockMsg);
      delay(2000);
      fadeToColor(0,0,0);
      
    }
  }
  else
  {
    Serial.print("Not Enough Knocks (");
    Serial.print(knocking);
    Serial.print(", ");
    Serial.print(millis() - lastKnock);
    Serial.println(")");
    fadeToColor(0,0,255);
  }
}

void fadeToColor(int r2, int g2, int b2)
{
  int ri = r, gi = g, bi = b;
  float p;
  /*for(int i = 0; i < 50; i++)
  {
    p = (float)i / 50.0f;
    ri = r + (int)((r2 - r) * p);
    gi = g + (int)((g2 - g) * p);
    bi = b + (int)((b2 - b) * p);
    analogWrite(red, 255 - ri);
    analogWrite(green, 255 - gi);
    analogWrite(blue, 255 - bi);
    
    delay(10);
  }*/
  analogWrite(red, 255 - r2);
  analogWrite(green, 255 - g2);
  analogWrite(blue, 255 - b2);
  r = r2;
  g = g2;
  b = b2;
  ledTime = millis();
}

void loop()
{
  sensorReading = analogRead(knockSensor);
  if((r > 0 || g > 0 || b > 0) && millis() - ledTime > ledFadeDelay) fadeToColor(0, 0, 0);
  
  if(knocked)
  {
    if(sensorReading < knockLowThreshold)
    {
      knocked = false;
      knockValue = 0;
      //digitalWrite(led, LOW);
      fadeToColor(0, 0, 0);
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
        //digitalWrite(led, HIGH);
        fadeToColor(255, 255, 0);
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
        delay(20); // need a small delay or else it triggers multiple times
      }
    }
    else
    {
      if(knocking && millis() - lastKnock >= knockTimeout) processKnocks();
    }
  }
}

String getTimeFromInternet(){
  Serial.println("getting time...");
  if(client.connect(server,80)){
    Serial.println("connected to timeapi...");
    client.println("GET /est/now HTTP/1.1");
    client.println("Host:www.timeapi.org");
    client.println("Connection: close");
    client.println();
    return readPage();
  }
  return "Unable to connect to timeserver...";
}

String readPage(){
  
  //read the page, and capture & return everything between '<' and '>'
  boolean readNow = false;
  boolean firstLF = false;
  stringPos = 0;
  memset( &inString, 0, 50 ); //clear inString memory
  Serial.println("Entered readPage...");
  //return inString;
  while(true){
    //Serial.println("going in while loop...");
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
      if(readNow){
        Serial.println("\nreading");
        if(c==lf){
          firstLF=false;
          readNow=false;
          client.stop();
          client.flush();
          Serial.println("Returning value...");
          return inString;
        }
        inString[stringPos] = c;
        stringPos ++;
        continue;
      }
      
      if(firstLF && c==lf){
        readNow = true;
      }
      firstLF=false;
      if(c==lf){
        firstLF=true;
      }
    }//end of if clinet.available
    else
     return "error";
  }//end of while
}//end of readPage

void sendTweet(String msg){
  Serial.println("going to send tweet");
  Serial.println(msg);
  //String time = getTimeFromInternet();
  unsigned long timeMS = millis();
  String time = String(timeMS);
  //String time="1";
  //Serial.println(time);
  
  msg=msg+" ["+time+"]";
  Serial.println(msg);
  int length = msg.length()+1;
  char msgArray[length];
  msg.toCharArray(msgArray,length);
  
  Serial.print("posting to twitter on ");
  Serial.print(time);
  Serial.println(" ...");
  
  Serial.println(msgArray);
  
  if (twitter.post(msgArray)) {
    // Specify &Serial to output received response to Serial.
    // If no output is required, you can just omit the argument, e.g.
    // int status = twitter.wait();
    int status = twitter.wait(&Serial);
    if (status == 200) {
      Serial.println("OK.");
    } else {
      Serial.print("failed : code ");
      Serial.println(status);
    }
  } else {
    Serial.println("connection failed.");
  }
  //free(msgArray);
}

