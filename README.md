A tool for extracting data from visual novels.

Currently supported games:

`--fmt` string | Game or format
-------------- | --------------------------------
xp3/noop       | Unecrypted Kirikiri XP3 archive
xp3/fsn        | Fate/Stay Night
xp3/fha        | Fate/Hollow Ataraxia

Usage:

1. Choose the correct --fmt parameter to decrypt the files properly.
2. Run this script on the archive files. These can be found in the game's
   installation folder.

Example:

    ruby bin/arcunpacker.rb fate/fgimage.xp3 fgimage_unpacked/

Warning:

Some archives provide no way to verify correctness of the extracted files. This
means that for some unsupported games, the script may extract the files and
show no errors, but you will be unable to open them nonetheless.
