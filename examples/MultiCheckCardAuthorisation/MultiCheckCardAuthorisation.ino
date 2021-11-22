/*
 * Example of the Trivial RFID authorisation library
 * 
 * https://github.com/ncmreynolds/TrivialRFIDauthorisation
 * 
 * This example will check authorisation of the card for a multiple IDs when presented to the reader
 * 
 * 
 */
#include <TrivialRFIDauthorisation.h>

TrivialRFIDauthorisation rfid(D8);  //RFID reader is on pin D8 of a D1 mini

uint8_t idsToCheck[] = {2,254}; //Checks flags at the beginning and end of the data

void setup() {
  Serial.begin(115200);
  Serial.println("*******************************");
  Serial.println("Checking authorisation of cards");
  Serial.println("*******************************");
  rfid.debug(Serial); //Enables debugging. Doing this before 'begin' means you can see the reader initialisation information!
  rfid.begin(); //Start the authorisation
}

void loop() {
  rfid.pollForCard(); //Must run regularly to read and process the card
  if(rfid.cardPresent() == true && rfid.cardChanged() == true)
  {
    Serial.println("*******************************");
    Serial.print("Checking card for IDs:");
    for(uint8_t i = 0; i < sizeof(idsToCheck); i++)
    {
      Serial.print(idsToCheck[i]);
      if(i < sizeof(idsToCheck) - 1)
      {
        Serial.print('/');
      }
      else
      {
        Serial.println();
      }
    }
    if(rfid.checkCardAuthorisation(idsToCheck,sizeof(idsToCheck)) == true)
    {
      Serial.println(" Success, at least one checked ID authorised");
    }
    else
    {
      Serial.println(" Failure, no checked IDs authorised");
    }
    Serial.println("*******************************");
  }
}
