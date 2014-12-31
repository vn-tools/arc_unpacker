A tool for extracting data from visual novels.
Some formats allow repacking.

Supported games
---------------

`--fmt` string                 | Game                             | Features<sup>1</sup>
------------------------------ | -------------------------------- | ------------
`--fmt=xp3 --plugin=none`      | Sono Hanabira ni Kuchizuke o 12  | ![][sup]![][par]![][sup]![][par] ![][non]![][non]![][non]![][non]
`--fmt=xp3 --plugin=fsn`       | Fate/Stay Night                  | ![][sup]![][par]![][sup]![][par] ![][non]![][non]![][non]![][non]
`--fmt=xp3 --plugin=fha`       | Fate/Hollow Ataraxia             | ![][sup]![][par]![][sup]![][par] ![][non]![][non]![][non]![][non]
`--fmt=melty_blood`            | Melty Blood                      | ![][sup]![][non]![][non]![][non] ![][non]![][non]![][non]![][non]
`--fmt=nsa`                    | Tsukihime                        | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=sar`                    | Tsukihime                        | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=ykc`                    | Hoshizora e Kakaru Hashi         | ![][sup]![][sup]![][sup]![][sup] ![][sup]![][sup]![][sup]![][sup]
`--fmt=fjsys --key=sonohana1`  | Sono Hanabira ni Kuchizuke o 1   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana2`  | Sono Hanabira ni Kuchizuke o 2   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana3`  | Sono Hanabira ni Kuchizuke o 3   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana4`  | Sono Hanabira ni Kuchizuke o 4   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana5`  | Sono Hanabira ni Kuchizuke o 5   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana6`  | Sono Hanabira ni Kuchizuke o 6   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana7`  | Sono Hanabira ni Kuchizuke o 7   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana8`  | Sono Hanabira ni Kuchizuke o 8   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana9`  | Sono Hanabira ni Kuchizuke o 9   | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana10` | Sono Hanabira ni Kuchizuke o 10  | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=fjsys --key=sonohana11` | Sono Hanabira ni Kuchizuke o 11  | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=nitroplus/pak2`         | Saya no Uta                      | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=rpa`                    | Everlasting Summer               | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=rpa`                    | Katawa Shoujo                    | ![][sup]![][sup]![][sup]![][non] ![][sup]![][sup]![][sup]![][non]
`--fmt=mbl`                    | Wanko to Kurasou                 | ![][sup]![][sup]![][sup]![][non] ![][sup]![][non]![][sup]![][non]
`--fmt=exe`                    | Generic `.exe` resources         | ![][sup]![][sup]![][sup]![][sup] ![][non]![][non]![][non]![][non]

<sup>1</sup> Feature legend:

1. File unpacking
	1. Graphics
	2. Music
	3. Scripts
2. File packing
	1. Graphics
	2. Music
	3. Scripts

[sup]: http://tmp.sakuya.pl/f/chk.png?v=2
[par]: http://tmp.sakuya.pl/f/chk2.png?v=2
[non]: http://tmp.sakuya.pl/f/chk3.png?v=2

Requirements
------------

1. `ruby` interpreter (tested on 2.0.0p598)
2. `rmagick` gem
3. `gcc`.

This is subject to change - see [#5](/../../issues/5).

Before running the script, please run `sh build.sh` to build native extensions.
Otherwise, some formats might not work correctly.

Usage
-----

1. Choose the correct --fmt parameter to decrypt the files properly.
2. Run this script on the archive files. These can be found in the game's
   installation folder.

Example
-------

    ruby ./bin/arc_unpacker.rb --fmt=xp3 --plugin=fsn ~/games/fate/fgimage.xp3 ./fgimage_unpacked/

Disclaimer
----------

Some archives provide no way to verify correctness of the extracted files. This
means that for some unsupported games, the script may extract the files and
show no errors, but you will be unable to open them nonetheless.
