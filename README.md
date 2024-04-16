# Trivial RFID authorisation library
An Arduino library providing trivial authorisation for access to up to 256 'things' (or groups of 'things') with a MIFARE Classic/1K RFID card. It acts as a wrapper for the [MFRC522v2 RFID library](https://github.com/OSSLibraries/Arduino_MFRC522v2).

This library is not 'secure' in any way, it is intended for use in props, homebrew games, LARP, escape room environments, fun maker projects and so on. The authorisation is stored on the card and security on this type of RFID cards is long compromised. They and the RC522 readers are however cheap and widely available.

Every 'thing' has an 8-bit (0-255) ID. If 'things' have the same ID they are considered to be in a group. Zero is a valid ID.

The library stores access flags in a 256-bit bitmask on the card, mapped directly to IDs, so a card can authorise access to none, one, many or all IDs and the block still takes the same amount of space.

This library was created as part of the [LARP hackable RFID lock](https://github.com/ncmreynolds/LarpHackableRfidLock) project. The authorisation IDs are specifically stored on the card so no 'back end' of any kind is needed, but one can be used to update the authorisation on the card if necessary.

## Sector/block usage

If you use sector 0 the library will use block 1&2 for the authorisation bitmask data, the only usable blocks in sector 0.

If you use sector 1 or higher the library will use the first two blocks in that sector, potentially leaving the third block free for other data.

By default the library uses sector 1, in early versions it used sector 0.

## Methods

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
