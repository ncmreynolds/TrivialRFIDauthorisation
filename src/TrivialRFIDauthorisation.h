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

class TrivialRFIDauthorisation {

	public:
		TrivialRFIDauthorisation(uint8_t sspin);								//Constructor function
		~TrivialRFIDauthorisation();											//Destructor function
		void begin(uint8_t sector = 0);											//Start the RFID authentication on a specific sector
		void debug(Stream &);													//Enable debug output on a stream
		bool authoriseCard();													//Authorise this card for all IDs (For admins maybe?)
		bool authoriseCard(uint8_t);											//Authorise this card for an ID
		bool authoriseCard(const uint8_t*, uint8_t);							//Authorise this card for multiple IDs
		bool revokeCard();														//Revoke all authorisations of this card
		bool revokeCard(uint8_t);												//Revoke authorisation of this card for an ID
		bool revokeCard(const uint8_t*, uint8_t);								//Revoke authorisation of this card for multiple IDs
		bool checkCardAuthorisation(uint8_t);									//Is this card authorised for this ID?
		bool checkCardAuthorisation(const uint8_t*, uint8_t);					//Is this card authorised for any of these IDs?
		bool pollForCard();														//Poll to check if a card is there
		bool cardPresent();														//Is a card just present?
		bool cardChanged();														//Has it changed?
		uint8_t* cardUID();														//Retrieve a pointer to the current UID
		uint8_t cardUIDsize();													//Size of the current UID
	protected:
	private:
		Stream *debugStream_ = nullptr;											//The stream used for debugging
		MFRC522DriverPinSimple rfid_ss_pin_;
		MFRC522DriverSPI rfid_driver_;
		MFRC522 rfid_reader_;
		MFRC522::MIFARE_Key key_;
		bool rfid_antenna_enabled_ = true;										//Tracks state of the RFID antenna
		uint8_t current_uid_[10];												//UID of last presented card
		uint8_t current_uid_size_ = 4;											//UID size will be 4, 7 or 10
		bool card_present_ = false;												//Is card present
		bool card_changed_ = false;												//Has card changed
		uint8_t rfid_read_failures_ = 0;										//Count up before considering a card removed
		uint8_t rfid_read_failure_threshold_ = 2;								//Threshold to hit for card removal
		uint32_t rfid_reader_last_polled_ = 0;									//Timer for regular polling of RFID
		uint32_t rfid_reader_polling_interval_ = 100;							//Timer for regular polling of RFID
		uint8_t card_flags_[32];												//Take a copy of the flags on the card
		uint8_t flags_start_block_ = 4;											//Block to start flags from
		uint8_t flags_start_sector_ = 1;										//Sector to start flags from
		uint8_t trailerBlock_  = 7;
		bool authenticateWithCard_();											//Authenticate with Key_A
		void deAuthenticateWithCard_();											//Deauthenticate after a transaction is done
		bool readCardFlags_();													//Read the flags from a card
		bool writeCardFlags_();													//Write the flags to a card
};

#endif
