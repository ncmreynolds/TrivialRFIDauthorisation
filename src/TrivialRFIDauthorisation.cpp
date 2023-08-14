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
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::begin(const uint8_t sector) {
#else
bool TrivialRFIDauthorisation::begin(const uint8_t sector) {
#endif
	bool initPass = false;
	bool selfTestPass = false;
	//Initialise RFID reader
	while(self_test_retries_ > 0 && (initPass == false || selfTestPass == false))
	{
		if(initPass == false)
		{
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
				debugStream_->print(F("Initialising RFID reader: "));
			}
			#endif
			initPass = rfid_reader_.PCD_Init();
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr)
			{
				if (initPass == true) {
					debugStream_->println(F("OK"));
				}
				else	{
					debugStream_->println(F("Fail"));
				}
			}
			#endif
			if(initPass == false)
			{
				rfid_reader_.PCD_Reset();
			}

		}
		else
		{
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
				debugStream_->print(F("RFID reader self test: "));
			}
			#endif
			selfTestPass = rfid_reader_.PCD_PerformSelfTest();
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
				if (selfTestPass == true)	{
					debugStream_->println(F("OK"));
				}
				else	{
					debugStream_->println(F("Fail"));
				}
			}
			#endif
		}
		self_test_retries_--;
	}
	if(selfTestPass == true)
	{
		#ifdef TrivialRFIDauthorisationSupportEncryption
		if(keyAset_ == false)
		#endif
		{
			setDefaultCardKeyA();
		}
		#ifdef TrivialRFIDauthorisationSupportEncryption
		if(keyBset_ == false)
		#endif
		{
			setDefaultCardKeyB();
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
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("Using sector:"));
			debugStream_->print(flags_start_sector_);
			debugStream_->print(F(" block "));
			debugStream_->print(flags_start_block_);
			debugStream_->print('&');
			debugStream_->print(flags_start_block_+1);
			debugStream_->println(F(" to store authentication bitmask"));
		}
		#endif
	}
	return selfTestPass;
}

