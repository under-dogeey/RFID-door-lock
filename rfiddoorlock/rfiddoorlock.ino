
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
 
#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

const char* ssid = "KTWL2";
const  char* password = "robertwl";
String serverUrl = "http://192.168.1.21:8000/hello?id=";

const char* data = "Callback function called";

static int callback(void *data, int argc, char **argv, char **azColName) 
{
   int i;
   Serial.printf("%s: ", (const char*)data);
   for (i = 0; i<argc; i++){
       Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   Serial.printf("\n");
   return 0;
}

int db_open(const char *filename, sqlite3 **db) 
{
   int rc = sqlite3_open(filename, db);
   if (rc) {
       Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
       return rc;
   } else {
       Serial.printf("Opened database successfully\n");
   }
   return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql) 
{
   Serial.println(sql);
   long start = micros();
   int rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if (rc != SQLITE_OK) {
       Serial.printf("SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
   } else {
       Serial.printf("Operation done successfully\n");
   }
   Serial.print(F("Time taken:"));
   Serial.println(micros()-start);
   return rc;
}

// Set web server port number to 80
WiFiServer server(80);

void database()
{
  int rc;
  sqlite3 *db1;
  // remove existing file
   SPIFFS.remove("/cardData.db");

   sqlite3_initialize();

   if (db_open("/spiffs/test1.db", &db1))
       return;

   rc = db_exec(db1, "CREATE TABLE cardData (id TEXT);");
   if (rc != SQLITE_OK) {
       sqlite3_close(db1);
       return;
   }

   rc = db_exec(db1, "INSERT INTO cardData VALUES ('testdata');");
   if (rc != SQLITE_OK) {
       sqlite3_close(db1);
       return;
   }

   rc = db_exec(db1, "SELECT * FROM test1");
   if (rc != SQLITE_OK) {
       sqlite3_close(db1);
       return;
   }

   sqlite3_close(db1);
}

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
      database();
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

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
       Serial.println("Failed to mount file system");
       return;
   }

   // list SPIFFS contents
   File root = SPIFFS.open("/");
   if (!root) {
       Serial.println("- failed to open directory");
       return;
   }
   if (!root.isDirectory()) {
       Serial.println(" - not a directory");
       return;
   }
   File file = root.openNextFile();
   while (file) {
       if (file.isDirectory()) {
           Serial.print("  DIR : ");
           Serial.println(file.name());
       } else {
           Serial.print("  FILE: ");
           Serial.print(file.name());
           Serial.print("\tSIZE: ");
           Serial.println(file.size());
       }
       file = root.openNextFile();
   }

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
  Serial.println("Approximate your card to the reader...");
  Serial.println();

}

void loop() 
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
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
 
 else   {

  }
} 