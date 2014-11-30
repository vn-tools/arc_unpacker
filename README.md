A tool for extracting sprites from Fate/Stay Night.

Usage:

1. Run this script on the XP3 archives. These can be found in the game's
installation folder.

Notes:

FSN uses Kirikiri game engine, which operates on XP3 archives. The
inconvenience of reversing XP3 archives mainly lies in the way Kirikiri handles
encryption - each game can (and is encouraged to) implement customized
encryption / decryption routine:

https://github.com/krkrz/krkrz/blob/29cd9abe461fe1b0ead0b47ecb76bd5c0619542d/src/core/base/XP3Archive.cpp#L1007-L1014  
(BTW: I am almost positive that ADLR chunk isn't always included.)

As an example, in the case of FSN this routine goes like this:

    decrypted[i] = encrypted[i] ^ 0x36 for every i
    decrypted[i] = encrypted[i] ^ 0x01 for i = 0x13
    decrypted[i] = encrypted[i] ^ 0x03 for i = 0x2ea29

In any case... detecting the game based on archive contents seems to be
difficult enough to thwart any effort to create a tool that can perfectly
decompress every XP3 out there. That's why I made `fate_unpacker`, not
`xp3_unpacker`. Note that this might change when I discover more games using
this engine.
