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

#if defined(ARDUINO_AVR_NANO)
TrivialRFIDauthorisation rfid(10);  //RFID reader CS is connected to pin 10 of an Arduino Nano
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO)
TrivialRFIDauthorisation rfid(D8);  //RFID reader CS is connected to pin D8 of a WeMos D1 mini
#elif defined(ARDUINO_ESP32S2_DEV)
TrivialRFIDauthorisation rfid(38);  //RFID reader CS is connected to pin 38 of a ESP32S2-Saola-1
#endif

uint8_t idsToCheck[] = {0,1,2,3,4,5,6,7,28,248,249,250,251,252,253,254,255}; //Checks flags at the beginning and end of the data

void setup() {
  Serial.begin(115200);
  //rfid.debug(Serial); //Enables debugging. Doing this before 'begin' means you can see the reader initialisation information!
  if(rfid.begin()) { //Start the RFID reader
    Serial.println(F("*******************************"));
    Serial.print(F("Checking authorisation of card for IDs: "));
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
    bool result = rfid.checkCardAuthorisation(idsToCheck,sizeof(idsToCheck));
    Serial.println(F("*******************************"));
    Serial.print("Checking card for IDs:");
    for(uint8_t i = 0; i < sizeof(idsToCheck); i++)
    {
      Serial.print(idsToCheck[i]);
      if(i < sizeof(idsToCheck) - 1)
      {
        Serial.print('/');
      }
    }
    if(result == true)
    {
      Serial.println(F(" success, at least one checked ID authorised"));
    }
    else
    {
      Serial.println(F(" failure, no checked IDs authorised"));
    }
    Serial.println(F("*******************************"));
  }
}
