/*
 *	Wrapper class for MFRC522v2
 *
 */
#ifndef TrivialRFIDauthorisation_h
#define TrivialRFIDauthorisation_h
#include <Arduino.h>
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#define TrivialRFIDauthorisationSupportDebugging
//#define TrivialRFIDauthorisationSupportEncryption
#if defined(ESP32)
#include <SPI.h>
#endif

class TrivialRFIDauthorisation {

	public:
		TrivialRFIDauthorisation(uint8_t cspin);								//Constructor function
		#if defined(ESP32)
		TrivialRFIDauthorisation(uint8_t clkPin, uint8_t cipoPin,
			uint8_t copiPin, uint8_t csPin);									//Constructor function with non-default SPI pins, which can be set freely on an ESP32
		#endif
		~TrivialRFIDauthorisation();											//Destructor function
		bool begin(uint8_t sector = 1);											//Start the RFID authentication on a specific sector
		#ifdef TrivialRFIDauthorisationSupportDebugging
		void debug(Stream &);													//Enable debug output on a stream
		#endif
		bool authoriseCard();													//Authorise this card for all IDs (For admins maybe?)
		bool authoriseCard(uint8_t, bool append);								//Authorise this card for an ID
		bool authoriseCard(const uint8_t*, uint8_t, bool append);				//Authorise this card for multiple IDs
		bool revokeCardAuthorisation();											//Revoke all authorisations of this card
		bool revokeCardAuthorisation(uint8_t);									//Revoke authorisation of this card for an ID
		bool revokeCardAuthorisation(const uint8_t*, uint8_t);					//Revoke authorisation of this card for multiple IDs
		bool checkCardAuthorisation(uint8_t);									//Is this card authorised for this ID?
		bool checkCardAuthorisation(const uint8_t*, uint8_t);					//Is this card authorised for any of these IDs?
		bool pollForCard();														//Poll to check if a card is there
		bool cardPresent();														//Has a card just been presented?
		bool cardChanged();														//Has it changed since the last card?
		uint8_t* cardUID();														//Retrieve a pointer to the current UID
		bool cardUID(uint32_t &uid);											//Retrieve the current UID into an uint32_t, if possible
		uint8_t cardUIDsize();													//Size of the current UID
		uint8_t cardKeySize();													//Size of the current card key
		void setDefaultCardKeyA();												//Set the default card key for reading
		void setDefaultCardKeyB();												//Set the default card key for writing
		#ifdef TrivialRFIDauthorisationSupportEncryption
		void setCustomCardKeyA(uint8_t *key);									//Set a custom card key for reading. Note this doesn't change the existing key A of a card.
		void setCustomCardKeyB(uint8_t *key);									//Set a custom card key for writing. Note this doesn't change the existing key B of a card.
		#endif
	protected:
	private:
		#ifdef TrivialRFIDauthorisationSupportDebugging
		Stream *debugStream_ = nullptr;											//The stream used for debugging
		#endif
		MFRC522DriverPinSimple rfid_ss_pin_;
		MFRC522DriverSPI rfid_driver_;
		MFRC522 rfid_reader_;
		MFRC522::MIFARE_Key keyA_;
		MFRC522::MIFARE_Key keyB_;
		#ifdef TrivialRFIDauthorisationSupportEncryption
		bool keyAset_ = false;													//true if KeyA set
		bool keyBset_ = false;													//true if KeyA set
		#endif
		uint8_t self_test_retries_ = 3;											//Sometimes the self test fails on the first try
		bool rfid_antenna_enabled_ = true;										//Tracks state of the RFID antenna
		bool card_awake_ = false;												//Tracks awake state of the card, which will go idle/halt after transactions
		uint8_t current_uid_[10];												//UID of last presented card
		uint8_t current_uid_size_ = 4;											//UID size will be 4, 7 or 10
		bool card_present_ = false;												//Is card present
		bool card_changed_ = false;												//Has card changed
		uint8_t rfid_read_failures_ = 0;										//Count up before considering a card removed
		uint8_t rfid_read_failure_threshold_ = 2;								//Threshold to hit for card removal
		uint32_t rfid_reader_last_polled_ = 0;									//Timer for regular polling of RFID
		uint32_t rfid_reader_polling_interval_ = 250;							//Timer for regular polling of RFID
		uint8_t card_flags_[32];												//Take a copy of the flags on the card
		uint8_t flags_start_block_ = 4;											//Block to start flags from
		uint8_t flags_start_sector_ = 1;										//Sector to start flags from
		bool flagsRead_ = false;												//Cache flags once read, the library maintains a copy
		uint8_t trailerBlock_  = 7;												//Trailer block
		//bool PICC_IsCardPresent();												//Similar to PICC_IsNewCardPresent from the library but covers existing cards
		bool wakeCard_();														//Wake teh card when idle/halted
		bool authenticateWithCardForRead_(uint8_t block);						//Authenticate with keyA_
		bool authenticatedWithCardForRead_ = false;								//Track auth state
		bool authenticateWithCardForWrite_(uint8_t block);						//Authenticate with keyB_
		bool authenticatedWithCardForWrite_ = false;								//Track auth state
		void deAuthenticateWithCard_();											//Deauthenticate after a transaction is done
		bool readCardFlags_();													//Read the flags from a card
		bool writeCardFlags_();													//Write the flags to a card
};

#endif
