A tool for extracting data from visual novels.
Some formats allow repacking.

Currently supported games:

`--fmt` string  | Engine / format   | Game
--------------- | ----------------- | --------
`xp3`           | Kirikiri / XP3    | <ul><li>Sono Hanabira ni Kuchizuke o 12 (`--plugin none`)<li>Fate/Stay Night (`--plugin fsn`)<li>Fate/Hollow Ataraxia (`--plugin fha`)
`melty_blood`   | French Bread / ?  | <ul><li>Melty Blood
`nsa`           | Nscripter / NSA   | <ul><li>Tsukihime
`sar`           | Nscripter / SAR   | <ul><li>Tsukihime
`ykc`           | Yukascript / YKC  | <ul><li>Hoshizora e Kakaru Hashi
`fjsys`         | NSystem / FJSYS   | <ul><li>Sono Hanabira ni Kuchizuke o 1 (`--key sonohana1`)<li>Sono Hanabira ni Kuchizuke o 2 (`--key sonohana2`)<li>Sono Hanabira ni Kuchizuke o 3 (`--key sonohana3`)<li>Sono Hanabira ni Kuchizuke o 4 (`--key sonohana4`)<li>Sono Hanabira ni Kuchizuke o 5 (`--key sonohana5`)<li>Sono Hanabira ni Kuchizuke o 6 (`--key sonohana6`)<li>Sono Hanabira ni Kuchizuke o 7 (`--key sonohana7`)<li>Sono Hanabira ni Kuchizuke o 8 (`--key sonohana8`)<li>Sono Hanabira ni Kuchizuke o 9 (`--key sonohana9`)<li>Sono Hanabira ni Kuchizuke o 10 (`--key sonohana10`)<li>Sono Hanabira ni Kuchizuke o 11 (`--key sonohana11`)
`nitroplus/pak` | Nitroplus / PAK   | <ul><li>Saya no Uta
`rpa`           | Ren'py / RPA      | <ul><li>Everlasting Summer<li>Katawa Shoujo
`mbl`           | Ivory / MBL       | <ul><li>Wanko to Kurasou
`exe`           | Windows / PE/EXE  | Generic `.exe` embedded resources

Usage:

1. Choose the correct --fmt parameter to decrypt the files properly.
2. Run this script on the archive files. These can be found in the game's
   installation folder.

Example:

    ruby ./bin/arc_unpacker.rb --fmt xp3 --plugin fsn ~/games/fate/fgimage.xp3 ./fgimage_unpacked/

Warning:

Some archives provide no way to verify correctness of the extracted files. This
means that for some unsupported games, the script may extract the files and
show no errors, but you will be unable to open them nonetheless.
