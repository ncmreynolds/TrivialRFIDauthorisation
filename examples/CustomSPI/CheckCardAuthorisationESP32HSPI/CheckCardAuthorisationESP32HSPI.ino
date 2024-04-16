/*
 * Example of the Trivial RFID authorisation library
 * 
 * https://github.com/ncmreynolds/TrivialRFIDauthorisation
 * 
 * This example will authorise the card for a single ID when presented to the reader
 * 
 * Here you can see how to change the pins used for connecting the reader to the second SPI interface (HSPI) on an ESP32
 * 
 * Unlike many other microcontrollers ESP32s can be flexibly configured use _most_ of their GPIO pins for hardware SPI and have multiple SPI interfaces
 * 
 * This example was tested with an ESP32S3 but should work with most ESP32s if you set appropriate pins
 * 
 */
#include <TrivialRFIDauthorisation.h>

#if defined(ESP32)
  const int rfidCsPin = 47;       //Set the CS pin
  const int rfidClkPin = 48;      //Set the CLK pin
  const int rfidCopiPin = 45;     //Set the COPI pin
  const int rfidCipoPin = 38;     //Set the CIPO pin
  SPIClass hspi = SPIClass(HSPI); //Create an instance of an SPI driver specifically for the RFID reader
  TrivialRFIDauthorisation rfid(hspi, rfidClkPin, rfidCipoPin, rfidCopiPin, rfidCsPin);
#else
  #error This sketch will only work on an ESP32
#endif

uint8_t idToAuthorise = 28;

void setup() {
  Serial.begin(115200);
  //rfid.debug(Serial); //Enables debugging. Doing this before 'begin' means you can see the reader initialisation information!
  if(rfid.begin()) { //Start the RFID reader
    Serial.println(F("*******************************"));
    Serial.println(F("Checking authorisation of cards"));
    Serial.println(F("*******************************"));
  }
  else
  {
    Serial.println(F("*******************************"));
    Serial.println(F("RFID reader self test failed"));
    Serial.println(F("*******************************"));
    delay(0);
  }
}

void loop() {
  rfid.pollForCard(); //Must run regularly to read and process the card
  if(rfid.cardPresent() == true && rfid.cardChanged() == true)
  {
    bool result = rfid.checkCardAuthorisation(idToAuthorise);
    Serial.println(F("********************************"));
    Serial.print(F("Checking card for authorisation against ID:"));
    Serial.print(idToAuthorise);
    Serial.print(F(" - "));
    if(result == true)
    {
      Serial.println(F("authorised"));
    }
    else
    {
      Serial.println(F("not authorised"));
    }
    Serial.println(F("********************************"));
  }
}
