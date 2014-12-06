A tool for extracting data from XP3 archives used by Kirikiri engine.

Currently supported decryption routines:

- Noop (no encryption)
- Fate/Stay Night

Usage:

1. Choose the correct --fmt parameter to decrypt the files properly.
2. Run this script on the XP3 archives. These can be found in the game's
installation folder.

Notes:

The inconvenience of reversing XP3 archives mainly lies in the way Kirikiri
handles encryption - each game can (and is encouraged to) implement customized
encryption / decryption routine:

https://github.com/krkrz/krkrz/blob/29cd9abe461fe1b0ead0b47ecb76bd5c0619542d/src/core/base/XP3Archive.cpp#L1007-L1014  
(BTW: I am almost positive that ADLR chunk isn't always included.)

As an example, in the case of Fate/Stay Night, this routine goes like this:

    decrypted[i] = encrypted[i] ^ 0x36 for every i
    decrypted[i] = encrypted[i] ^ 0x01 for i = 0x13
    decrypted[i] = encrypted[i] ^ 0x03 for i = 0x2ea29
