#ifndef TrivialRFIDauthorisation_cpp
#define TrivialRFIDauthorisation_cpp
#include <TrivialRFIDauthorisation.h>

TrivialRFIDauthorisation::TrivialRFIDauthorisation(const uint8_t pin)	:	//Constructor function and member constructors
	rfid_ss_pin_(pin),
	rfid_driver_{rfid_ss_pin_},
	rfid_reader_{rfid_driver_}
{
}

TrivialRFIDauthorisation::~TrivialRFIDauthorisation()	//Destructor function
{
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::begin(const uint8_t sector) {
#else
void TrivialRFIDauthorisation::begin(const uint8_t sector) {
#endif
	//Initialise RFID reader
	rfid_reader_.PCD_Init();
	if(debugStream_ != nullptr)
	{
		debugStream_->println(F("Initialising RFID reader"));
		debugStream_->print(F("MFRC522 Self test: "));
		if (rfid_reader_.PCD_PerformSelfTest() == true)	{
			debugStream_->println(F("OK"));
		}
		else	{
			debugStream_->println(F("Fail"));
		}
	}
	for (uint8_t i = 0; i < MFRC522::MIFARE_Misc::MF_KEY_SIZE; i++) {
		key_.keyByte[i] = 0xFF;
	}
	if(sector > 15)
	{
		flags_start_sector_ = 15;
	}
	else
	{
		flags_start_sector_ = sector;
	}
	if(flags_start_sector_ == 0)
	{
		flags_start_block_ = 1;
	}
	else
	{
		flags_start_block_ =  flags_start_sector_ * 4;
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print("Using sector:");
		debugStream_->print(flags_start_sector_);
		debugStream_->print(" block ");
		debugStream_->print(flags_start_block_);
		debugStream_->print('&');
		debugStream_->print(flags_start_block_+1);
		debugStream_->println(" to store authentication bitmask");
	}
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::debug(Stream &debugStream)
#else
void TrivialRFIDauthorisation::debug(Stream &debugStream)
#endif
{
	debugStream_ = &debugStream;		//Set the stream used for the terminal
	#if defined(ESP8266)
	if(&debugStream == &Serial)
	{
		  debugStream_->write(17);		//Send an XON to stop the hung terminal after reset on ESP8266
	}
	#endif
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::pollForCard() {
#else
bool TrivialRFIDauthorisation::pollForCard() {
#endif
	if(millis() - rfid_reader_last_polled_ > rfid_reader_polling_interval_)
	{
		rfid_reader_last_polled_ = millis();
		/*if(debugStream_ != nullptr)
		{
			debugStream_->print(F("Polling card:"));
		}*/
		if(rfid_reader_.PICC_IsNewCardPresent() == false) {
			if(rfid_read_failures_++ >= rfid_read_failure_threshold_)
			{
				rfid_read_failures_ = rfid_read_failure_threshold_;
				if(card_present_ == true) {
					if(debugStream_ != nullptr)
					{
						debugStream_->println(F("Card removed"));
					}
					for(uint8_t i = 0; i < current_uid_size_; i++) {
						current_uid_[i] = 0;
					}
					current_uid_size_ = 0;
					card_present_ = false;
				}
				return(false);
			}
			/*if(debugStream_ != nullptr)
			{
				debugStream_->println(F("not present"));
			}*/
			return(false);
		}
		if(rfid_reader_.PICC_ReadCardSerial() == false) {
			if(rfid_read_failures_++ >= rfid_read_failure_threshold_)
			{
				rfid_read_failures_ = rfid_read_failure_threshold_;
				if(card_present_ == true) {
					if(debugStream_ != nullptr)
					{
						debugStream_->println(F("Card removed"));
					}
					for(uint8_t i = 0; i < current_uid_size_; i++) {
						current_uid_[i] = 0;
					}
					current_uid_size_ = 0;
					card_present_ = false;
				}
				return(false);
			}
			/*if(debugStream_ != nullptr)
			{
				debugStream_->println(F("can't read PICC"));
			}*/
			return(false);
		}
		/*if(debugStream_ != nullptr)
		{
			debugStream_->println(F("present"));
		}*/
		card_present_ = true;
		card_changed_ = false;
		rfid_read_failures_ = 0;
		if(current_uid_size_ != rfid_reader_.uid.size)
		{
			current_uid_size_ = rfid_reader_.uid.size;
			card_changed_ = true;
		}
		for(uint8_t i = 0; i < rfid_reader_.uid.size; i++) {
			if(current_uid_[i] != rfid_reader_.uid.uidByte[i])
			{
				current_uid_[i] = rfid_reader_.uid.uidByte[i];
				card_changed_ = true;
			}
		}
		if(card_present_ == true && debugStream_ != nullptr)	{
			if(card_changed_)	{
				debugStream_->print(F("New card presented "));
			} else {
				debugStream_->print(F("Card present "));
			}
			debugStream_->print("UID:");
			for(uint8_t i = 0; i < current_uid_size_; i++)
			{
				debugStream_->print(current_uid_[i], HEX);
				if(i < current_uid_size_ - 1)
				{
					debugStream_->print(':');
				}
			}
			debugStream_->println();
		}
		return(true);
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authenticateWithCard_(uint8_t block) {
#else
bool TrivialRFIDauthorisation::authenticateWithCard_() {
#endif
	if(debugStream_ != nullptr) {
		debugStream_->print(F("Authenticating PICC using key A:"));
	}
	MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.PCD_Authenticate(MFRC522Constants::PICC_Command::PICC_CMD_MF_AUTH_KEY_A, block, &key_, &(rfid_reader_.uid));
	if (status != MFRC522::StatusCode::STATUS_OK) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("PCD_Authenticate() failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		return(false);
	}
	else if(debugStream_ != nullptr)
	{
		debugStream_->println(F("success"));
	}
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::readCardFlags_() {
#else
bool TrivialRFIDauthorisation::readCardFlags_() {
#endif
	uint8_t buffer[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Needs two bytes for the CRC
	uint8_t size = sizeof(buffer);
	if(debugStream_ != nullptr)
	{
		//MFRC522Debug::PICC_DumpMifareClassicSectorToSerial(rfid_reader_, Serial, &(rfid_reader_.uid), &key_, flags_start_sector_);
		debugStream_->print("MIFARE_Read() sector ");
		debugStream_->print(flags_start_sector_);
		debugStream_->print(" block ");
		debugStream_->print(flags_start_block_);
		debugStream_->print(" start: ");
	}
	MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Read(flags_start_block_, buffer, &size);
	if (status != MFRC522::StatusCode::STATUS_OK) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		return(false);
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print(F("success: "));
	}
	for (uint8_t i = 0; i < 16; i++) {
		card_flags_[i] = buffer[i];
		if(debugStream_ != nullptr)
		{
			debugStream_->print(buffer[i], HEX);
			if(i<15)
			{
				debugStream_->print(':');
			}
			else
			{
				debugStream_->println();
			}
		}
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print("MIFARE_Read() sector ");
		debugStream_->print(flags_start_sector_);
		debugStream_->print(" block ");
		debugStream_->print(flags_start_block_+1);
		debugStream_->print(" start: ");
	}
	status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Read(flags_start_block_+1, buffer, &size);
	if (status != MFRC522::StatusCode::STATUS_OK) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		return(false);
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print(F("success: "));
	}
	for (uint8_t i = 0; i < 16; i++) {
		card_flags_[i + 16] = buffer[i];
		if(debugStream_ != nullptr)
		{
			debugStream_->print(buffer[i], HEX);
			if(i<15)
			{
				debugStream_->print(':');
			}
			else
			{
				debugStream_->println();
			}
		}
	}
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::writeCardFlags_() {
#else
bool TrivialRFIDauthorisation::writeCardFlags_() {
#endif
	if(debugStream_ != nullptr)
	{
		//MFRC522Debug::PICC_DumpMifareClassicSectorToSerial(rfid_reader_, Serial, &(rfid_reader_.uid), &key_, flags_start_sector_);
		debugStream_->print("MIFARE_Write() sector ");
		debugStream_->print(flags_start_sector_);
		debugStream_->print(" block ");
		debugStream_->print(flags_start_block_);
		debugStream_->print(" start: ");
	}
	MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Write(flags_start_block_, &card_flags_[0], 16);
	if (status != MFRC522::StatusCode::STATUS_OK) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		return(false);
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print(F("success: "));
	}
	for (uint8_t i = 0; i < 16; i++) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(card_flags_[i], HEX);
			if(i<15)
			{
				debugStream_->print(':');
			}
			else
			{
				debugStream_->println();
			}
		}
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print("MIFARE_Write() sector ");
		debugStream_->print(flags_start_sector_);
		debugStream_->print(" block ");
		debugStream_->print(flags_start_block_+1);
		debugStream_->print(" start: ");
	}
	status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Write(flags_start_block_+1, &card_flags_[16], 16);
	if (status != MFRC522::StatusCode::STATUS_OK) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		return(false);
	}
	if(debugStream_ != nullptr)
	{
		debugStream_->print(F("success: "));
	}
	for (uint8_t i = 0; i < 16; i++) {
		if(debugStream_ != nullptr)
		{
			debugStream_->print(card_flags_[i + 16], HEX);
			if(i<15)
			{
				debugStream_->print(':');
			}
			else
			{
				debugStream_->println();
			}
		}
	}
	//MFRC522Debug::PICC_DumpMifareClassicSectorToSerial(rfid_reader_, Serial, &(rfid_reader_.uid), &key_, flags_start_sector_);
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::deAuthenticateWithCard_()
#else
void TrivialRFIDauthorisation::deAuthenticateWithCard_()
#endif
{
	rfid_reader_.PICC_HaltA();
	rfid_reader_.PCD_StopCrypto1();
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardChanged() {
#else
bool TrivialRFIDauthorisation::cardChanged() {
#endif
	if(card_changed_)
	{
		card_changed_ = false;
		return(true);
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardPresent() {
#else
bool TrivialRFIDauthorisation::cardPresent() {
#endif
	return(card_present_);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authoriseCard() {
#else
bool TrivialRFIDauthorisation::authoriseCard() {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	for(uint8_t i = 0; i < 32; i++)
	{
		card_flags_[i] = 0xff;	//Set ALL the bits
	}
	if(writeCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authoriseCard(const uint8_t id) {
#else
bool TrivialRFIDauthorisation::authoriseCard(const uint8_t id) {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	card_flags_[id>>3] = card_flags_[id>>3] | 0x01<<(id%8);	//Set the specific bit
	if(writeCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authoriseCard(const uint8_t* ids, const uint8_t numberOfIds) {
#else
bool TrivialRFIDauthorisation::authoriseCard(const uint8_t* ids, const uint8_t numberOfIds) {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	for(uint8_t i = 0; i < numberOfIds; i++)
	{
		card_flags_[ids[i]>>3] = card_flags_[ids[i]>>3] | 0x01<<(ids[i]%8);	//Set the specific bit
	}
	if(writeCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::revokeCard() {
#else
bool TrivialRFIDauthorisation::wipeCard() {
#endif
	for(uint8_t i = 0; i < 32 ; i++)
	{
		card_flags_[0] = 0;
	}
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(writeCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::revokeCard(const uint8_t id) {
#else
bool TrivialRFIDauthorisation::revokeCard(const uint8_t id) {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	card_flags_[id>>3] = card_flags_[id>>3] & (0xFF ^ (0x01<<(id%8)));	//Unset the specific bit
	if(writeCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	return(true);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::revokeCard(const uint8_t* ids, const uint8_t numberOfIds) {
#else
bool TrivialRFIDauthorisation::revokeCard(const uint8_t* ids, const uint8_t numberOfIds) {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	for(uint8_t i = 0; i < numberOfIds; i++)
	{
		card_flags_[ids[i]>>3] = card_flags_[ids[i]>>3] & (0xFF ^ (0x01<<(ids[i]%8)));	//Unset the specific bit
	}
	if(writeCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	return(true);
}


#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t id) {
#else
bool TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t id) {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	if(card_flags_[id>>3] & 0x01<<(id%8))	//Check the specific bit
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t* ids, const uint8_t numberOfIds) {
#else
bool TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t* ids, const uint8_t numberOfIds) {
#endif
	if(authenticateWithCard_(flags_start_block_) == false)
	{
		return(false);
	}
	if(readCardFlags_() == false)
	{
		return(false);
	}
	deAuthenticateWithCard_();
	for(uint8_t i = 0; i < numberOfIds; i++)
	{
		if(card_flags_[ids[i]>>3] & 0x01<<(ids[i]%8))	//Check the specific bit
		{
			return(true);
		}
	}
	return(false);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t* ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardUID() {
#else
uint8_t* TrivialRFIDauthorisation::cardUID() {
#endif
	return(current_uid_);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardUIDsize() {
#else
uint8_t TrivialRFIDauthorisation::cardUIDsize() {
#endif
	return(current_uid_size_);
}

#endif