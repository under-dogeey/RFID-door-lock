
#include <stdio.h>
#include <stdlib.h>

#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
 
#define SS_PIN 5
#define RST_PIN 22
#define IRQ_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

const char* ssid = "KTWL2";
const  char* password = "robertwl";
String serverUrl = "http://192.168.1.21:8000/hello?id=";

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


void postID(String id)
{
  if(WiFi.status() == WL_CONNECTED)
  {

    HTTPClient http;

    String fullServerUrl = serverUrl + id;

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
  }
}
 
void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication

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
  // Look for new cards
  if(flag)
  {
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      flag = false;
      rearmIRQ();
    }
    // Select one of the cards
    else if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      Serial.println("read failed");
      flag = false;
      rearmIRQ();
    }
    else
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

      if (content) //change here the UID of the card/cards that you want to give access
      {
        postID(content);
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      flag = false;
      rearmIRQ();
      delay(1000);

    }
    

  }
  else
  {
    delay(10);
  }

  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);

} 