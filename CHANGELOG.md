# Changelog



- 0.1.4 Add to API to allow clearing of existing authorised while setting new ones in one action. Added a new example and updated others to reflect this.
  
- 0.1.3 Make sure all debugging strings are stored in flash
  
- 0.1.2 Change default sector on the card to 1 from 0. **This is a breaking change for any previously written cards** but can be fixed by setting the sector to 0 with begin(0) instead of begin().
  - Refactored revokeCard to revokeCardAuthorisation for clarity, **this is a breaking change**.
  - Add #ifdefs in examples to better support different boards Arduino Nano, WeMos D1 Mini, ESP32S2
  - Made all examples use ID 28, where previously it was inconsistent
  - Added methods to set custom encryption keys, but these are not yet functional, please ignore them
  - **Known issues**
    - A program can do **ONE** transaction with a card when it is first presented, further attempts fail. This is related to handling the encryption keys being worked on.

- 0.1.1 Some API changes
  - begin() now returns a bool to show success/fail in initialising the reader. Reader initialisation will now retry up to three times before failing. 
  - pollForCard() now returns true more reliably if a card is present.
  - New method cardUID(uint32_t &uid) to put a common 4-byte card ID into an uint32_t. Will fail for 7/10 byte card IDs.
- 0.1.0 Initial release