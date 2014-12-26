A tool for extracting data from visual novels.

Currently supported games:

`--fmt` string  | Game or format
--------------- | --------------------------------
`xp3`           | Kirikiri XP3 archive <ul><li>`--plugin none`: unecrypted<li>`--plugin fsn`: Fate/Stay Night<li>`--plugin fha`: Fate/Hollow Ataraxia
`melty_blood`   | Melty Blood
`nsa`           | Nscripter NSA archives (e.g. Tsukihime)
`sar`           | Nscripter SAR archives (e.g. Tsukihime)
`ykc`           | YKC archives (e.g. Hoshizora e Kakaru Hashi)
`fjsys`         | FJSYS archives (e.g. Sono Hanabira ni Kuchizuke o)
`nitroplus/pak` | Nitroplus's PAK archives (e.g. Saya no Uta)
`exe`           | Windows executable files (embedded resources)

Usage:

1. Choose the correct --fmt parameter to decrypt the files properly.
2. Run this script on the archive files. These can be found in the game's
   installation folder.

Example:

    ruby bin/arc_unpacker.rb fate/fgimage.xp3 fgimage_unpacked/

Warning:

Some archives provide no way to verify correctness of the extracted files. This
means that for some unsupported games, the script may extract the files and
show no errors, but you will be unable to open them nonetheless.