#ifdef TrivialRFIDauthorisationSupportDebugging
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::debug(Stream &debugStream)
#else
void TrivialRFIDauthorisation::debug(Stream &debugStream)
#endif
{
	debugStream_ = &debugStream;		//Set the stream used for the terminal
}
#endif

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::pollForCard() {
#else
bool TrivialRFIDauthorisation::pollForCard() {
#endif
	if(millis() - rfid_reader_last_polled_ > rfid_reader_polling_interval_)
	{
		rfid_reader_last_polled_ = millis();
		/*
		if(PICC_IsCardPresent() == false) {
		//if(rfid_reader_.PICC_IsNewCardPresent() == false) {
			if(rfid_read_failures_++ >= rfid_read_failure_threshold_)
			{
				rfid_read_failures_ = rfid_read_failure_threshold_;
				if(card_present_ == true) {
					if(debugStream_ != nullptr) {
						debugStream_->println(F("Card removed"));
					}
					for(uint8_t i = 0; i < current_uid_size_; i++) {
						current_uid_[i] = 0;
					}
					current_uid_size_ = 0;
					authenticatedWithCardForRead_ = false;
					authenticatedWithCardForWrite_ = false;
					card_present_ = false;
				}
				return false;
			}
			return(card_present_);
		}
		*/
		if(rfid_reader_.PICC_IsNewCardPresent() == true) {
			if(rfid_reader_.PICC_ReadCardSerial() == true) {
				card_present_ = true;
				card_changed_ = true;
				card_awake_ = true;
				flagsRead_ = false;
			}
		}
		else if(rfid_reader_.PICC_ReadCardSerial() == false) {
			if(rfid_read_failures_++ >= rfid_read_failure_threshold_)
			{
				rfid_read_failures_ = rfid_read_failure_threshold_;
				if(card_present_ == true) {
					#ifdef TrivialRFIDauthorisationSupportDebugging
					if(debugStream_ != nullptr) {
						debugStream_->println(F("Card removal detected due to read failures"));
					}
					#endif
					for(uint8_t i = 0; i < current_uid_size_; i++) {
						current_uid_[i] = 0;
					}
					current_uid_size_ = 0;
					card_present_ = false;
					flagsRead_ = false;
				}
				return false;
			#ifdef TrivialRFIDauthorisationSupportDebugging
			} else {
				if(debugStream_ != nullptr) {
					debugStream_->println(F("Card read failure"));
				}
			#endif
			}
			return false;
		}
		/*if(debugStream_ != nullptr)
		{
			debugStream_->println(F("present"));
		}*/
		//card_present_ = true;
		//card_changed_ = false;
		if(card_present_ == true && card_awake_ == true) {
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
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(card_present_ == true && debugStream_ != nullptr) {
				if(card_changed_ == true)	{
					debugStream_->print(F("New"));
				} else {
					debugStream_->print(F("Previous"));
				}
				debugStream_->print(F(" card presented, UID:"));
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
			#endif
			return true;
		}
	}
	return(card_present_);
}
/*
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::PICC_IsCardPresent() {
#else
bool TrivialRFIDauthorisation::PICC_IsCardPresent() {
#endif
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);
  
  // Reset baud rates
  rfid_driver_.PCD_WriteRegister(MFRC522Constants::PCD_Register::TxModeReg, 0x00);
  rfid_driver_.PCD_WriteRegister(MFRC522Constants::PCD_Register::RxModeReg, 0x00);
  // Reset ModWidthReg
  rfid_driver_.PCD_WriteRegister(MFRC522Constants::PCD_Register::ModWidthReg, 0x26);
  
  MFRC522::StatusCode result = rfid_reader_.PICC_RequestA(bufferATQA, &bufferSize);
  return (result == MFRC522Constants::StatusCode::STATUS_OK || result == MFRC522Constants::StatusCode::STATUS_COLLISION);
}
*/
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::wakeCard_() {
#else
bool TrivialRFIDauthorisation::wakeCard_() {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("Waking card: "));
	}
	#endif
	byte bufferATQA[2];
	byte bufferSize = sizeof(bufferATQA);
	MFRC522::StatusCode result = rfid_reader_.PICC_WakeupA(bufferATQA, &bufferSize);
	if(result == MFRC522Constants::StatusCode::STATUS_OK) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->println(F("success"));
		}
		#endif
		return true;
	} else {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->println(F("failure"));
		}
		#endif
	}
	return false;
}
#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authenticateWithCardForRead_(uint8_t block) {
#else
bool TrivialRFIDauthorisation::authenticateWithCardForRead_(uint8_t block) {
#endif
	if(card_awake_ == false) {
		wakeCard_();
	}
	if(authenticatedWithCardForRead_ == false) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F("Authenticating PICC for read using key A:"));
		}
		#endif
		MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.PCD_Authenticate(MFRC522Constants::PICC_Command::PICC_CMD_MF_AUTH_KEY_A, block, &keyA_, &(rfid_reader_.uid));
		if (status != MFRC522::StatusCode::STATUS_OK) {
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
				debugStream_->print(F("PCD_Authenticate() failed: "));
				debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
			}
			#endif
			return false;
		#ifdef TrivialRFIDauthorisationSupportDebugging
		} else if(debugStream_ != nullptr) {
			debugStream_->println(F("success"));
		#endif
		}
		authenticatedWithCardForRead_ = true;
	#ifdef TrivialRFIDauthorisationSupportDebugging
	} else {
		if(debugStream_ != nullptr) {
			debugStream_->println(F("Already authenticated for read using key A"));
		}
	#endif
	}
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authenticateWithCardForWrite_(uint8_t block) {
#else
bool TrivialRFIDauthorisation::authenticateWithCardForWrite_(uint8_t block) {
#endif
	if(card_awake_ == false) {
		wakeCard_();
	}
	if(authenticatedWithCardForWrite_ == false) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F("Authenticating PICC for write using key B:"));
		}
		#endif
		MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.PCD_Authenticate(MFRC522Constants::PICC_Command::PICC_CMD_MF_AUTH_KEY_A, block, &keyB_, &(rfid_reader_.uid));
		if (status != MFRC522::StatusCode::STATUS_OK) {
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
				debugStream_->print(F("PCD_Authenticate() failed: "));
				debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
			}
			#endif
			return false;
		#ifdef TrivialRFIDauthorisationSupportDebugging
		} else if(debugStream_ != nullptr) {
			debugStream_->println(F("success"));
		#endif
		}
		authenticatedWithCardForWrite_ = true;
	#ifdef TrivialRFIDauthorisationSupportDebugging
	} else {
		if(debugStream_ != nullptr) {
			debugStream_->println(F("Already authenticated for write using key B"));
		}
	#endif
	}
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::readCardFlags_() {
#else
bool TrivialRFIDauthorisation::readCardFlags_() {
#endif
	uint8_t buffer[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //Needs two bytes for the CRC
	uint8_t size = sizeof(buffer);
	for(uint8_t blockOffset = 0; blockOffset <= 1; blockOffset++) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F("MIFARE_Read() sector "));
			debugStream_->print(flags_start_sector_);
			debugStream_->print(F(" block "));
			debugStream_->print(flags_start_block_ + blockOffset);
			debugStream_->print(' ');
		}
		#endif
		MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Read(flags_start_block_ + blockOffset, buffer, &size);
		if (status != MFRC522::StatusCode::STATUS_OK) {
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
				debugStream_->print(F("failed: "));
				debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
			}
			#endif
			rfid_reader_.PICC_HaltA();
			rfid_reader_.PCD_StopCrypto1();
			return false;
		}
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F("success: "));
		}
		#endif
		for (uint8_t i = 0; i < 16; i++) {
			card_flags_[i + (16 * blockOffset)] = buffer[i];
			#ifdef TrivialRFIDauthorisationSupportDebugging
			if(debugStream_ != nullptr) {
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
			#endif
		}
	}
	flagsRead_ = true;
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::writeCardFlags_() {
#else
bool TrivialRFIDauthorisation::writeCardFlags_() {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		//MFRC522Debug::PICC_DumpMifareClassicSectorToSerial(rfid_reader_, Serial, &(rfid_reader_.uid), &keyA_, flags_start_sector_);
		debugStream_->print(F("MIFARE_Write() sector "));
		debugStream_->print(flags_start_sector_);
		debugStream_->print(F(" block "));
		debugStream_->print(flags_start_block_);
		debugStream_->print(F(" start: "));
	}
	#endif
	MFRC522::StatusCode status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Write(flags_start_block_, &card_flags_[0], 16);
	if (status != MFRC522::StatusCode::STATUS_OK) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F("failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		#endif
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		return false;
	}
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("success: "));
		for (uint8_t i = 0; i < 16; i++) {
			if(debugStream_ != nullptr) {
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
		debugStream_->print(F("MIFARE_Write() sector "));
		debugStream_->print(flags_start_sector_);
		debugStream_->print(F(" block "));
		debugStream_->print(flags_start_block_+1);
		debugStream_->print(F(" start: "));
	}
	#endif
	status = (MFRC522::StatusCode) rfid_reader_.MIFARE_Write(flags_start_block_+1, &card_flags_[16], 16);
	if (status != MFRC522::StatusCode::STATUS_OK) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr)
		{
			debugStream_->print(F("failed: "));
			debugStream_->println(MFRC522Debug::GetStatusCodeName(status));
		}
		#endif
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		return false;
	}
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr)
	{
		debugStream_->print(F("success: "));
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
	}
	#endif
	//MFRC522Debug::PICC_DumpMifareClassicSectorToSerial(rfid_reader_, Serial, &(rfid_reader_.uid), &keyA_, flags_start_sector_);
	return true;
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::deAuthenticateWithCard_() {
#else
void TrivialRFIDauthorisation::deAuthenticateWithCard_() {
#endif
	if(authenticatedWithCardForRead_ == true) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->println(F("De-authenticating PICC for read with key A"));
		}
		#endif
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		authenticatedWithCardForRead_ = false;
	}
	else if(authenticatedWithCardForWrite_ == true) {
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->println(F("De-authenticating PICC for write with key B"));
		}
		#endif
		rfid_reader_.PICC_HaltA();
		rfid_reader_.PCD_StopCrypto1();
		authenticatedWithCardForWrite_ = false;
	#ifdef TrivialRFIDauthorisationSupportDebugging
	} else {
		if(debugStream_ != nullptr) {
			debugStream_->println(F("Already de-authenticated"));
		}
	#endif
	}
	card_awake_ = false;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardChanged() {
#else
bool TrivialRFIDauthorisation::cardChanged() {
#endif
	if(card_changed_)
	{
		card_changed_ = false;
		return true;
	}
	return false;
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
	if(flagsRead_ == false) {
		if(authenticateWithCardForRead_(flags_start_block_) == false)
		{
			return false;
		}
		if(readCardFlags_() == false)
		{
			return false;
		}
	}
	for(uint8_t i = 0; i < 32; i++)
	{
		card_flags_[i] = 0xff;	//Set ALL the bits
	}
	if(writeCardFlags_() == false)
	{
		return false;
	}
	deAuthenticateWithCard_();
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authoriseCard(const uint8_t id, bool append = true) {
#else
bool TrivialRFIDauthorisation::authoriseCard(const uint8_t id, bool append = true) {
#endif
	if(append == true) {	//Read the existing flags
		if(flagsRead_ == false) {
			if(authenticateWithCardForRead_(flags_start_block_) == false)
			{
				return false;
			}
			if(readCardFlags_() == false)
			{
				return false;
			}
		}
	} else {	//Wipe all the flags
		if(flagsRead_ == false) {
			if(authenticateWithCardForRead_(flags_start_block_) == false)
			{
				return false;
			}
		}
		for(uint8_t index = 0 ; index < 32; index++) {
			card_flags_[index] = 0;
		}
	}
	card_flags_[id>>3] = card_flags_[id>>3] | 0x01<<(id%8);	//Set the specific bit
	if(writeCardFlags_() == false)
	{
		return false;
	}
	deAuthenticateWithCard_();
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::authoriseCard(const uint8_t* ids, const uint8_t numberOfIds, bool append = true) {
#else
bool TrivialRFIDauthorisation::authoriseCard(const uint8_t* ids, const uint8_t numberOfIds, bool append = true) {
#endif
	if(append == true) {	//Read the existing flags
		if(flagsRead_ == false) {
			if(authenticateWithCardForRead_(flags_start_block_) == false)
			{
				return false;
			}
			if(readCardFlags_() == false)
			{
				return false;
			}
		}
	} else {	//Wipe all the flags
		if(flagsRead_ == false) {
			if(authenticateWithCardForRead_(flags_start_block_) == false)
			{
				return false;
			}
		}
		for(uint8_t index = 0 ; index < 32; index++) {
			card_flags_[index] = 0;
		}
	}
	for(uint8_t i = 0; i < numberOfIds; i++)
	{
		card_flags_[ids[i]>>3] = card_flags_[ids[i]>>3] | 0x01<<(ids[i]%8);	//Set the specific bit
	}
	if(writeCardFlags_() == false)
	{
		return false;
	}
	deAuthenticateWithCard_();
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::revokeCardAuthorisation() {
#else
bool TrivialRFIDauthorisation::revokeCardAuthorisation() {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		//debugStream_->println(F("complete revokeCardAuthorisation"));
	}
	#endif
	for(uint8_t i = 0; i < 32 ; i++)
	{
		card_flags_[0] = 0;
	}
	if(authenticateWithCardForWrite_(flags_start_block_) == false)
	{
		return false;
	}
	if(writeCardFlags_() == false)
	{
		return false;
	}
	deAuthenticateWithCard_();
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::revokeCardAuthorisation(const uint8_t id) {
#else
bool TrivialRFIDauthorisation::revokeCardAuthorisation(const uint8_t id) {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		//debugStream_->println(F("single revokeCardAuthorisation"));
	}
	#endif
	if(flagsRead_ == false) {
		if(authenticateWithCardForRead_(flags_start_block_) == false)
		{
			return false;
		}
		if(readCardFlags_() == false)
		{
			return false;
		}
	}
	card_flags_[id>>3] = card_flags_[id>>3] & (0xFF ^ (0x01<<(id%8)));	//Unset the specific bit
	if(authenticateWithCardForWrite_(flags_start_block_) == false)
	{
		return false;
	}
	if(writeCardFlags_() == false)
	{
		return false;
	}
	deAuthenticateWithCard_();
	return true;
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::revokeCardAuthorisation(const uint8_t* ids, const uint8_t numberOfIds) {
#else
bool TrivialRFIDauthorisation::revokeCardAuthorisation(const uint8_t* ids, const uint8_t numberOfIds) {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		//debugStream_->println(F("multi revokeCardAuthorisation"));
	}
	#endif
	if(flagsRead_ == false) {
		if(authenticateWithCardForRead_(flags_start_block_) == false)
		{
			return false;
		}
		if(readCardFlags_() == false)
		{
			return false;
		}
	}
	for(uint8_t i = 0; i < numberOfIds; i++)
	{
		card_flags_[ids[i]>>3] = card_flags_[ids[i]>>3] & (0xFF ^ (0x01<<(ids[i]%8)));	//Unset the specific bit
	}
	if(authenticateWithCardForWrite_(flags_start_block_) == false)
	{
		return false;
	}
	if(writeCardFlags_() == false)
	{
		return false;
	}
	deAuthenticateWithCard_();
	return true;
}


#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t id) {
#else
bool TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t id) {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		//debugStream_->println(F("single checkCardAuthorisation"));
	}
	#endif
	if(flagsRead_ == false) {
		if(authenticateWithCardForRead_(flags_start_block_) == false)
		{
			return false;
		}
		if(readCardFlags_() == false)
		{
			return false;
		}
		deAuthenticateWithCard_();
	}
	if(card_flags_[id>>3] & 0x01<<(id%8))	//Check the specific bit
	{
		return true;
	}
	else
	{
		return false;
	}
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t* ids, const uint8_t numberOfIds) {
#else
bool TrivialRFIDauthorisation::checkCardAuthorisation(const uint8_t* ids, const uint8_t numberOfIds) {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->println(F("multi checkCardAuthorisation"));
	}
	#endif
	if(flagsRead_ == false) {
		if(authenticateWithCardForRead_(flags_start_block_) == false)
		{
			return false;
		}
		if(readCardFlags_() == false)
		{
			return false;
		}
		deAuthenticateWithCard_();
	}
	for(uint8_t i = 0; i < numberOfIds; i++)
	{
		if(card_flags_[ids[i]>>3] & 0x01<<(ids[i]%8))	//Check the specific bit
		{
			return true;
		}
	}
	return false;
}

#if defined(ESP8266) || defined(ESP32)
uint8_t* ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardUID() {
#else
uint8_t* TrivialRFIDauthorisation::cardUID() {
#endif
	return(current_uid_);
}

#if defined(ESP8266) || defined(ESP32)
bool ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardUID(uint32_t &uid) {
#else
bool TrivialRFIDauthorisation::cardUID(uint32_t &uid) {
#endif
	if(current_uid_size_ == 4) {
		uid =  uint32_t(current_uid_[0])<<24;
		uid += uint32_t(current_uid_[1])<<16;
		uid += uint32_t(current_uid_[2])<<8;
		uid += current_uid_[3];
		return true;
	} else {
		return false;
	}
}


#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardUIDsize() {
#else
uint8_t TrivialRFIDauthorisation::cardUIDsize() {
#endif
	return(current_uid_size_);
}

#if defined(ESP8266) || defined(ESP32)
uint8_t ICACHE_FLASH_ATTR TrivialRFIDauthorisation::cardKeySize() {
#else
uint8_t TrivialRFIDauthorisation::cardKeySize() {
#endif
	return(MFRC522::MIFARE_Misc::MF_KEY_SIZE);
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::setDefaultCardKeyA() {
#else
void TrivialRFIDauthorisation::setDefaultCardKeyA() {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("Setting default card key A:"));
	}
	#endif
	for (uint8_t i = 0; i < MFRC522::MIFARE_Misc::MF_KEY_SIZE; i++) {
		keyA_.keyByte[i] = 0xFF;
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F(" 0xFF"));
		}
		#endif
	}
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("\r\n"));
	}
	#endif
	#ifdef TrivialRFIDauthorisationSupportEncryption
	keyAset_ = true;
	#endif
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::setDefaultCardKeyB() {
#else
void TrivialRFIDauthorisation::setDefaultCardKeyB() {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("Setting default card key B:"));
	}
	#endif
	for (uint8_t i = 0; i < MFRC522::MIFARE_Misc::MF_KEY_SIZE; i++) {
		keyB_.keyByte[i] = 0xFF;
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(F(" 0xFF"));
		}
		#endif
	}
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("\r\n"));
	}
	#endif
	#ifdef TrivialRFIDauthorisationSupportEncryption
	keyBset_ = true;
	#endif
}
#ifdef TrivialRFIDauthorisationSupportEncryption
#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::setCustomCardKeyA(uint8_t *key) {
#else
void TrivialRFIDauthorisation::setCustomCardKeyA(uint8_t *key) {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("Setting custom card key A:"));
	}
	#endif
	for (uint8_t i = 0; i < MFRC522::MIFARE_Misc::MF_KEY_SIZE; i++) {
		keyA_.keyByte[i] = key[i];
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(' ');
			debugStream_->print(key[i], HEX);
		}
		#endif
	}
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("\r\n"));
	}
	#endif
	#ifdef TrivialRFIDauthorisationSupportEncryption
	keyAset_ = true;
	#endif
}

#if defined(ESP8266) || defined(ESP32)
void ICACHE_FLASH_ATTR TrivialRFIDauthorisation::setCustomCardKeyB(uint8_t *key) {
#else
void TrivialRFIDauthorisation::setCustomCardKeyB(uint8_t *key) {
#endif
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("Setting custom card key B:"));
	}
	#endif
	for (uint8_t i = 0; i < MFRC522::MIFARE_Misc::MF_KEY_SIZE; i++) {
		keyB_.keyByte[i] = key[i];
		#ifdef TrivialRFIDauthorisationSupportDebugging
		if(debugStream_ != nullptr) {
			debugStream_->print(' ');
			debugStream_->print(key[i], HEX);
		}
		#endif
	}
	#ifdef TrivialRFIDauthorisationSupportDebugging
	if(debugStream_ != nullptr) {
		debugStream_->print(F("\r\n"));
	}
	#endif
	#ifdef TrivialRFIDauthorisationSupportEncryption
	keyAset_ = true;
	#endif
}
#endif
#endif