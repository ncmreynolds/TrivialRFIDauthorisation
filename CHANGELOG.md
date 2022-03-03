# Changelog



- 0.1.1 Some API changes
  - begin() now returns a bool to show success/fail in initialising the reader. Reader initialisation will now retry up to three times before failing. 
  - pollForCard() now returns true more reliably if a card is present.
  - New method cardUID(uint32_t &uid) to put a common 4-byte card ID into an uint32_t. Will fail for 7/10 byte card IDs.
- 0.1.0 Initial release