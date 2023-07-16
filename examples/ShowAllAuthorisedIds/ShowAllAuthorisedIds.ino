/*
 * Example of the Trivial RFID authorisation library
 * 
 * https://github.com/ncmreynolds/TrivialRFIDauthorisation
 * 
 * This example will show all the authorised IDs when presented to the reader
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
    Serial.println(F("*******************************"));
    Serial.println(F("Showing authorised IDs for cards"));
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
    Serial.println(F("********************************"));
    Serial.print(F("Authorised IDs:"));
    bool first = true;
    for(uint16_t id = 0; id <= 255; id++)
    {
      bool result = rfid.checkCardAuthorisation((uint8_t)id);
      if(result == true)
      {
        if(first == false)
        {
          Serial.print('/');
        }
        else
        {
          first = false;
        }
        Serial.print(id);
      }
    }
    if(first == true)
    {
      Serial.println(F("none"));
    }
    else
    {
      Serial.println();
    }
    Serial.println(F("********************************"));
  }
}
