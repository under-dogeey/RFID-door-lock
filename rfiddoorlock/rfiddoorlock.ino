
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


void postID(String id, int add)
{

  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Error in WiFi connection");
    return;
  }

  HTTPClient http;

  String fullServerUrl = serverUrl + id + "&add=" + add;
  //String fullServerUrl = hotspotserverUrl + id;

  Serial.println(fullServerUrl);

  http.begin(fullServerUrl);
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST("POSTING from ESP32");

  if(httpResponseCode != 200)
  {
    Serial.println(" Access denied");
    delay(3000);
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    return;
  }

  String response = http.getString();

  Serial.println("Authorized access");
  Serial.println();
  delay(3000);

  Serial.println(httpResponseCode);
  Serial.println(response);

  http.end();

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
    //Serial.println(String(currentMillis) + "-" + String(previousMillis) + "=" + String(currentMillis - previousMillis)); //debug
    ledState = HIGH;
    digitalWrite(ledPin, ledState);
    return;
  }

  ledState = LOW;
  digitalWrite(ledPin, ledState);

}

void scanCard()
{
  Serial.println("Card read OK");
  //Show UID on serial monitor
  Serial.print("UID tag : ");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  if (content && ledState == HIGH) //change here the UID of the card/cards that you want to give access
  {
    postID(content, 1);    
  }
  else
  {
    Serial.println("Not in Read Mode");
    postID(content, 0);
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
 
void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication

  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522

  pinMode(IRQ_PIN, INPUT_PULLUP);
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), setFlag, FALLING);

  Serial.println("Approximate your card to the reader...");
  Serial.println();

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