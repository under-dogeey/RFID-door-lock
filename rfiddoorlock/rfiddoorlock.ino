
#include <stdio.h>
#include <stdlib.h>

#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
 
#define SS_PIN 5
#define RST_PIN 22
#define IRQ_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

const int buttonPin = 16;
const int ledPin =  17;
bool ledState = LOW;

unsigned long currentMillis;
unsigned long previousMillis = 0;
const long interval = 30000;

const char* ssid = "KTWL2";
const  char* password = "robertwl";

const char* hotspotid = "roberttrinh";
const char* hotspotpass = "?????????";

String serverUrl = "http://192.168.1.21:8000/hello?id=";
String hotspotserverUrl = "http://172.20.10.2:8000/hello?id=";

bool flag = false;

void rearmIRQ()
{
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0x7F);
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0);
}

IRAM_ATTR void setFlag()
{
  flag = true;
}

// Set web server port number to 80
WiFiServer server(80);


String postID(String id, int add)
{

  String message = "";

  if(WiFi.status() != WL_CONNECTED)
  {
    message = "Error in WiFi connection\n";
    return message;
  }

  HTTPClient http;

  String fullServerUrl = serverUrl + id + "&add=" + add;
  //String fullServerUrl = hotspotserverUrl + id;


  message += ("\n" + fullServerUrl + "\n");

  http.begin(fullServerUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST("POSTING from ESP32");

  if(httpResponseCode != 200)
  {
    message += " Access denied\n";
    delay(3000);
    message += "Error on sending POST: " + (String(httpResponseCode) + "\n");
    return message;
  }

  String response = http.getString();

  message += "Authorized access\n";
  delay(3000);

  message += (String(httpResponseCode) + "\n");

  message += (String(response) + "\n");

  http.end();

  return message;

}

void handleAddState()
{
  int buttonState = digitalRead(buttonPin);
  currentMillis = millis();
  // Look for new cards

  if(buttonState == HIGH)
  {
    previousMillis = currentMillis;
  }

  if(currentMillis - previousMillis < interval)
  {
    //logMessage(String(currentMillis) + "-" + String(previousMillis) + "=" + String(currentMillis - previousMillis)); //debug
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    return;
  }

  ledState = LOW;
  digitalWrite(ledPin, ledState);

}

void scanCard()
{
  logMessage("Card read OK");
  //Show UID on serial monitor

  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {

    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  logMessage("UID tag : " + content);

  if (content && ledState == HIGH) //change here the UID of the card/cards that you want to give access
  {
    logMessage("Message : " + postID(content, 1));
    //postID(content, 1);    
  }
  else
  {
    logMessage("Message : " + postID(content, 0));
    //logMessage("Not in Read Mode");
    //postID(content, 0);
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  flag = false;
  rearmIRQ();

  ledState = LOW;
  digitalWrite(ledPin, ledState);
  previousMillis = currentMillis;
  delay(1000);
}

void logMessage(String str)
{
  Serial.println("\n[" + String(millis()) + "] " + str);
}
 
void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  logMessage("Connecting to " + String(ssid));
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  logMessage("WiFi connected.");
  logMessage("IP address: " + WiFi.localIP().toString());

  server.begin();

  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  pinMode(IRQ_PIN, INPUT_PULLUP);
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), setFlag, FALLING);

  logMessage("Approximate your card to the reader...");


}

void loop() 
{

  handleAddState();

  if(flag && mfrc522.PICC_ReadCardSerial())
  {    

    scanCard();
      
  }

    //Serial.println("No Card Present!");
    flag = false;
    rearmIRQ();

    mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
    mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);  
    delay(10);
  
} 