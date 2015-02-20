A tool for extracting images and sounds from visual novels.

Disclaimer
----------

This project is for data extraction only. If you'd like to translate a game
into English and need tools for game script/image modification, contact me via
the e-mail address listed in my [Github profile](https://github.com/rr-).

Building requirements
---------------------

1. g++ 4.8+
2. libpng 1.4+
3. zlib (comes with libpng)
4. iconv (comes with POSIX)
5. POSIX compatibility <sub>(for Windows this means MinGW or Cygwin; might work
   with Visual Studio with minor tweaks, but I don't support it)</sub>.

To download binaries for Windows, head over to
[releases](https://github.com/vn-tools/arc_unpacker/releases).

Usage
-----

1. Drag and drop archive files to `arc_unpacker`.
2. Drag and drop non-archive files onto `file_decoder`.
3. Some formats need additional switches in order to unpack correctly.
   These can be found out using `--help`.

Note that some archives provide no way to verify correctness of the extracted
files. This means that for some unsupported games, the script may extract the
files and show no errors, but you will be unable to open them nonetheless.

Supported games
---------------

If you believe there is an error in these tables, please let me know.

### Archives

CLI invocation                        | Game                                | Features<sup>1</sup>
------------------------------------- | ----------------------------------- | ------------
`--fmt=xp3 --plugin=noop`             | Sono Hanabira ni Kuchizuke o 12     | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=noop`             | Sharin no Kuni, Himawari no Shoujo  | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=fsn`              | Fate/Stay Night                     | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=fha`              | Fate/Hollow Ataraxia                | ![][sup]![][sup]![][sup]
`--fmt=xp3 --plugin=comyu`            | Comyu Kuroi Ryuu to Yasashii Oukoku | ![][sup]![][sup]![][sup]
`--fmt=p`                             | Melty Blood                         | ![][sup]![][sup]![][sup]
`--fmt=sar`                           | Tsukihime                           | ![][sup]![][sup]![][sup]
`--fmt=nsa`                           | Tsukihime                           | ![][sup]![][sup]![][sup]
`--fmt=nsa`                           | Umineko No Naku Koro Ni             | ![][sup]![][sup]![][sup]
`--fmt=ykc`                           | Hoshizora e Kakaru Hashi            | ![][sup]![][sup]![][sup]
`--fmt=fjsys`                         | Sono Hanabira ni Kuchizuke o 1-11   | ![][sup]![][sup]![][sup]
`--fmt=pak`                           | Saya no Uta                         | ![][sup]![][sup]![][sup]
`--fmt=npa`                           | Chaos;Head                          | ![][sup]![][sup]![][sup]
`--fmt=rpa`                           | Everlasting Summer                  | ![][sup]![][sup]![][sup]
`--fmt=rpa`                           | Long Live The Queen                 | ![][sup]![][sup]![][sup]
`--fmt=rpa`                           | Katawa Shoujo                       | ![][sup]![][sup]![][sup]
`--fmt=mbl`                           | Wanko to Kurasou                    | ![][sup]![][sup]![][sup]
`--fmt=rgssad`                        | Cherry Tree High Comedy Club        | ![][sup]![][sup]![][sup]
`--fmt=arc`                           | Higurashi No Naku Koro Ni           | ![][sup]![][sup]![][sup]
`--fmt=npa_sg`                        | Steins;Gate                         | ![][sup]![][sup]![][sup]
`--fmt=pac`                           | Maji de Watashi ni Koishinasai!     | ![][sup]![][sup]![][sup]
`--fmt=pack`                          | Soshite Ashita no Sekai yori        | ![][sup]![][sup]![][sup]
`--fmt=pack --fkey=... --gameexe=...` | Koiken Otome                        | ![][sup]![][sup]![][sup]
`--fmt=pbg3`                          | Touhou 06                           | ![][sup]![][sup]![][sup]
`--fmt=pbg4`                          | Touhou 07                           | ![][sup]![][sup]![][par]
`--fmt=xfl`                           | Souten No Celenaria                 | ![][sup]![][non]![][sup]
`--fmt=exe`                           | Generic `.exe` resources            | ![][sup]![][nap]![][nap]

### Files

CLI invocation             | Game                                | Features<sup>1</sup>
-------------------------- | ----------------------------------- | ------------
`--fmt=g00`, `--fmt=nwa`   | Little Busters                      | ![][nap]![][sup]![][sup]
`--fmt=g00`, `--fmt=nwa`   | Clannad                             | ![][nap]![][sup]![][sup]
`--fmt=g00`, `--fmt=nwa`   | Kanon                               | ![][nap]![][sup]![][sup]
`--fmt=xyz`                | Yume Nikki                          | ![][nap]![][sup]![][sup]
`--fmt=sotes`              | Fortune Summoners                   | ![][nap]![][sup]![][sup]

<sup>1</sup> Feature legend:

- Generic files
- Graphics
- Sound and music

<sub>![][sup] - fully supported, ![][par] - partially supported, ![][non] -
unsupported, ![][nap] - doesn't apply.  
If generic file unpacking "doesn't apply", it means the game doesn't use
archives.</sub>

[sup]: http://i.imgur.com/PeYsbCg.png
[par]: http://i.imgur.com/NMBy1C0.png
[non]: http://i.imgur.com/2aTNlHb.png
[nap]: http://i.imgur.com/jQTmqxl.png
