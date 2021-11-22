# Trivial RFID authorisation library
An Arduino library providing trivial authorisation for access to up to 256 'things' (or groups of 'things') with a MIFARE Classic/1K RFID card. It acts as a wrapper for the MFRC522v2 RFID library.

This library is not 'secure' in any way, it is intended for use in props, homebrew games, LARP, escape room environments, fun maker projects and so on. The security on this type of RFID cards is long compromised. They and the RC522 readers are however cheap and widely available.

Every 'thing' has an 8-bit (0-255) ID. If 'things' have the same ID they are considered to be in a group. Zero is a valid ID.

The library stores access flags in a 256-bit bitmask on the card, mapped directly to IDs, so a card can authorise access to none, one, many or all IDs and the block still takes the same amount of space. The sector where this block is stored can be configured but is by default sector 0.

This library was created as part of the [LARP hackable RFID lock](https://github.com/ncmreynolds/LarpHackableRfidLock) project.
