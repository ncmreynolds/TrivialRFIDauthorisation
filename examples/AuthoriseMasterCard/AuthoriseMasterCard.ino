/*
 * Example of the Trivial RFID authorisation library
 * 
 * https://github.com/ncmreynolds/TrivialRFIDauthorisation
 * 
 * This example will authorise the card for ALL IDs when presented to the reader
 * 
 * 
 */
#include <TrivialRFIDauthorisation.h>

TrivialRFIDauthorisation rfid(D8);  //RFID reader is on pin D8 of a D1 mini

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
    Serial.print("Authorising card for all IDs - ");
    if(rfid.authoriseCard() == true)
    {
      Serial.println("Success");
    }
    else
    {
      Serial.println("Failure");
    }
    Serial.println("*******************************");
  }
}
