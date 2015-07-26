## arc\_unpacker

1. [Overview](#overview)
2. [Download, building...](#download)
3. [Usage](#usage)
4. [Supported games](#supported-games)
5. [License](#license)
6. [Contact](#contact)



Overview
--------

`arc_unpacker` is a tool for extracting images and sounds from visual novels.

*Disclaimer*: this project doesn't support modifying the game files. If you'd
like to translate a game into English and need tools for game script/image
modification, contact me via the e-mail address listed in my [Github
profile](https://github.com/rr-).



Download
--------

- [Windows - releases](https://github.com/vn-tools/arc_unpacker/releases)
- [Windows - nightly builds](http://tmp.sakuya.pl/au/)
- [Build instructions](https://github.com/vn-tools/arc_unpacker/blob/master/BUILD.md)  
  Supported platforms: Windows, Cygwin and GNU/Linux.

Usage
-----

Drag'n'drop the archive or file onto `arc_unpacker`. It will guess the format
and unpack it for you.

Caveats:

1. The file format might be detected by two or more decoders at once. In such
cases you need to look at the console output, choose the valid format and tell
the program to use it by supplying `--fmt=...` option.

2. The file might need more parameters to be correctly unpacked. In such cases
you need to supply `--fmt=FORMAT` option and additional parameters. For
example, XP3 archives need `--plugin` that tells what kind of decryption to
use. To tell it to use Fate/Stay Night decryption, supply `--fmt=xp3
--plugin=fsn`. All of these options along with their values can be discovered
with `--fmt=FORMAT --help` switch.



Supported games
---------------

Feature legend:

![legend](https://cloud.githubusercontent.com/assets/1045476/7106067/af96459a-e135-11e4-956b-bc733b1e4ab2.png)

### Archives

Features           | Game                                | CLI invocation
------------------ | ----------------------------------- | --------------
![][F]![][X]![][X] | Generic `.exe` resources            | `--fmt=exe`
![][F]![][F]![][F] | Chaos;Head                          | `--fmt=npa`
![][F]![][F]![][F] | Cherry Tree High Comedy Club        | `--fmt=rgssad`
![][F]![][F]![][F] | Comyu Kuroi Ryuu to Yasashii Oukoku | `--fmt=xp3 --plugin=comyu`
![][F]![][F]![][F] | Everlasting Summer                  | `--fmt=rpa`
![][F]![][F]![][F] | Fate/Hollow Ataraxia                | `--fmt=xp3 --plugin=fha`
![][F]![][F]![][F] | Fate/Stay Night                     | `--fmt=xp3 --plugin=fsn`
![][F]![][F]![][F] | Higurashi No Naku Koro Ni           | `--fmt=arc`
![][F]![][F]![][F] | Hoshizora e Kakaru Hashi            | `--fmt=ykc`
![][F]![][F]![][F] | Irotoridori no Sekai                | `--fmt=fvp`
![][F]![][F]![][F] | Katawa Shoujo                       | `--fmt=rpa`
![][F]![][F]![][F] | Kimihagu                            | `--fmt=pack`
![][F]![][F]![][F] | Koiken Otome                        | `--fmt=pack --fkey=... --gameexe=...`
![][F]![][F]![][F] | Long Live The Queen                 | `--fmt=rpa`
![][F]![][F]![][F] | Mahou Tsukai no Yoru                | `--fmt=xp3 --plugin=mahoyoru`
![][F]![][F]![][F] | Maji de Watashi ni Koishinasai!     | `--fmt=pac`
![][F]![][F]![][F] | Mebae                               | `--fmt=tac`
![][F]![][F]![][F] | Melty Blood                         | `--fmt=p`
![][F]![][F]![][F] | Musume Shimai                       | `--fmt=g2`
![][F]![][F]![][F] | Pure My Imouto Milk Purin           | `--fmt=g2`
![][F]![][F]![][F] | Saya no Uta                         | `--fmt=pak`
![][F]![][F]![][F] | Sekien no Inganock                  | `--fmt=xfl`
![][F]![][F]![][F] | Sharin no Kuni, Himawari no Shoujo  | `--fmt=xp3 --plugin=noop`
![][F]![][F]![][F] | Shikkoku no Sharnoth                | `--fmt=xfl`
![][F]![][F]![][F] | Sono Hanabira ni Kuchizuke o 1-11   | `--fmt=fjsys`
![][F]![][F]![][F] | Sono Hanabira ni Kuchizuke o 12     | `--fmt=xp3 --plugin=noop`
![][F]![][F]![][F] | Soshite Ashita no Sekai yori        | `--fmt=pack`
![][F]![][F]![][F] | Souten no Celenaria                 | `--fmt=xfl`
![][F]![][F]![][F] | Steins;Gate                         | `--fmt=npa_sg`
![][F]![][F]![][F] | Sukimazakura to Uso no Machi        | `--fmt=mpk`
![][F]![][F]![][F] | Touhou 06                           | `--fmt=pbg3`
![][F]![][F]![][P] | Touhou 07                           | `--fmt=pbg4`
![][F]![][F]![][F] | Touhou 07.5                         | `--fmt=th-pak1`
![][F]![][F]![][P] | Touhou 08                           | `--fmt=pbgz`
![][F]![][F]![][P] | Touhou 09                           | `--fmt=pbgz`
![][F]![][F]![][P] | Touhou 09.5                         | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 10                           | `--fmt=tha1`
![][F]![][F]![][F] | Touhou 10.5                         | `--fmt=th-pak2`
![][F]![][F]![][P] | Touhou 11                           | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 12                           | `--fmt=tha1`
![][F]![][F]![][F] | Touhou 12.3                         | `--fmt=th-pak2`
![][F]![][F]![][P] | Touhou 12.5                         | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 12.8                         | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 13                           | `--fmt=tha1`
![][F]![][F]![][F] | Touhou 13.5                         | `--fmt=tfpk --file-names=extra/th135-file-names.lst`
![][F]![][F]![][P] | Touhou 14                           | `--fmt=tha1`
![][F]![][F]![][P] | Touhou 14.3                         | `--fmt=tha1`
![][F]![][P]![][F] | Touhou 14.5                         | `--fmt=tfpk --file-names=extra/th145-file-names.lst`
![][F]![][F]![][F] | Tsujidou-san no Jun'ai Road         | `--fmt=dat-whale --game-title=辻堂さんの純愛ロード --file-names=extra/tsujidou-junai.lst`
![][F]![][F]![][F] | Tsukihime                           | `--fmt=nsa`, `--fmt=sar`
![][F]![][F]![][F] | Umineko No Naku Koro Ni             | `--fmt=nsa`
![][F]![][F]![][F] | Wanko to Kurasou                    | `--fmt=mbl`
![][F]![][F]![][F] | Watashi no Puni Puni                | `--fmt=gml`

### Files

Features           | Game                                | CLI invocation
------------------ | ----------------------------------- | --------------
![][X]![][F]![][F] | Clannad                             | `--fmt=g00`, `--fmt=nwa`
![][X]![][F]![][F] | Fortune Summoners                   | `--fmt=sotes`
![][X]![][F]![][F] | Kanon                               | `--fmt=g00`, `--fmt=nwa`
![][X]![][F]![][F] | Little Busters                      | `--fmt=g00`, `--fmt=nwa`
![][X]![][F]![][F] | Yume Nikki                          | `--fmt=xyz`

[F]: http://i.imgur.com/PeYsbCg.png
[P]: http://i.imgur.com/NMBy1C0.png
[N]: http://i.imgur.com/2aTNlHb.png
[X]: http://i.imgur.com/jQTmqxl.png

If a game isn't listed above and it works, please let me know so I can update
the table.



License
-------

See
[LICENSE.md](https://github.com/vn-tools/arc_unpacker/blob/master/LICENSE.md).



Contact
-------

To report issues, use [GitHub issue
tracker](https://github.com/vn-tools/arc_unpacker/issues).  
For more casual discussion, hit the IRC channel - `#arc_unpacker` on Rizon.
