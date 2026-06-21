#include <MFRC522.h>
#define SS_PIN 5
#define RST_PIN 22
#define IRQ_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

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

void setup() {
  Serial.begin(9600); 

  SPI.begin();
  mfrc522.PCD_Init();   // Initiate MFRC522

  pinMode(IRQ_PIN, INPUT_PULLUP);
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0);
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readCard, FALLING);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(flag)
  {
    //Serial.println("interrupt fired");
    if ( ! mfrc522.PICC_IsNewCardPresent()) 
    {
      //Serial.println("no card");
      flag = false;
      rearmIRQ();
    }
    // Select one of the cards
    else if ( ! mfrc522.PICC_ReadCardSerial()) 
    {
      Serial.println("read fialed");
      flag = false;
      rearmIRQ();
    }
    else
    {
      Serial.println("Card read OK");

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
