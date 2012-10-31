/*
  Wifi Twitter Client with Strings
 
 This sketch connects to Twitter using using an Arduino WiFi shield. 
 It parses the XML returned, and looks for <text>this is a tweet</text>
 
 This example is written for a network using WPA encryption. For 
 WEP or WPA, change the Wifi.begin() call accordingly.
 
 This example uses the String library, which is part of the Arduino core from
 version 0019.  
 
 Circuit:
 * WiFi shield attached to pins 10, 11, 12, 13
 
 created 23 apr 2012
 modified 31 May 2012
 by Tom Igoe
 
 This code is in the public domain.
 
 */
#include <SPI.h>
#include <WiFi.h>
#include <Ethernet.h>
#include <aJSON.h>
#include <MemoryFree.h>

char ssid[] = "BigBang"; //  your network SSID (name) 
char pass[] = "DiamondBack";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; // status of the wifi connection

// initialize the library instance:
WiFiClient client;

const unsigned long requestInterval = 30*1000;    // delay between requests; 30 seconds

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(199,59,149,200);    // numeric IP for api.twitter.com
char server[] = "api.twitter.com";     // name address for twitter API

boolean requested;                     // whether you've made a request since connecting
unsigned long lastAttemptTime = 0;     // last time you connected to the server, in milliseconds

String currentLine = "";               // string to hold the text from server
String tweet = "";                     // string to hold the tweet
boolean readingTweet = false;          // if you're currently reading the tweet

Twitter myTwitter;

void setup() {
  // reserve space for the strings:
  currentLine.reserve(256);
  tweet.reserve(150);
  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);  
 
    // wait 1 second for connection:
    delay(1000);
  } 
  // you're connected now, so print out the status:
  printWifiStatus();
  connectToServer();
  myTwitter = new Twitter("cmsc838f", "838fcmsc");
}

void loop()
{
  if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar; 

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      } 
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<text>")) {
        // tweet is beginning. Clear the tweet string:
        readingTweet = true; 
        tweet = "";
        // break out of the loop so this character isn't added to the tweet:
        return;
      }
      // if you're currently reading the bytes of a tweet,
      // add them to the tweet String:
      if (readingTweet) {
        if (inChar != '<') {
          tweet += inChar;
        } 
        else {
          // if you got a "<" character,
          // you've reached the end of the tweet:
          readingTweet = false;
          Serial.println(tweet);   
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
  }
}

void sendTweet(){
Serial.println("Sending tweet...");
try
  {
    Status status1 = twitter.updateStatus("There is someone on the door");//update twitter status
  }
  catch( TwitterException e)  {
    println(e.getStatusCode());
  }

}

void connectToServer() {
  // attempt to connect, and wait a millisecond:
  Serial.println("connecting to server...");
  if (client.connect(server, 80)) {
    Serial.println("making HTTP request...");
    sendTweet();
    //const char* msg = "Someone Knocked";
    //String encodedMsg = URLEncode(msg);
    // make HTTP GET request to twitter:
    //client.println("GET /1/statuses/user_timeline.xml?screen_name=rajankz HTTP/1.1");
    //client.println("POST /1/statuses/update.json?oauth_token=917007349-BZPttKvggVHRNWe7osDxaG5a1PFQINPMoMwd5v7o&oauth_token_secret=hgQQD0egnnz0dOI00cqBF38UTfnEzBi2Xa0RAGakg&status="+encodedMsg);
    //client.println("Host:api.twitter.com");
    //client.println("Connection:close");
    //client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}   


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/**
 * URL Encode a string.
 * 
 * Based on http://www.icosaedro.it/apache/urlencode.c
 *
 */
String URLEncode(const char* msg)
{
    const char *hex = "0123456789abcdef";
    String encodedMsg = "";

    while (*msg!='\0'){
        if( ('a' <= *msg && *msg <= 'z')
                || ('A' <= *msg && *msg <= 'Z')
                || ('0' <= *msg && *msg <= '9') ) {
            encodedMsg += *msg;
        } else {
            encodedMsg += '%';
            encodedMsg += hex[*msg >> 4];
            encodedMsg += hex[*msg & 15];
        }
        msg++;
    }
    return encodedMsg;
}




