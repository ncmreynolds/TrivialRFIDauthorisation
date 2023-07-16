/*
 * Example of the Trivial RFID authorisation library
 * 
 * https://github.com/ncmreynolds/TrivialRFIDauthorisation
 * 
 * This example will revoke the card's authorisation for a single ID when presented to the reader
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

uint8_t idToRevoke = 28;

void setup() {
  Serial.begin(115200);
  //rfid.debug(Serial); //Enables debugging. Doing this before 'begin' means you can see the reader initialisation information!
  if(rfid.begin()) { //Start the RFID reader
    Serial.println(F("*******************************"));
    Serial.print(F("Starting revocation of cards' authorisation for ID:"));
    Serial.println(idToRevoke);
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
    bool result = rfid.revokeCardAuthorisation(idToRevoke);
    Serial.println(F("*******************************"));
    Serial.print(F("Revoking card's authorisation for ID:"));
    Serial.print(idToRevoke);
    Serial.print(F(" - "));
    if(result == true)
    {
      Serial.println(F("success"));
    }
    else
    {
      Serial.println(F("failure"));
    }
    Serial.println(F("*******************************"));
  }
}
