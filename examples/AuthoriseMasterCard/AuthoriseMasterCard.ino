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

#if defined(ARDUINO_AVR_NANO)
TrivialRFIDauthorisation rfid(10);  //RFID reader CS is connected to pin 10 of an Arduino Nano
#elif defined(ARDUINO_ESP8266_WEMOS_D1MINI) || defined(ARDUINO_ESP8266_WEMOS_D1MINIPRO)
TrivialRFIDauthorisation rfid(D8);  //RFID reader CS is connected to pin D8 of a WeMos D1 mini
#elif defined(ARDUINO_ESP32S2_DEV)
TrivialRFIDauthorisation rfid(38);  //RFID reader CS is connected to pin 38 of a ESP32S2-Saola-1
#endif

void setup() {
  Serial.begin(115200);
  //rfid.debug(Serial); //Enables debugging. Doing this before 'begin' means you can see the reader initialisation information!
  if(rfid.begin()) { //Start the RFID reader
    Serial.println("*******************************");
    Serial.println("Starting authorisation of cards for ALL IDs");
    Serial.println("*******************************");
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
    bool result = rfid.authoriseCard();
    Serial.println("*******************************");
    Serial.print("Authorising card for all IDs - ");
    if(result == true)
    {
      Serial.println("success");
    }
    else
    {
      Serial.println("failure");
    }
    Serial.println("*******************************");
  }
}
