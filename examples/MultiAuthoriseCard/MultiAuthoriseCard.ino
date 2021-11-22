/*
 * Example of the Trivial RFID authorisation library
 * 
 * https://github.com/ncmreynolds/TrivialRFIDauthorisation
 * 
 * This example will authoris the card for a single ID when presented to the reader
 * 
 * 
 */
#include <TrivialRFIDauthorisation.h>

TrivialRFIDauthorisation rfid(D8);  //RFID reader is on pin D8 of a D1 mini

uint8_t idsToAuthorise[] = {0,1,2,3,4,5,6,7,248,249,250,251,252,253,254,255}; //Adds flags at the beginning and end of the data

void setup() {
  Serial.begin(115200);
  Serial.println("*******************************");
  Serial.println("Starting authorisation of cards");
  Serial.println("*******************************");
  rfid.debug(Serial); //Enables debugging. Doing this before 'begin' means you can see the reader initialisation information!
  rfid.begin(); //Start the authorisation
}

void loop() {
  rfid.pollForCard(); //Must run regularly to read and process the card
  if(rfid.cardPresent() == true && rfid.cardChanged() == true)
  {
    Serial.println("*******************************");
    Serial.print("Authorising card for IDs:");
    for(uint8_t i = 0; i < sizeof(idsToAuthorise); i++)
    {
      Serial.print(idsToAuthorise[i]);
      if(i < sizeof(idsToAuthorise) - 1)
      {
        Serial.print('/');
      }
      else
      {
        Serial.println();
      }
    }
    if(rfid.authoriseCard(idsToAuthorise,sizeof(idsToAuthorise)) == true)
    {
      Serial.println(" Success");
    }
    else
    {
      Serial.println(" Failure");
    }
    Serial.println("*******************************");
  }
}
