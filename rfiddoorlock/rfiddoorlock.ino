
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
bool addCardMode = true;
bool ledState = HIGH;
int lastButtonState = HIGH;

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

IRAM_ATTR void readCard()
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

  /*if(WiFi.status() == WL_CONNECTED)
  {

    HTTPClient http;

    String fullServerUrl = serverUrl + id + "&add=" + add;
    //String fullServerUrl = hotspotserverUrl + id;

    Serial.println(fullServerUrl);

    http.begin(fullServerUrl);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST("POSTING from ESP32");

    if(httpResponseCode == 200)
    {
      String response = http.getString();

      Serial.println("Authorized access");
      Serial.println();
      delay(3000);

      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else
    {
      Serial.println(" Access denied");
      delay(3000);
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();

  }
  else
  {
    Serial.println("Error in WiFi connection");
  }*/
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
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);

  Serial.println("Approximate your card to the reader...");
  Serial.println();

}

void loop() 
{

  int buttonState = digitalRead(buttonPin);
  unsigned long currentMillis = millis();
  //Serial.println(buttonState);
  // Look for new cards
  
  if(buttonState == LOW && lastButtonState == HIGH)
  {
    if(!addCardMode)
    {
      addCardMode = true;
      ledState = HIGH;
      previousMillis = currentMillis;
    }
    else
    { 
      addCardMode = false;
      ledState = LOW;
      flag = false;
      rearmIRQ();
    }

    digitalWrite(ledPin, ledState);
  }

  lastButtonState = buttonState;

  if(addCardMode)
  { 
    //Serial.println(String(currentMillis) + "-" + String(previousMillis) + "=" + String(currentMillis - previousMillis));

    if(currentMillis - previousMillis >= interval)
    {
      Serial.println("Timed out");
      addCardMode = false;
      ledState = LOW;
      flag = false;
      digitalWrite(ledPin, ledState);
        
    }
  }

  if(flag)
  {    

    /*if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      flag = false;
      rearmIRQ();
    }*/
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      //Serial.println("No Card Present!");
      flag = false;
      rearmIRQ();
    }
    else
    {
      //Serial.println("4");
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

      if (content) //change here the UID of the card/cards that you want to give access
      {
        if(addCardMode)
        {
          postID(content, 1);
        }
        else
        {
          Serial.println("Not in Read Mode");
          postID(content, 0);
        }
        
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      flag = false;
      rearmIRQ();

      addCardMode = false;
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      previousMillis = currentMillis;
      delay(1000);
      
    }
  }            
    mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
    mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);  

  delay(10);
  
} 